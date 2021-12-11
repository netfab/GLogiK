/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 *	TODO Implement potential extra descriptors parsing
 *		(search for «extra parsing» string in this file)
 */

#include <bitset>
#include <mutex>
#include <sstream>

#include "lib/utils/utils.hpp"

#include "libusb.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

/*
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 *
 *  === protected === protected === protected === protected ===
 *
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 */

void libusb::openUSBDevice(USBDevice & device)
{
	GK_LOG_FUNC

	/* throws on failure */
	this->seekUSBDevice(device);

	int ret = libusb_open( device._pUSBDevice, &(device._pUSBDeviceHandle) );
	if( this->USBError(ret) ) {
		throw GLogiKExcept("opening device failure");
	}

	GKLog2(trace, device.getID(), " opened USB device")

	try {
		this->setUSBDeviceActiveConfiguration(device);
		this->findUSBDeviceInterface(device);
	}
	catch ( const GLogiKExcept & e ) {
		this->closeUSBDevice(device);
		throw;
	}
}

void libusb::closeUSBDevice(USBDevice & device) noexcept
{
	GK_LOG_FUNC

	if( device.getUSBRequestsStatus() ) {
		/* if we ever claimed or detached some interfaces, set them back
		 * to the same state in which we found them */
		this->releaseUSBDeviceInterfaces(device);
	}
	else {
		GKSysLogWarning("skip device interfaces release, would probably fail");
	}

	libusb_close(device._pUSBDeviceHandle);
	device._pUSBDeviceHandle = nullptr;

	GKLog2(trace, device.getID(), " closed USB device")
}

void libusb::sendUSBDeviceFeatureReport(
	USBDevice & device,
	const unsigned char * data,
	uint16_t wLength)
{
	GK_LOG_FUNC

	if( ! device.getUSBRequestsStatus() ) {
		GKSysLogWarning("skip device feature report sending, would probably fail");
		return;
	}

	/*
	 *	Device Class Definition for HID 1.11
	 *		7.2   Class-Specific Requests
	 *		7.2.2 Set_Report Request
	 *	https://www.usb.org/document-library/device-class-definition-hid-111
	 */
	const uint16_t HIDReportType = 0x03 << 8;  /* high byte - Report Type: Feature */
	const uint16_t HIDReportID = data[0];      /* low byte  - Report ID */
	// TODO - support when HIDReportID == 0
	if( HIDReportID == 0) {
		GKSysLogError("HIDReportID 0 not implemented");
		return;
	}

	int ret = 0;
	yield_for(std::chrono::microseconds(100));
	{
		std::lock_guard<std::mutex> lock(device._libUSBMutex);
		ret = libusb_control_transfer(
			device._pUSBDeviceHandle,
			LIBUSB_ENDPOINT_OUT|LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,
			0x09,                           /* bRequest - SET_REPORT */
			(HIDReportType | HIDReportID),  /* wValue - Report Type and ReportID */
			device.getBInterfaceNumber(),   /* wIndex - interface */
			const_cast<unsigned char *>(data),
			wLength,
			1000
		);
	}
	if( ret < 0 ) {
		GKSysLogError("error sending control request");
		this->USBError(ret);
	}
	else {
#if DEBUGGING_ON
		if(GKLogging::GKDebug) {
			LOG(trace)	<< device.getID()
						<< " sent " << ret
						<< " bytes - expected : " << wLength;
		}
#endif
	}
}

int libusb::performUSBDeviceKeysInterruptTransfer(
	USBDevice & device,
	unsigned int timeout)
{
	GK_LOG_FUNC

	if( ! device.getUSBRequestsStatus() ) {
		GKSysLogWarning("skip device interrupt transfer read, would probably fail");
		return 0;
	}

	yield_for(std::chrono::microseconds(100));

	int ret = 0;
	{
		std::lock_guard<std::mutex> lock(device._libUSBMutex);
		ret = libusb_interrupt_transfer(
			device._pUSBDeviceHandle,
			device._keysEndpoint,
			static_cast<unsigned char*>(device._pressedKeys),
			device.getKeysInterruptBufferMaxLength(),
			&(device._lastKeysInterruptTransferLength),
			timeout
		);
	}

	if(ret < 0 and ret != LIBUSB_ERROR_TIMEOUT) {
		this->USBError(ret);
	}

	return ret;
}

