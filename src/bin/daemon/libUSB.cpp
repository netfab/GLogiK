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

#include "libUSB.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

libusb_context * LibUSB::pContext = nullptr;
uint8_t LibUSB::counter = 0;
bool LibUSB::status = false;

/*
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 *
 *  === protected === protected === protected === protected ===
 *
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 */

LibUSB::LibUSB(void)
{
	LibUSB::counter++;

	if( ! LibUSB::status ) {
#if DEBUGGING_ON
		LOG(DEBUG3) << "initializing libusb";
#endif
		int ret = libusb_init( &(LibUSB::pContext) );
		if ( this->USBError(ret) ) {
			throw GLogiKExcept("libusb initialization failure");
		}

		LibUSB::status = true;
	}
}

LibUSB::~LibUSB() {
	LibUSB::counter--;

	if (LibUSB::status and LibUSB::counter == 0) {
#if DEBUGGING_ON
		LOG(DEBUG3) << "closing libusb";
#endif
		libusb_exit(LibUSB::pContext);
		LibUSB::status = false;
	}
}

void LibUSB::openUSBDevice(USBDevice & device) {
	libusb_device **list;
	int numDevices = libusb_get_device_list(LibUSB::pContext, &(list));
	if( numDevices < 0 ) {
		this->USBError(numDevices);
		throw GLogiKExcept("error getting USB devices list");
	}

	for (int i = 0; i < numDevices; ++i) {
		device._pUSBDevice = list[i];
		if( libusb_get_bus_number(device._pUSBDevice) == device.getBus() and
			libusb_get_device_address(device._pUSBDevice) == device.getNum() ) {
			break;
		}
		device._pUSBDevice = nullptr;
	}

	if( device._pUSBDevice == nullptr ) {
		std::ostringstream buffer(std::ios_base::app);
		buffer	<< "libusb cannot find device " << device.getNum()
				<< " on bus " << device.getBus();
		libusb_free_device_list(list, 1);
		throw GLogiKExcept(buffer.str());
	}

	int ret = libusb_open( device._pUSBDevice, &(device._pUSBDeviceHandle) );
	if( this->USBError(ret) ) {
		libusb_free_device_list(list, 1);
		throw GLogiKExcept("opening device failure");
	}

#if DEBUGGING_ON
	LOG(DEBUG3) << device.getID() << " opened USB device";
#endif

	libusb_free_device_list(list, 1);

	try {
		this->setUSBDeviceActiveConfiguration(device);
		this->findUSBDeviceInterface(device);
	}
	catch ( const GLogiKExcept & e ) {
		this->closeUSBDevice(device);
		throw;
	}
}

void LibUSB::closeUSBDevice(USBDevice & device) noexcept {
	if( device.runDeviceUSBRequests() ) {
		/* if we ever claimed or detached some interfaces, set them back
		 * to the same state in which we found them */
		this->releaseUSBDeviceInterfaces(device);
	}
	else {
		GKSysLog(LOG_WARNING, WARNING, "skip device interfaces release, would probably fail");
	}

	libusb_close(device._pUSBDeviceHandle);
	device._pUSBDeviceHandle = nullptr;
#if DEBUGGING_ON
	LOG(DEBUG3) << device.getID() << " closed USB device";
#endif
}

void LibUSB::sendControlRequest(
	USBDevice & device,
	const unsigned char * data,
	uint16_t wLength)
{
	if( ! device.runDeviceUSBRequests() ) {
		GKSysLog(LOG_WARNING, WARNING, "skip device feature report sending, would probably fail");
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
		GKSysLog(LOG_ERR, ERROR, "HIDReportID 0 not implemented");
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
		GKSysLog(LOG_ERR, ERROR, "error sending control request");
		this->USBError(ret);
	}
	else {
#if DEBUGGING_ON
		LOG(DEBUG2) << "sent " << ret << " bytes - expected: " << wLength;
#endif
	}
}

