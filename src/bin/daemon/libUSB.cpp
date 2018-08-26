/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2018  Fabrice Delliaux <netbox253@gmail.com>
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

#include <bitset>
#include <mutex>
#include <sstream>

#include "lib/utils/utils.h"

#include "libUSB.h"

namespace GLogiK
{

using namespace NSGKUtils;

bool LibUSB::status = false;
uint8_t LibUSB::counter = 0;
libusb_context * LibUSB::pContext = nullptr;

LibUSB::LibUSB(const int maxLength, const ExpectedDescriptorsValues & values)
	:	_expectedDescriptorsValues(values)
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

	_keysInterruptBufferMaxLength = maxLength;

	if( maxLength > KEYS_BUFFER_LENGTH ) {
		GKSysLog(LOG_WARNING, WARNING, "interrupt read length too large, set it to max buffer length");
		_keysInterruptBufferMaxLength = KEYS_BUFFER_LENGTH;
	}

}

LibUSB::~LibUSB() {
	LibUSB::counter--;

	if (LibUSB::status and LibUSB::counter == 0) {
#if DEBUGGING_ON
		LOG(DEBUG3) << "closing libusb";
#endif
		// FIXME
		//if( this->initialized_devices_.size() != 0 ) { // sanity check
		//	GKSysLog(LOG_WARNING, WARNING, "closing libusb with opened device(s) !");
		//}
		libusb_exit(LibUSB::pContext);
		LibUSB::status = false;
	}
}

int LibUSB::USBError(int errorCode) {
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
	LOG(DEBUG3) << device.getStringID() << " opened USB device";
#endif

	libusb_free_device_list(list, 1);
}