int libusb::performUSBDeviceLCDScreenInterruptTransfer(
	USBDevice & device,
	const unsigned char * buffer,
	int bufferLength,
	unsigned int timeout)
{
	GK_LOG_FUNC

	if( ! device.getUSBRequestsStatus() ) {
		GKSysLogWarning("skip device interrupt transfer write, would probably fail");
		return 0;
	}

	int ret = 0;
	{
		std::lock_guard<std::mutex> lock(device._libUSBMutex);
		/* here we assume that buffer will be used read-only by
		 * interrupt_transfer since _LCDEndpoint direction is OUT
		 * (host-to-device) */
		ret = libusb_interrupt_transfer(
			device._pUSBDeviceHandle,
			device._LCDEndpoint,
			const_cast<unsigned char *>(buffer),
			bufferLength,
			&(device._lastLCDInterruptTransferLength),
			timeout
		);
	}

#if DEBUGGING_ON && DEBUG_LIBUSB_EXTRA
	if(GKLogging::GKDebug) {
		LOG(trace)	<< device.getID()
					<< " sent " << device.getLastLCDInterruptTransferLength()
					<< " bytes - expected : " << bufferLength;
	}
#endif

	if( ret < 0 ) {
		this->USBError(ret);
	}

	return ret;
}

/*
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 *
 * === private === private === private === private === private ===
 *
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 */

void libusb::releaseUSBDeviceInterfaces(USBDevice & device) noexcept
{
	GK_LOG_FUNC

	int ret = 0;
	for(auto it = device._toRelease.begin(); it != device._toRelease.end();) {
		int numInt = (*it);

		GKLog3(trace, device.getID(), " releasing claimed interface : ", numInt)

		ret = libusb_release_interface(device._pUSBDeviceHandle, numInt); /* release */
		if( this->USBError(ret) ) {
			std::ostringstream buffer(std::ios_base::app);
			buffer << "failed to release interface : " << numInt;
			GKSysLogError(buffer.str());
		}
		it++;
	}
	device._toRelease.clear();

	this->attachUSBDeviceInterfacesToKernelDrivers(device);
}

/*
 * This function is trying to (in this order) :
 *	- check the current active configuration value
 *	- if previous value matches the wanted one, simply return, else :
 *	- detach all kernel drivers from all interfaces
 *	- set the wanted configuration
 *	- re-attach all the previously attached drivers to interfaces
 *
 * In the comments on libusb/core.c, you can find :
 *		-#	libusb will be unable to set a configuration if other programs or
 *			drivers have claimed interfaces. In particular, this means that kernel
 *			drivers must be detached from all the interfaces before
 *			libusb_set_configuration() may succeed.
 *
 */