int LibUSB::performKeysInterruptTransfer(
	USBDevice & device,
	unsigned int timeout)
{
	if( ! device.runDeviceUSBRequests() ) {
		GKSysLog(LOG_WARNING, WARNING, "skip device interrupt transfer read, would probably fail");
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

	if(ret != LIBUSB_ERROR_TIMEOUT and ret < 0 ) {
		this->USBError(ret);
	}

	return ret;
}

int LibUSB::performLCDScreenInterruptTransfer(
	USBDevice & device,
	const unsigned char * buffer,
	int bufferLength,
	unsigned int timeout)
{
	if( ! device.runDeviceUSBRequests() ) {
		GKSysLog(LOG_WARNING, WARNING, "skip device interrupt transfer write, would probably fail");
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
	LOG(DEBUG2) << "sent " << device.getLastLCDInterruptTransferLength() << " bytes - expected: " << bufferLength;
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

int LibUSB::USBError(int errorCode) noexcept {
	switch(errorCode) {
		case LIBUSB_SUCCESS:
			break;
		default:
			std::ostringstream buffer(std::ios_base::app);
			buffer	<< "libusb error (" << libusb_error_name(errorCode) << ") : "
					<< libusb_strerror( (libusb_error)errorCode );
			GKSysLog(LOG_ERR, ERROR, buffer.str());
			break;
	}

	return errorCode;
}

void LibUSB::releaseUSBDeviceInterfaces(USBDevice & device) noexcept {
	int ret = 0;
	for(auto it = device._toRelease.begin(); it != device._toRelease.end();) {
		int numInt = (*it);
#if DEBUGGING_ON
		LOG(DEBUG1) << "releasing claimed interface " << numInt;
#endif
		ret = libusb_release_interface(device._pUSBDeviceHandle, numInt); /* release */
		if( this->USBError(ret) ) {
			std::ostringstream buffer(std::ios_base::app);
			buffer << "failed to release interface : " << numInt;
			GKSysLog(LOG_ERR, ERROR, buffer.str());
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
void LibUSB::setUSBDeviceActiveConfiguration(USBDevice & device) {
	int ret = 0;

#if DEBUGGING_ON
	LOG(DEBUG1) << device.getID() << " setting up usb device configuration";
#endif

	int b = toInt( device.getBConfigurationValue() );

	{
		int bConfigurationValue = -1;
		ret = libusb_get_configuration(device._pUSBDeviceHandle, &bConfigurationValue);
		if ( this->USBError(ret) )
			throw GLogiKExcept("libusb get_configuration error");

#if DEBUGGING_ON
		LOG(DEBUG3) << device.getID() << " current active configuration value : " << bConfigurationValue;
#endif

		if( bConfigurationValue == b )
		{
#if DEBUGGING_ON
			LOG(INFO) << device.getID() << " current active configuration value matches the wanted value, skipping configuration";
#endif
			return;
		}
	}

#if DEBUGGING_ON
	LOG(DEBUG2)	<< device.getID()
				<< " wanted configuration : " << b;
	LOG(DEBUG2) << device.getID() << " will try to set the active configuration to the wanted value";
#endif

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
			GKSysLog(LOG_ERR, ERROR, buffer.str());
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
#if DEBUGGING_ON
	LOG(DEBUG2) << device.getID() << " checking current active configuration";
#endif
	ret = libusb_set_configuration(device._pUSBDeviceHandle, b);
	if ( this->USBError(ret) ) {
		throw GLogiKExcept("libusb set_configuration failure");
	}

	this->attachUSBDeviceInterfacesToKernelDrivers(device);
}

void LibUSB::findUSBDeviceInterface(USBDevice & device) {
	int ret = 0;

#if DEBUGGING_ON
	LOG(DEBUG1) << device.getID() << " searching for the expected USB device interface";
#endif

	libusb_device_descriptor deviceDescriptor;
	ret = libusb_get_device_descriptor(device._pUSBDevice, &deviceDescriptor);
	if ( this->USBError(ret) )
		throw GLogiKExcept("libusb get_device_descriptor failure");

#if DEBUGGING_ON
	LOG(DEBUG2) << "device has " << toUInt(deviceDescriptor.bNumConfigurations)
				<< " possible configuration(s)";
#if DEBUG_LIBUSB_EXTRA
	LOG(DEBUG3) << "--";
	LOG(DEBUG3) << "device descriptor";
	LOG(DEBUG3) << "--";
	LOG(DEBUG3) << "bLength            : " << toUInt(deviceDescriptor.bLength);
	LOG(DEBUG3) << "bDescriptorType    : " << toUInt(deviceDescriptor.bDescriptorType);
	LOG(DEBUG3) << "bcdUSB             : " << std::bitset<16>( toUInt(deviceDescriptor.bcdUSB) ).to_string();
	LOG(DEBUG3) << "bDeviceClass       : " << toUInt(deviceDescriptor.bDeviceClass);
	LOG(DEBUG3) << "bDeviceSubClass    : " << toUInt(deviceDescriptor.bDeviceSubClass);
	LOG(DEBUG3) << "bDeviceProtocol    : " << toUInt(deviceDescriptor.bDeviceProtocol);
	LOG(DEBUG3) << "bMaxPacketSize0    : " << toUInt(deviceDescriptor.bMaxPacketSize0);
	LOG(DEBUG3) << "idVendor           : " << std::hex << toUInt(deviceDescriptor.idVendor);
	LOG(DEBUG3) << "idProduct          : " << std::hex << toUInt(deviceDescriptor.idProduct);
	LOG(DEBUG3) << "bcdDevice          : " << std::bitset<16>( toUInt(deviceDescriptor.bcdDevice) ).to_string();
	LOG(DEBUG3) << "iManufacturer      : " << toUInt(deviceDescriptor.iManufacturer);
	LOG(DEBUG3) << "iProduct           : " << toUInt(deviceDescriptor.iProduct);
	LOG(DEBUG3) << "iSerialNumber      : " << toUInt(deviceDescriptor.iSerialNumber);
	LOG(DEBUG3) << "bNumConfigurations : " << toUInt(deviceDescriptor.bNumConfigurations);
	LOG(DEBUG3) << "--";
	LOG(DEBUG3) << "--";
	LOG(DEBUG3) << "--";
#endif
#endif

	for (unsigned int i = 0; i < toUInt(deviceDescriptor.bNumConfigurations); i++) {
		libusb_config_descriptor * configDescriptor = nullptr;
		ret = libusb_get_config_descriptor(device._pUSBDevice, i, &configDescriptor);
		if ( this->USBError(ret) ) {
			std::ostringstream buffer(std::ios_base::app);
			buffer << "get_config_descriptor failure with index : " << i;
			GKSysLog(LOG_ERR, ERROR, buffer.str());
			continue;
		}

#if DEBUGGING_ON
		LOG(DEBUG2) << "configuration " << toUInt(configDescriptor->bConfigurationValue)
					<< " has " << toUInt(configDescriptor->bNumInterfaces) << " interface(s)";

#if DEBUG_LIBUSB_EXTRA
		LOG(DEBUG3) << "--";
		LOG(DEBUG3) << "config descriptor";
		LOG(DEBUG3) << "--";
		LOG(DEBUG3) << "bLength             : " << toUInt(configDescriptor->bLength);
		LOG(DEBUG3) << "bDescriptorType     : " << toUInt(configDescriptor->bDescriptorType);
		LOG(DEBUG3) << "wTotalLength        : " << toUInt(configDescriptor->wTotalLength);
		LOG(DEBUG3) << "bNumInterfaces      : " << toUInt(configDescriptor->bNumInterfaces);
		LOG(DEBUG3) << "bConfigurationValue : " << toUInt(configDescriptor->bConfigurationValue);
		LOG(DEBUG3) << "iConfiguration      : " << toUInt(configDescriptor->iConfiguration);
		LOG(DEBUG3) << "bmAttributes        : " << toUInt(configDescriptor->bmAttributes);
		LOG(DEBUG3) << "MaxPower            : " << toUInt(configDescriptor->MaxPower);
		/* extra parsing */
		LOG(DEBUG3) << "extra_length        : " << static_cast<int>(configDescriptor->extra_length);
		LOG(DEBUG3) << "--";
		LOG(DEBUG3) << "--";
		LOG(DEBUG3) << "--";
#endif
#endif

		if ( configDescriptor->bConfigurationValue != device.getBConfigurationValue() ) {
			libusb_free_config_descriptor( configDescriptor ); /* free */
			continue; /* skip non expected configuration */
		}

		for (unsigned int j = 0; j < toUInt(configDescriptor->bNumInterfaces); j++) {
			const libusb_interface *iface = &(configDescriptor->interface[j]);
#if DEBUGGING_ON
			LOG(DEBUG2) << device.getID() << " interface " << j << " has " << iface->num_altsetting << " alternate settings";
#endif

			for (unsigned int k = 0; k < toUInt(iface->num_altsetting); k++) {
				const libusb_interface_descriptor * asDescriptor = &(iface->altsetting[k]);

#if DEBUGGING_ON
				LOG(DEBUG3)	<< device.getID() << " interface " << j
							<< " alternate setting " << toUInt(asDescriptor->bAlternateSetting)
							<< " has " << toUInt(asDescriptor->bNumEndpoints) << " endpoints";

#if DEBUG_LIBUSB_EXTRA
				LOG(DEBUG3) << "--";
				LOG(DEBUG3) << "interface descriptor";
				LOG(DEBUG3) << "--";
				LOG(DEBUG3) << "bLength            : " << toUInt(asDescriptor->bLength);
				LOG(DEBUG3) << "bDescriptorType    : " << toUInt(asDescriptor->bDescriptorType);
				LOG(DEBUG3) << "bInterfaceNumber   : " << toUInt(asDescriptor->bInterfaceNumber);
				LOG(DEBUG3) << "bAlternateSetting  : " << toUInt(asDescriptor->bAlternateSetting);
				LOG(DEBUG3) << "bNumEndpoints      : " << toUInt(asDescriptor->bNumEndpoints);
				LOG(DEBUG3) << "bInterfaceClass    : " << toUInt(asDescriptor->bInterfaceClass);
				LOG(DEBUG3) << "bInterfaceSubClass : " << toUInt(asDescriptor->bInterfaceSubClass);
				LOG(DEBUG3) << "bInterfaceProtocol : " << toUInt(asDescriptor->bInterfaceProtocol);
				LOG(DEBUG3) << "iInterface         : " << toUInt(asDescriptor->iInterface);
				/* extra parsing */
				LOG(DEBUG3) << "extra_length       : " << static_cast<int>(asDescriptor->extra_length);
				LOG(DEBUG3) << "--";
				LOG(DEBUG3) << "--";
				LOG(DEBUG3) << "--";
#endif
#endif

				if ( asDescriptor->bInterfaceNumber != device.getBInterfaceNumber() ) {
					continue; /* skip non expected interface */
				}

				if ( asDescriptor->bAlternateSetting != device.getBAlternateSetting() ) {
					continue; /* skip non expected alternate setting */
				}

				if( asDescriptor->bInterfaceClass != LIBUSB_CLASS_HID ) {
#if DEBUGGING_ON
					LOG(WARNING) << "interface " << j << " alternate settings " << k << " is not for HID device, skipping it";
#endif
					continue; /* sanity check */
				}

				if ( asDescriptor->bNumEndpoints != device.getBNumEndpoints() ) {
#if DEBUGGING_ON
					LOG(WARNING) << "skipping settings. numEndpoints: " << toUInt(asDescriptor->bNumEndpoints)
							<< " expected: " << toUInt(device.getBNumEndpoints());
#endif
					libusb_free_config_descriptor( configDescriptor ); /* free */
					throw GLogiKExcept("num_endpoints does not match");
				}

				/* specs found */
#if DEBUGGING_ON
				LOG(DEBUG1) << device.getID() << " found the expected interface, keep going on this road";
#endif

				int numInt = toInt(asDescriptor->bInterfaceNumber);
				try {
					this->detachKernelDriverFromUSBDeviceInterface(device, numInt);
				}
				catch ( const GLogiKExcept & e ) {
					libusb_free_config_descriptor( configDescriptor ); /* free */
					throw;
				}

				/* claiming interface */
#if DEBUGGING_ON
				LOG(DEBUG1) << device.getID() << " claiming interface " << numInt;
#endif
				ret = libusb_claim_interface(device._pUSBDeviceHandle, numInt);	/* claiming */
				if( this->USBError(ret) ) {
					libusb_free_config_descriptor( configDescriptor ); /* free */
					throw GLogiKExcept("claiming interface failure");
				}
				device._toRelease.push_back(numInt);	/* claimed */

				{
					/* once that the interface is claimed, check that the right configuration is set */
#if DEBUGGING_ON
					LOG(DEBUG1) << device.getID() << " checking current active configuration";
#endif
					int bConfigurationValue = -1;
					ret = libusb_get_configuration(device._pUSBDeviceHandle, &bConfigurationValue);
					if ( this->USBError(ret) ) {
						libusb_free_config_descriptor( configDescriptor ); /* free */
						throw GLogiKExcept("libusb get_configuration error");
					}

#if DEBUGGING_ON
					LOG(DEBUG2) << device.getID() << " current active configuration value : " << bConfigurationValue;
#endif
					if ( bConfigurationValue != toInt( device.getBConfigurationValue() ) ) {
						libusb_free_config_descriptor( configDescriptor ); /* free */
						std::ostringstream buffer(std::ios_base::app);
						buffer << "wrong configuration value : " << bConfigurationValue;
						GKSysLog(LOG_ERR, ERROR, buffer.str());
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
								LOG(WARNING) << "[Keys] endpoint already found !";
							}
							else {
#if DEBUGGING_ON
								LOG(DEBUG3) << "found [Keys] endpoint, address 0x" << std::hex << addr
											<< " MaxPacketSize " << toUInt(ep->wMaxPacketSize);
#endif
								device._keysEndpoint = addr & 0xff;
							}
						}
						else if( (addr & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_OUT ) {
							/* Out: host-to-device */

							if(device._LCDEndpoint != 0) {
								LOG(WARNING) << "[LCD] endpoint already found !";
							}
							else {
#if DEBUGGING_ON
								LOG(DEBUG3) << "found [LCD] endpoint, address 0x" << std::hex << addr
											<< " MaxPacketSize " << toUInt(ep->wMaxPacketSize);
#endif
								device._LCDEndpoint = addr & 0xff;
							}
						}

#if DEBUGGING_ON
						LOG(DEBUG3) << "int. " << j
									<< " alt_s. " << toUInt(asDescriptor->bAlternateSetting)
									<< " endpoint " << l;

#if DEBUG_LIBUSB_EXTRA
						LOG(DEBUG3) << "--";
						LOG(DEBUG3) << "endpoint descriptor";
						LOG(DEBUG3) << "--";
						LOG(DEBUG3) << "bLength          : " << toUInt(ep->bLength);
						LOG(DEBUG3) << "bDescriptorType  : " << toUInt(ep->bDescriptorType);
						LOG(DEBUG3) << "bEndpointAddress : " << toUInt(ep->bEndpointAddress);
						LOG(DEBUG3) << "bmAttributes     : " << toUInt(ep->bmAttributes);
						LOG(DEBUG3) << "wMaxPacketSize   : " << toUInt(ep->wMaxPacketSize);
						LOG(DEBUG3) << "bInterval        : " << toUInt(ep->bInterval);
						LOG(DEBUG3) << "bRefresh         : " << toUInt(ep->bRefresh);
						LOG(DEBUG3) << "bSynchAddress    : " << toUInt(ep->bSynchAddress);
						/* extra parsing */
						LOG(DEBUG3) << "extra_length     : " << static_cast<int>(ep->extra_length);
						LOG(DEBUG3) << "--";
						LOG(DEBUG3) << "--";
						LOG(DEBUG3) << "--";
#endif
#endif
					}
				}

				if(device._keysEndpoint == 0) {
					libusb_free_config_descriptor( configDescriptor ); /* free */
					const std::string err("[Keys] endpoint not found");
					GKSysLog(LOG_ERR, ERROR, err);
					throw GLogiKExcept(err);
				}

				if(device._LCDEndpoint == 0) {
					libusb_free_config_descriptor( configDescriptor ); /* free */
					const std::string err("[LCD] endpoint not found");
					GKSysLog(LOG_ERR, ERROR, err);
					throw GLogiKExcept(err);
				}

#if DEBUGGING_ON
				LOG(INFO)	<< device.getID() << " all done ! "
							<< device.getFullName()
							<< " interface " << numInt
							<< " opened and ready for I/O transfers";
#endif

			} /* for ->num_altsetting */
		} /* for ->bNumInterfaces */

		libusb_free_config_descriptor( configDescriptor ); /* free */
	} /* for .bNumConfigurations */

}
void LibUSB::attachUSBDeviceInterfacesToKernelDrivers(USBDevice & device) noexcept {
	int ret = 0;
	for(auto it = device._toAttach.begin(); it != device._toAttach.end();) {
		int numInt = (*it);
#if DEBUGGING_ON
		LOG(DEBUG1) << device.getID() << " attaching kernel driver to interface " << numInt;
#endif
		ret = libusb_attach_kernel_driver(device._pUSBDeviceHandle, numInt); /* attaching */
		if( this->USBError(ret) ) {
			std::ostringstream buffer(std::ios_base::app);
			buffer << "failed to attach kernel driver to interface " << numInt;
			GKSysLog(LOG_ERR, ERROR, buffer.str());
		}
		it++;
	}
	device._toAttach.clear();
}

void LibUSB::detachKernelDriverFromUSBDeviceInterface(USBDevice & device, int numInt) {
	int ret = libusb_kernel_driver_active(device._pUSBDeviceHandle, numInt);
	if( ret < 0 ) {
		this->USBError(ret);
		throw GLogiKExcept("libusb kernel_driver_active error");
	}
	if( ret ) {
#if DEBUGGING_ON
		LOG(DEBUG1) << device.getID() << " detaching kernel driver from interface " << numInt;
#endif
		ret = libusb_detach_kernel_driver(device._pUSBDeviceHandle, numInt); /* detaching */
		if( this->USBError(ret) ) {
			std::ostringstream buffer(std::ios_base::app);
			buffer << "failed to detach kernel driver from USB interface " << numInt;
			GKSysLog(LOG_ERR, ERROR, buffer.str());
			throw GLogiKExcept(buffer.str());
		}

		device._toAttach.push_back(numInt);	/* detached */
	}
	else {
#if DEBUGGING_ON
		LOG(DEBUG1) << device.getID() << " interface " << numInt << " is currently free :)";
#endif
	}
}

} // namespace GLogiK