void LibUSB::closeUSBDevice(USBDevice & device) {
	libusb_close(device._pUSBDeviceHandle);
	device._pUSBDeviceHandle = nullptr;
#if DEBUGGING_ON
	LOG(DEBUG3) << device.getStringID() << " closed USB device";
#endif
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
	unsigned int i, j, k = 0;
	int ret = 0;
#if DEBUGGING_ON
	LOG(DEBUG1) << device.getStringID() << " setting up usb device configuration";
#endif

	int b = -1;
	ret = libusb_get_configuration(device._pUSBDeviceHandle, &b);
	if ( this->USBError(ret) )
		throw GLogiKExcept("libusb get_configuration error");

#if DEBUGGING_ON
	LOG(DEBUG3) << device.getStringID() << " current active configuration value : " << b;
#endif

	if ( b == (int)(_expectedDescriptorsValues.bConfigurationValue) ) {
#if DEBUGGING_ON
		LOG(INFO) << device.getStringID() << " current active configuration value matches the wanted value, skipping configuration";
#endif
		return;
	}

#if DEBUGGING_ON
	LOG(DEBUG2) << device.getStringID() << " wanted configuration : " << (int)(_expectedDescriptorsValues.bConfigurationValue);
	LOG(DEBUG2) << device.getStringID() << " will try to set the active configuration to the wanted value";
#endif

	/* have to detach all interfaces first */

	libusb_device_descriptor deviceDescriptor;
	ret = libusb_get_device_descriptor(device._pUSBDevice, &deviceDescriptor);
	if ( this->USBError(ret) )
		throw GLogiKExcept("libusb get_device_descriptor failure");

	for (i = 0; i < to_uint(deviceDescriptor.bNumConfigurations); i++) {
		/* configuration descriptor */
		libusb_config_descriptor * configDescriptor = nullptr;
		ret = libusb_get_config_descriptor(device._pUSBDevice, i, &configDescriptor);
		if ( this->USBError(ret) ) {
			std::ostringstream buffer(std::ios_base::app);
			buffer << "get_config_descriptor failure with index : " << i;
			GKSysLog(LOG_ERR, ERROR, buffer.str());
			continue;
		}

		for (j = 0; j < to_uint(configDescriptor->bNumInterfaces); j++) {
			const libusb_interface *iface = &(configDescriptor->interface[j]);

			for (k = 0; k < to_uint(iface->num_altsetting); k++) {
				/* interface alt_setting descriptor */
				const libusb_interface_descriptor * asDescriptor = &(iface->altsetting[k]);

				int numInt = (int)asDescriptor->bInterfaceNumber;

				try {
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
	LOG(DEBUG2) << device.getStringID() << " checking current active configuration";
#endif
	ret = libusb_set_configuration(device._pUSBDeviceHandle, (int)_expectedDescriptorsValues.bConfigurationValue);
	if ( this->USBError(ret) ) {
		throw GLogiKExcept("libusb set_configuration failure");
	}

	this->attachUSBDeviceInterfacesToKernelDrivers(device);
}

void LibUSB::findUSBDeviceInterface(USBDevice & device) {
	unsigned int i, j, k, l = 0;
	int ret = 0;

#if DEBUGGING_ON
	LOG(DEBUG1) << device.getStringID() << " trying to find expected interface";
#endif

	libusb_device_descriptor deviceDescriptor;
	ret = libusb_get_device_descriptor(device._pUSBDevice, &deviceDescriptor);
	if ( this->USBError(ret) )
		throw GLogiKExcept("libusb get_device_descriptor failure");

#if DEBUGGING_ON
	LOG(DEBUG2) << "device has " << to_uint(deviceDescriptor.bNumConfigurations)
				<< " possible configuration(s)";

	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "device descriptor";
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "bLength            : " << to_uint(deviceDescriptor.bLength);
	LOG(DEBUG4) << "bDescriptorType    : " << to_uint(deviceDescriptor.bDescriptorType);
	LOG(DEBUG4) << "bcdUSB             : " << std::bitset<16>( to_uint(deviceDescriptor.bcdUSB) ).to_string();
	LOG(DEBUG4) << "bDeviceClass       : " << to_uint(deviceDescriptor.bDeviceClass);
	LOG(DEBUG4) << "bDeviceSubClass    : " << to_uint(deviceDescriptor.bDeviceSubClass);
	LOG(DEBUG4) << "bDeviceProtocol    : " << to_uint(deviceDescriptor.bDeviceProtocol);
	LOG(DEBUG4) << "bMaxPacketSize0    : " << to_uint(deviceDescriptor.bMaxPacketSize0);
	LOG(DEBUG4) << "idVendor           : " << std::hex << to_uint(deviceDescriptor.idVendor);
	LOG(DEBUG4) << "idProduct          : " << std::hex << to_uint(deviceDescriptor.idProduct);
	LOG(DEBUG4) << "bcdDevice          : " << std::bitset<16>( to_uint(deviceDescriptor.bcdDevice) ).to_string();
	LOG(DEBUG4) << "iManufacturer      : " << to_uint(deviceDescriptor.iManufacturer);
	LOG(DEBUG4) << "iProduct           : " << to_uint(deviceDescriptor.iProduct);
	LOG(DEBUG4) << "iSerialNumber      : " << to_uint(deviceDescriptor.iSerialNumber);
	LOG(DEBUG4) << "bNumConfigurations : " << to_uint(deviceDescriptor.bNumConfigurations);
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "--";
#endif

	for (i = 0; i < to_uint(deviceDescriptor.bNumConfigurations); i++) {
		libusb_config_descriptor * configDescriptor = nullptr;
		ret = libusb_get_config_descriptor(device._pUSBDevice, i, &configDescriptor);
		if ( this->USBError(ret) ) {
			std::ostringstream buffer(std::ios_base::app);
			buffer << "get_config_descriptor failure with index : " << i;
			GKSysLog(LOG_ERR, ERROR, buffer.str());
			continue;
		}

#if DEBUGGING_ON
		LOG(DEBUG2) << "configuration " << to_uint(configDescriptor->bConfigurationValue)
					<< " has " << to_uint(configDescriptor->bNumInterfaces) << " interface(s)";

		LOG(DEBUG4) << "--";
		LOG(DEBUG4) << "config descriptor";
		LOG(DEBUG4) << "--";
		LOG(DEBUG4) << "bLength             : " << to_uint(configDescriptor->bLength);
		LOG(DEBUG4) << "bDescriptorType     : " << to_uint(configDescriptor->bDescriptorType);
		LOG(DEBUG4) << "wTotalLength        : " << to_uint(configDescriptor->wTotalLength);
		LOG(DEBUG4) << "bNumInterfaces      : " << to_uint(configDescriptor->bNumInterfaces);
		LOG(DEBUG4) << "bConfigurationValue : " << to_uint(configDescriptor->bConfigurationValue);
		LOG(DEBUG4) << "iConfiguration      : " << to_uint(configDescriptor->iConfiguration);
		LOG(DEBUG4) << "bmAttributes        : " << to_uint(configDescriptor->bmAttributes);
		LOG(DEBUG4) << "MaxPower            : " << to_uint(configDescriptor->MaxPower);
		/* TODO extra */
		LOG(DEBUG4) << "extra_length        : " << static_cast<int>(configDescriptor->extra_length);
		LOG(DEBUG4) << "--";
		LOG(DEBUG4) << "--";
		LOG(DEBUG4) << "--";
#endif

		if ( configDescriptor->bConfigurationValue != _expectedDescriptorsValues.bConfigurationValue ) {
			libusb_free_config_descriptor( configDescriptor ); /* free */
			continue; /* skip non expected configuration */
		}

		for (j = 0; j < to_uint(configDescriptor->bNumInterfaces); j++) {
			const libusb_interface *iface = &(configDescriptor->interface[j]);
#if DEBUGGING_ON
			LOG(DEBUG2) << device.getStringID() << " interface " << j << " has " << iface->num_altsetting << " alternate settings";
#endif

			for (k = 0; k < to_uint(iface->num_altsetting); k++) {
				const libusb_interface_descriptor * asDescriptor = &(iface->altsetting[k]);

#if DEBUGGING_ON
				LOG(DEBUG3)	<< device.getStringID() << " interface " << j
							<< " alternate setting " << to_uint(asDescriptor->bAlternateSetting)
							<< " has " << to_uint(asDescriptor->bNumEndpoints) << " endpoints";

				LOG(DEBUG4) << "--";
				LOG(DEBUG4) << "interface descriptor";
				LOG(DEBUG4) << "--";
				LOG(DEBUG4) << "bLength            : " << to_uint(asDescriptor->bLength);
				LOG(DEBUG4) << "bDescriptorType    : " << to_uint(asDescriptor->bDescriptorType);
				LOG(DEBUG4) << "bInterfaceNumber   : " << to_uint(asDescriptor->bInterfaceNumber);
				LOG(DEBUG4) << "bAlternateSetting  : " << to_uint(asDescriptor->bAlternateSetting);
				LOG(DEBUG4) << "bNumEndpoints      : " << to_uint(asDescriptor->bNumEndpoints);
				LOG(DEBUG4) << "bInterfaceClass    : " << to_uint(asDescriptor->bInterfaceClass);
				LOG(DEBUG4) << "bInterfaceSubClass : " << to_uint(asDescriptor->bInterfaceSubClass);
				LOG(DEBUG4) << "bInterfaceProtocol : " << to_uint(asDescriptor->bInterfaceProtocol);
				LOG(DEBUG4) << "iInterface         : " << to_uint(asDescriptor->iInterface);
				/* TODO extra */
				LOG(DEBUG4) << "extra_length       : " << static_cast<int>(asDescriptor->extra_length);
				LOG(DEBUG4) << "--";
				LOG(DEBUG4) << "--";
				LOG(DEBUG4) << "--";
#endif

				if ( asDescriptor->bInterfaceNumber != _expectedDescriptorsValues.bInterfaceNumber) {
					continue; /* skip non expected interface */
				}

				if ( asDescriptor->bAlternateSetting != _expectedDescriptorsValues.bAlternateSetting) {
					continue; /* skip non expected alternate setting */
				}

				if( asDescriptor->bInterfaceClass != LIBUSB_CLASS_HID ) {
#if DEBUGGING_ON
					LOG(WARNING) << "interface " << j << " alternate settings " << k << " is not for HID device, skipping it";
#endif
					continue; /* sanity check */
				}

				if ( asDescriptor->bNumEndpoints != _expectedDescriptorsValues.bNumEndpoints) {
#if DEBUGGING_ON
					LOG(WARNING) << "skipping settings. numEndpoints: " << to_uint(asDescriptor->bNumEndpoints)
							<< " expected: " << to_uint(_expectedDescriptorsValues.bNumEndpoints);
#endif
					libusb_free_config_descriptor( configDescriptor ); /* free */
					throw GLogiKExcept("num_endpoints does not match");
				}

				/* specs found */
#if DEBUGGING_ON
				LOG(DEBUG1) << device.getStringID() << " found the expected interface, keep going on this road";
#endif

				int numInt = (int)asDescriptor->bInterfaceNumber;

				try {
					this->detachKernelDriverFromUSBDeviceInterface(device, numInt);
				}
				catch ( const GLogiKExcept & e ) {
					libusb_free_config_descriptor( configDescriptor ); /* free */
					throw;
				}

				/* claiming interface */
#if DEBUGGING_ON
				LOG(DEBUG1) << device.getStringID() << " trying to claim interface " << numInt;
#endif
				ret = libusb_claim_interface(device._pUSBDeviceHandle, numInt);	/* claiming */
				if( this->USBError(ret) ) {
					libusb_free_config_descriptor( configDescriptor ); /* free */
					throw GLogiKExcept("error claiming interface");
				}
				device._toRelease.push_back(numInt);	/* claimed */

				/* once that the interface is claimed, check that the right configuration is set */
#if DEBUGGING_ON
				LOG(DEBUG1) << device.getStringID() << " checking current active configuration";
#endif
				int b = -1;
				ret = libusb_get_configuration(device._pUSBDeviceHandle, &b);
				if ( this->USBError(ret) ) {
					libusb_free_config_descriptor( configDescriptor ); /* free */
					throw GLogiKExcept("libusb get_configuration error");
				}

#if DEBUGGING_ON
				LOG(DEBUG2) << device.getStringID() << " current active configuration value : " << b;
#endif
				if ( b != (int)(_expectedDescriptorsValues.bConfigurationValue) ) {
					libusb_free_config_descriptor( configDescriptor ); /* free */
					std::ostringstream buffer(std::ios_base::app);
					buffer << "wrong configuration value : " << b;
					GKSysLog(LOG_ERR, ERROR, buffer.str());
					throw GLogiKExcept(buffer.str());
				}

				for (l = 0; l < to_uint(asDescriptor->bNumEndpoints); l++) {
					const libusb_endpoint_descriptor * ep = &(asDescriptor->endpoint[l]);

					/* storing endpoint for later usage */
					// FIXME no usage ?
					device._USBEndpoints.push_back(*ep);

					unsigned int addr = to_uint(ep->bEndpointAddress);
					if( addr & LIBUSB_ENDPOINT_IN ) { /* In: device-to-host */
#if DEBUGGING_ON
						LOG(DEBUG3) << "found [Keys] endpoint, address 0x" << std::hex << addr
									<< " MaxPacketSize " << to_uint(ep->wMaxPacketSize);
#endif
						device._keysEndpoint = addr & 0xff;
					}
					else { /* Out: host-to-device */
#if DEBUGGING_ON
						LOG(DEBUG3) << "found [LCD] endpoint, address 0x" << std::hex << addr
									<< " MaxPacketSize " << to_uint(ep->wMaxPacketSize);
#endif
						device._LCDEndpoint = addr & 0xff;
					}

#if DEBUGGING_ON
					LOG(DEBUG3) << "int. " << j << " alt_s. " << to_uint(asDescriptor->bAlternateSetting)
								<< " endpoint " << l;

					LOG(DEBUG4) << "--";
					LOG(DEBUG4) << "endpoint descriptor";
					LOG(DEBUG4) << "--";
					LOG(DEBUG4) << "bLength          : " << to_uint(ep->bLength);
					LOG(DEBUG4) << "bDescriptorType  : " << to_uint(ep->bDescriptorType);
					LOG(DEBUG4) << "bEndpointAddress : " << to_uint(ep->bEndpointAddress);
					LOG(DEBUG4) << "bmAttributes     : " << to_uint(ep->bmAttributes);
					LOG(DEBUG4) << "wMaxPacketSize   : " << to_uint(ep->wMaxPacketSize);
					LOG(DEBUG4) << "bInterval        : " << to_uint(ep->bInterval);
					LOG(DEBUG4) << "bRefresh         : " << to_uint(ep->bRefresh);
					LOG(DEBUG4) << "bSynchAddress    : " << to_uint(ep->bSynchAddress);
					/* TODO extra */
					LOG(DEBUG4) << "extra_length     : " << static_cast<int>(ep->extra_length);
					LOG(DEBUG4) << "--";
					LOG(DEBUG4) << "--";
					LOG(DEBUG4) << "--";
#endif
				}

				if(device._keysEndpoint == 0) {
					libusb_free_config_descriptor( configDescriptor ); /* free */
					const std::string err("[Keys] endpoint not found");
					GKSysLog(LOG_ERR, ERROR, err);
					throw GLogiKExcept(err);
				}

#if DEBUGGING_ON
				LOG(INFO) << device.getStringID() << " all done ! " << device.getName() << " interface " << numInt
							<< " opened and ready for I/O transfers";
#endif

			} /* for ->num_altsetting */
		} /* for ->bNumInterfaces */

		libusb_free_config_descriptor( configDescriptor ); /* free */
	} /* for .bNumConfigurations */

}

void LibUSB::releaseUSBDeviceInterfaces(USBDevice & device) {
	int ret = 0;
	for(auto it = device._toRelease.begin(); it != device._toRelease.end();) {
		int numInt = (*it);
#if DEBUGGING_ON
		LOG(DEBUG1) << "trying to release claimed interface " << numInt;
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

void LibUSB::sendControlRequest(
	USBDevice & device,
	uint16_t wValue,
	uint16_t wIndex,
	unsigned char * data,
	uint16_t wLength)
{
	yield_for(std::chrono::microseconds(100));

	int ret = 0;
	{
		std::lock_guard<std::mutex> lock(device._libUSBMutex);
		ret = libusb_control_transfer( device._pUSBDeviceHandle,
			LIBUSB_ENDPOINT_OUT|LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,
			LIBUSB_REQUEST_SET_CONFIGURATION, /* 0x09 */
			wValue, wIndex, data, wLength, 10000 );
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
	yield_for(std::chrono::microseconds(100));

	int ret = 0;
	{
		std::lock_guard<std::mutex> lock(device._libUSBMutex);
		ret = libusb_interrupt_transfer(
			device._pUSBDeviceHandle,
			device._keysEndpoint,
			static_cast<unsigned char*>(device._pressedKeys),
			this->getKeysInterruptBufferMaxLength(),
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
	unsigned char* buffer,
	int bufferLength,
	unsigned int timeout)
{
	int ret = 0;
	{
		std::lock_guard<std::mutex> lock(device._libUSBMutex);
		ret = libusb_interrupt_transfer(
			device._pUSBDeviceHandle,
			device._LCDEndpoint,
			buffer,
			bufferLength,
			&(device._lastLCDInterruptTransferLength),
			timeout
		);
	}

#if DEBUGGING_ON
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

void LibUSB::attachUSBDeviceInterfacesToKernelDrivers(USBDevice & device) {
	int ret = 0;
	for(auto it = device._toAttach.begin(); it != device._toAttach.end();) {
		int numInt = (*it);
#if DEBUGGING_ON
		LOG(DEBUG1) << device.getStringID() << " trying to attach kernel driver to interface " << numInt;
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
		LOG(DEBUG1) << device.getStringID() << " kernel driver currently attached to the interface " << numInt << ", trying to detach it";
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
		LOG(DEBUG1) << device.getStringID() << " interface " << numInt << " is currently free :)";
#endif
	}
}

} // namespace GLogiK