void libusb::setUSBDeviceActiveConfiguration(USBDevice & device)
{
	GK_LOG_FUNC

	int ret = 0;

	GKLog2(trace, device.getID(), " setting up usb device configuration")

	int b = toInt( device.getBConfigurationValue() );

	{
		int bConfigurationValue = -1;
		ret = libusb_get_configuration(device._pUSBDeviceHandle, &bConfigurationValue);
		if ( this->USBError(ret) )
			throw GLogiKExcept("libusb get_configuration error");

		GKLog3(trace, device.getID(), " current active configuration value : ", bConfigurationValue)

		if( bConfigurationValue == b ) {
			GKLog2(info, device.getID(), " current active configuration value matches the wanted value, skipping configuration")
			return;
		}
	}

	GKLog2(trace, device.getID(), " will try to set the active configuration to the wanted value")

	/* have to detach all interfaces first */

	libusb_device_descriptor deviceDescriptor;
	ret = libusb_get_device_descriptor(device._pUSBDevice, &deviceDescriptor);
	if ( this->USBError(ret) )
		throw GLogiKExcept("libusb get_device_descriptor failure");

	for(unsigned int i = 0; i < toUInt(deviceDescriptor.bNumConfigurations); i++) {
		/* configuration descriptor */
		libusb_config_descriptor * configDescriptor = nullptr;
		ret = libusb_get_config_descriptor(device._pUSBDevice, i, &configDescriptor);
		if ( this->USBError(ret) ) {
			std::ostringstream buffer(std::ios_base::app);
			buffer << "get_config_descriptor failure with index : " << i;
			GKSysLogError(buffer.str());
			continue;
		}

		for(unsigned int j = 0; j < toUInt(configDescriptor->bNumInterfaces); j++) {
			const libusb_interface *iface = &(configDescriptor->interface[j]);

			for (unsigned int k = 0; k < toUInt(iface->num_altsetting); k++) {
				/* interface alt_setting descriptor */
				const libusb_interface_descriptor * asDescriptor = &(iface->altsetting[k]);

				try {
					int numInt = toInt(asDescriptor->bInterfaceNumber);
					this->detachKernelDriverFromUSBDeviceInterface(device, numInt);
				}
				catch ( const GLogiKExcept & e ) {
					libusb_free_config_descriptor( configDescriptor ); /* free */
					throw;
				}
			} /* for ->num_altsetting */
		} /* for ->bNumInterfaces */

		libusb_free_config_descriptor( configDescriptor ); /* free */
	} /* for .bNumConfigurations */

	/* trying to set configuration */
	GKLog2(trace, device.getID(), " setting device active configuration")

	ret = libusb_set_configuration(device._pUSBDeviceHandle, b);
	if ( this->USBError(ret) ) {
		throw GLogiKExcept("libusb set_configuration failure");
	}

	this->attachUSBDeviceInterfacesToKernelDrivers(device);
}

void libusb::findUSBDeviceInterface(USBDevice & device)
{
	GK_LOG_FUNC

	int ret = 0;

	GKLog2(trace, device.getID(), " searching for the expected USB device interface")

	libusb_device_descriptor deviceDescriptor;
	ret = libusb_get_device_descriptor(device._pUSBDevice, &deviceDescriptor);
	if( this->USBError(ret) )
		throw GLogiKExcept("libusb get_device_descriptor failure");

#if DEBUGGING_ON
	if(GKLogging::GKDebug) {
		LOG(trace)	<< device.getID() << " number of device configuration(s) : "
					<< toUInt(deviceDescriptor.bNumConfigurations);
#if DEBUG_LIBUSB_EXTRA
		LOG(trace) << "--";
		LOG(trace) << "device descriptor";
		LOG(trace) << "--";
		LOG(trace) << "bLength            : " << toUInt(deviceDescriptor.bLength);
		LOG(trace) << "bDescriptorType    : " << toUInt(deviceDescriptor.bDescriptorType);
		LOG(trace) << "bcdUSB             : " << std::bitset<16>( toUInt(deviceDescriptor.bcdUSB) ).to_string();
		LOG(trace) << "bDeviceClass       : " << toUInt(deviceDescriptor.bDeviceClass);
		LOG(trace) << "bDeviceSubClass    : " << toUInt(deviceDescriptor.bDeviceSubClass);
		LOG(trace) << "bDeviceProtocol    : " << toUInt(deviceDescriptor.bDeviceProtocol);
		LOG(trace) << "bMaxPacketSize0    : " << toUInt(deviceDescriptor.bMaxPacketSize0);
		LOG(trace) << "idVendor           : " << std::hex << toUInt(deviceDescriptor.idVendor);
		LOG(trace) << "idProduct          : " << std::hex << toUInt(deviceDescriptor.idProduct);
		LOG(trace) << "bcdDevice          : " << std::bitset<16>( toUInt(deviceDescriptor.bcdDevice) ).to_string();
		LOG(trace) << "iManufacturer      : " << toUInt(deviceDescriptor.iManufacturer);
		LOG(trace) << "iProduct           : " << toUInt(deviceDescriptor.iProduct);
		LOG(trace) << "iSerialNumber      : " << toUInt(deviceDescriptor.iSerialNumber);
		LOG(trace) << "bNumConfigurations : " << toUInt(deviceDescriptor.bNumConfigurations);
		LOG(trace) << "--";
#endif
	}
#endif

	for (unsigned int i = 0; i < toUInt(deviceDescriptor.bNumConfigurations); i++) {
		libusb_config_descriptor * configDescriptor = nullptr;
		ret = libusb_get_config_descriptor(device._pUSBDevice, i, &configDescriptor);
		if ( this->USBError(ret) ) {
			std::ostringstream buffer(std::ios_base::app);
			buffer << "get_config_descriptor failure with index : " << i;
			GKSysLogError(buffer.str());
			continue;
		}

#if DEBUGGING_ON
		if(GKLogging::GKDebug) {
			LOG(trace)	<< device.getID() << " configuration "
						<< toUInt(configDescriptor->bConfigurationValue) << ", interface(s) # : "
						<< toUInt(configDescriptor->bNumInterfaces);
#if DEBUG_LIBUSB_EXTRA
			LOG(trace) << "--";
			LOG(trace) << "config descriptor";
			LOG(trace) << "--";
			LOG(trace) << "bLength             : " << toUInt(configDescriptor->bLength);
			LOG(trace) << "bDescriptorType     : " << toUInt(configDescriptor->bDescriptorType);
			LOG(trace) << "wTotalLength        : " << toUInt(configDescriptor->wTotalLength);
			LOG(trace) << "bNumInterfaces      : " << toUInt(configDescriptor->bNumInterfaces);
			LOG(trace) << "bConfigurationValue : " << toUInt(configDescriptor->bConfigurationValue);
			LOG(trace) << "iConfiguration      : " << toUInt(configDescriptor->iConfiguration);
			LOG(trace) << "bmAttributes        : " << toUInt(configDescriptor->bmAttributes);
			LOG(trace) << "MaxPower            : " << toUInt(configDescriptor->MaxPower);
			/* extra parsing */
			LOG(trace) << "extra_length        : " << static_cast<int>(configDescriptor->extra_length);
			LOG(trace) << "--";
#endif
		}
#endif

		if ( configDescriptor->bConfigurationValue != device.getBConfigurationValue() ) {
			libusb_free_config_descriptor( configDescriptor ); /* free */
			continue; /* skip non expected configuration */
		}

		for (unsigned int j = 0; j < toUInt(configDescriptor->bNumInterfaces); j++) {
			const libusb_interface *iface = &(configDescriptor->interface[j]);
#if DEBUGGING_ON
			if(GKLogging::GKDebug) {
				LOG(trace)	<< device.getID() << " interface " << j
							<< ", alternate settings # : " << iface->num_altsetting;
			}
#endif

			for (unsigned int k = 0; k < toUInt(iface->num_altsetting); k++) {
				const libusb_interface_descriptor * asDescriptor = &(iface->altsetting[k]);

#if DEBUGGING_ON
				if(GKLogging::GKDebug) {
					LOG(trace)	<< device.getID() << " int. " << j
								<< " alt_s " << toUInt(asDescriptor->bAlternateSetting)
								<< ", endpoints # : " << toUInt(asDescriptor->bNumEndpoints);
#if DEBUG_LIBUSB_EXTRA
					LOG(trace) << "--";
					LOG(trace) << "interface descriptor";
					LOG(trace) << "--";
					LOG(trace) << "bLength            : " << toUInt(asDescriptor->bLength);
					LOG(trace) << "bDescriptorType    : " << toUInt(asDescriptor->bDescriptorType);
					LOG(trace) << "bInterfaceNumber   : " << toUInt(asDescriptor->bInterfaceNumber);
					LOG(trace) << "bAlternateSetting  : " << toUInt(asDescriptor->bAlternateSetting);
					LOG(trace) << "bNumEndpoints      : " << toUInt(asDescriptor->bNumEndpoints);
					LOG(trace) << "bInterfaceClass    : " << toUInt(asDescriptor->bInterfaceClass);
					LOG(trace) << "bInterfaceSubClass : " << toUInt(asDescriptor->bInterfaceSubClass);
					LOG(trace) << "bInterfaceProtocol : " << toUInt(asDescriptor->bInterfaceProtocol);
					LOG(trace) << "iInterface         : " << toUInt(asDescriptor->iInterface);
					/* extra parsing */
					LOG(trace) << "extra_length       : " << static_cast<int>(asDescriptor->extra_length);
					LOG(trace) << "--";
#endif
				}
#endif

				if ( asDescriptor->bInterfaceNumber != device.getBInterfaceNumber() ) {
					continue; /* skip non expected interface */
				}

				if ( asDescriptor->bAlternateSetting != device.getBAlternateSetting() ) {
					continue; /* skip non expected alternate setting */
				}

				if( asDescriptor->bInterfaceClass != LIBUSB_CLASS_HID ) {
					std::ostringstream buffer(std::ios_base::app);
					buffer	<< device.getID() << " interface " << j
							<< " alternate settings " << k
							<< " is not for HID device, skipping it";
					GKSysLogWarning(buffer.str());
					continue; /* sanity check */
				}

				if ( asDescriptor->bNumEndpoints != device.getBNumEndpoints() ) {
					std::ostringstream buffer(std::ios_base::app);
					buffer	<< device.getID() << " skipping settings. numEndpoints : "
							<< toUInt(asDescriptor->bNumEndpoints)
							<< " expected : " << toUInt(device.getBNumEndpoints());
					GKSysLogWarning(buffer.str());

					libusb_free_config_descriptor( configDescriptor ); /* free */
					throw GLogiKExcept("num_endpoints does not match");
				}

				/* specs found */
				GKLog2(trace, device.getID(), " found the expected interface, keep going on this road")

				int numInt = toInt(asDescriptor->bInterfaceNumber);
				try {
					this->detachKernelDriverFromUSBDeviceInterface(device, numInt);
				}
				catch ( const GLogiKExcept & e ) {
					libusb_free_config_descriptor( configDescriptor ); /* free */
					throw;
				}

				/* claiming interface */
				GKLog3(trace, device.getID(), " claiming interface : ", numInt)

				ret = libusb_claim_interface(device._pUSBDeviceHandle, numInt);	/* claiming */
				if( this->USBError(ret) ) {
					libusb_free_config_descriptor( configDescriptor ); /* free */
					throw GLogiKExcept("failed to claim interface");
				}
				device._toRelease.push_back(numInt);	/* claimed */

				{
					/* once that the interface is claimed, check that the right configuration is set */
					GKLog2(trace, device.getID(), " checking current active configuration")

					int bConfigurationValue = -1;
					ret = libusb_get_configuration(device._pUSBDeviceHandle, &bConfigurationValue);
					if ( this->USBError(ret) ) {
						libusb_free_config_descriptor( configDescriptor ); /* free */
						throw GLogiKExcept("libusb get_configuration error");
					}

					GKLog3(trace, device.getID(), " current active configuration value : ", bConfigurationValue)

					if ( bConfigurationValue != toInt( device.getBConfigurationValue() ) ) {
						libusb_free_config_descriptor( configDescriptor ); /* free */

						std::ostringstream buffer(std::ios_base::app);
						buffer << "wrong configuration value : " << bConfigurationValue;
						GKSysLogError(buffer.str());

						throw GLogiKExcept(buffer.str());
					}
				}

				for (unsigned int l = 0; l < toUInt(asDescriptor->bNumEndpoints); l++) {
					const libusb_endpoint_descriptor * ep = &(asDescriptor->endpoint[l]);

					/* check transfer type */
					if( (toUInt(ep->bmAttributes) & LIBUSB_TRANSFER_TYPE_MASK)
							== LIBUSB_TRANSFER_TYPE_INTERRUPT ) {

						const unsigned int addr = toUInt(ep->bEndpointAddress);

						if( (addr & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN ) {
							/* In: device-to-host */

							if(device._keysEndpoint != 0) {
								GKSysLogWarning("[Keys] endpoint already found !");
							}
							else {
#if DEBUGGING_ON
								if(GKLogging::GKDebug) {
									LOG(trace)	<< "found [Keys] endpoint, address 0x" << std::hex << addr
												<< " MaxPacketSize " << toUInt(ep->wMaxPacketSize);
								}
#endif
								device._keysEndpoint = addr & 0xff;
							}
						}
						else if( (addr & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_OUT ) {
							/* Out: host-to-device */

							if(device._LCDEndpoint != 0) {
								GKSysLogWarning("[LCD] endpoint already found !");
							}
							else {
#if DEBUGGING_ON
								if(GKLogging::GKDebug) {
									LOG(trace)	<< "found [LCD] endpoint, address 0x" << std::hex << addr
												<< " MaxPacketSize " << toUInt(ep->wMaxPacketSize);
								}
#endif
								device._LCDEndpoint = addr & 0xff;
							}
						}

#if DEBUGGING_ON
						if(GKLogging::GKDebug) {
							LOG(trace)	<< device.getID() << " int. " << j
										<< " alt_s. " << toUInt(asDescriptor->bAlternateSetting)
										<< " endpoint " << l;
#if DEBUG_LIBUSB_EXTRA
							LOG(trace) << "--";
							LOG(trace) << "endpoint descriptor";
							LOG(trace) << "--";
							LOG(trace) << "bLength          : " << toUInt(ep->bLength);
							LOG(trace) << "bDescriptorType  : " << toUInt(ep->bDescriptorType);
							LOG(trace) << "bEndpointAddress : " << toUInt(ep->bEndpointAddress);
							LOG(trace) << "bmAttributes     : " << toUInt(ep->bmAttributes);
							LOG(trace) << "wMaxPacketSize   : " << toUInt(ep->wMaxPacketSize);
							LOG(trace) << "bInterval        : " << toUInt(ep->bInterval);
							LOG(trace) << "bRefresh         : " << toUInt(ep->bRefresh);
							LOG(trace) << "bSynchAddress    : " << toUInt(ep->bSynchAddress);
							/* extra parsing */
							LOG(trace) << "extra_length     : " << static_cast<int>(ep->extra_length);
							LOG(trace) << "--";
#endif
						}
#endif
					}
				}

				if(device._keysEndpoint == 0) {
					libusb_free_config_descriptor( configDescriptor ); /* free */
					const std::string err("[Keys] endpoint not found");
					GKSysLogError(err);
					throw GLogiKExcept(err);
				}

				if(device._LCDEndpoint == 0) {
					libusb_free_config_descriptor( configDescriptor ); /* free */
					const std::string err("[LCD] endpoint not found");
					GKSysLogError(err);
					throw GLogiKExcept(err);
				}

#if DEBUGGING_ON
				if(GKLogging::GKDebug) {
					LOG(info)	<< device.getID() << " all done ! "
								<< device.getFullName()
								<< " interface " << numInt
								<< " opened and ready for I/O transfers";
				}
#endif

			} /* for ->num_altsetting */
		} /* for ->bNumInterfaces */

		libusb_free_config_descriptor( configDescriptor ); /* free */
	} /* for .bNumConfigurations */
}

void libusb::attachUSBDeviceInterfacesToKernelDrivers(USBDevice & device) noexcept
{
	GK_LOG_FUNC

	int ret = 0;
	for(auto it = device._toAttach.begin(); it != device._toAttach.end();) {
		int numInt = (*it);

		GKLog3(trace, device.getID(), " attaching kernel driver to interface : ", numInt)

		ret = libusb_attach_kernel_driver(device._pUSBDeviceHandle, numInt); /* attaching */
		if( this->USBError(ret) ) {
			std::ostringstream buffer(std::ios_base::app);
			buffer << "failed to attach kernel driver to interface " << numInt;
			GKSysLogError(buffer.str());
		}
		it++;
	}
	device._toAttach.clear();
}

void libusb::detachKernelDriverFromUSBDeviceInterface(USBDevice & device, int numInt)
{
	GK_LOG_FUNC

	int ret = libusb_kernel_driver_active(device._pUSBDeviceHandle, numInt);
	if( ret < 0 ) {
		this->USBError(ret);
		throw GLogiKExcept("libusb kernel_driver_active error");
	}
	if( ret ) {
		GKLog3(trace, device.getID(), " detaching kernel driver from interface : ", numInt)

		ret = libusb_detach_kernel_driver(device._pUSBDeviceHandle, numInt); /* detaching */
		if( this->USBError(ret) ) {
			std::ostringstream buffer(std::ios_base::app);
			buffer << "failed to detach kernel driver from USB interface " << numInt;
			GKSysLogError(buffer.str());
			throw GLogiKExcept(buffer.str());
		}

		device._toAttach.push_back(numInt);	/* detached */
	}
	else {
		GKLog3(trace, device.getID(), " interface already freed : ", numInt)
	}
}

} // namespace GLogiK

