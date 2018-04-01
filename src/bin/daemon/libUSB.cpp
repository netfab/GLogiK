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

#include "lib/utils/utils.h"

#include "libUSB.h"

namespace GLogiK
{

using namespace NSGKUtils;

bool LibUSB::libusb_status_ = false;
uint8_t LibUSB::drivers_cnt_ = 0;
libusb_context * LibUSB::context_ = nullptr;

LibUSB::LibUSB(const int key_read_length, const DescriptorValues & values)
	:	buffer_("", std::ios_base::app), expected_usb_descriptors_(values)
{
	LibUSB::drivers_cnt_++;

	if( ! LibUSB::libusb_status_ ) {
#if DEBUGGING_ON
		LOG(DEBUG3) << "initializing libusb";
#endif
		int ret_value = libusb_init( &(this->context_) );
		if ( this->USBError(ret_value) ) {
			throw GLogiKExcept("libusb initialization failure");
		}

		LibUSB::libusb_status_ = true;
	}

	this->interrupt_buffer_max_length_ = key_read_length;

	if( key_read_length > KEYS_BUFFER_LENGTH ) {
		GKSysLog(LOG_WARNING, WARNING, "interrupt read length too large, set it to max buffer length");
		this->interrupt_buffer_max_length_ = KEYS_BUFFER_LENGTH;
	}

}

LibUSB::~LibUSB() {
	LibUSB::drivers_cnt_--;

	if (LibUSB::libusb_status_ and LibUSB::drivers_cnt_ == 0) {
#if DEBUGGING_ON
		LOG(DEBUG3) << "closing libusb";
#endif
		// FIXME
		//if( this->initialized_devices_.size() != 0 ) { // sanity check
		//	GKSysLog(LOG_WARNING, WARNING, "closing libusb with opened device(s) !");
		//}
		libusb_exit(this->context_);
		LibUSB::libusb_status_ = false;
	}
}

int LibUSB::USBError(int error_code) {
	switch(error_code) {
		case LIBUSB_SUCCESS:
			break;
		default:
			this->buffer_.str("libusb error (");
			this->buffer_ << libusb_error_name(error_code) << ") : "
					<< libusb_strerror((libusb_error)error_code);
			GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
			break;
	}

	return error_code;
}

void LibUSB::openUSBDevice(USBDevice & device) {
	libusb_device **list;
	int num_devices = libusb_get_device_list(this->context_, &(list));
	if( num_devices < 0 ) {
		this->USBError(num_devices);
		throw GLogiKExcept("error getting USB devices list");
	}

	for (int idx = 0; idx < num_devices; ++idx) {
		device.usb_device = list[idx];
		if( (int)libusb_get_bus_number(device.usb_device) == device.getBus() and
			(int)libusb_get_device_address(device.usb_device) == device.getNum() ) {
			break;
		}
		device.usb_device = nullptr;
	}

	if( device.usb_device == nullptr ) {
		this->buffer_.str("libusb cannot find device ");
		this->buffer_ << device.getNum() << " on bus " << device.getBus();
		libusb_free_device_list(list, 1);
		throw GLogiKExcept(this->buffer_.str());
	}

	int ret_value = libusb_open( device.usb_device, &(device.usb_handle) );
	if( this->USBError(ret_value) ) {
		libusb_free_device_list(list, 1);
		throw GLogiKExcept("opening device failure");
	}

#if DEBUGGING_ON
	LOG(DEBUG3) << device.getStrID() << " opened USB device";
#endif

	libusb_free_device_list(list, 1);
}

void LibUSB::closeUSBDevice(USBDevice & device) {
	libusb_close(device.usb_handle);
	device.usb_handle = nullptr;
#if DEBUGGING_ON
	LOG(DEBUG3) << device.getStrID() << " closed USB device";
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
	LOG(DEBUG1) << device.getStrID() << " setting up usb device configuration";
#endif

	int b = -1;
	ret = libusb_get_configuration(device.usb_handle, &b);
	if ( this->USBError(ret) )
		throw GLogiKExcept("libusb get_configuration error");

#if DEBUGGING_ON
	LOG(DEBUG3) << device.getStrID() << " current active configuration value : " << b;
#endif

	if ( b == (int)(this->expected_usb_descriptors_.b_configuration_value) ) {
#if DEBUGGING_ON
		LOG(INFO) << device.getStrID() << " current active configuration value matches the wanted value, skipping configuration";
#endif
		return;
	}

#if DEBUGGING_ON
	LOG(DEBUG2) << device.getStrID() << " wanted configuration : " << (int)(this->expected_usb_descriptors_.b_configuration_value);
	LOG(DEBUG2) << device.getStrID() << " will try to set the active configuration to the wanted value";
#endif

	/* have to detach all interfaces first */

	struct libusb_device_descriptor device_descriptor;
	ret = libusb_get_device_descriptor(device.usb_device, &device_descriptor);
	if ( this->USBError(ret) )
		throw GLogiKExcept("libusb get_device_descriptor failure");

	for (i = 0; i < to_uint(device_descriptor.bNumConfigurations); i++) {
		/* configuration descriptor */
		struct libusb_config_descriptor * config_descriptor = nullptr;
		ret = libusb_get_config_descriptor(device.usb_device, i, &config_descriptor);
		if ( this->USBError(ret) ) {
			this->buffer_.str("get_config_descriptor failure with index : ");
			this->buffer_ << i;
			GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
			continue;
		}

		for (j = 0; j < to_uint(config_descriptor->bNumInterfaces); j++) {
			const struct libusb_interface *iface = &(config_descriptor->interface[j]);

			for (k = 0; k < to_uint(iface->num_altsetting); k++) {
				/* interface alt_setting descriptor */
				const struct libusb_interface_descriptor * as_descriptor = &(iface->altsetting[k]);

				int numInt = (int)as_descriptor->bInterfaceNumber;

				try {
					this->detachKernelDriverFromUSBDeviceInterface(device, numInt);
				}
				catch ( const GLogiKExcept & e ) {
					libusb_free_config_descriptor( config_descriptor ); /* free */
					throw;
				}
			} /* for ->num_altsetting */
		} /* for ->bNumInterfaces */

		libusb_free_config_descriptor( config_descriptor ); /* free */
	} /* for .bNumConfigurations */

	/* trying to set configuration */
#if DEBUGGING_ON
	LOG(DEBUG2) << device.getStrID() << " checking current active configuration";
#endif
	ret = libusb_set_configuration(device.usb_handle, (int)this->expected_usb_descriptors_.b_configuration_value);
	if ( this->USBError(ret) ) {
		throw GLogiKExcept("libusb set_configuration failure");
	}

	this->attachUSBDeviceInterfacesToKernelDrivers(device);
}

void LibUSB::findUSBDeviceInterface(USBDevice & device) {
	unsigned int i, j, k, l = 0;
	int ret = 0;

#if DEBUGGING_ON
	LOG(DEBUG1) << device.getStrID() << " trying to find expected interface";
#endif

	struct libusb_device_descriptor device_descriptor;
	ret = libusb_get_device_descriptor(device.usb_device, &device_descriptor);
	if ( this->USBError(ret) )
		throw GLogiKExcept("libusb get_device_descriptor failure");

#if DEBUGGING_ON
	LOG(DEBUG2) << "device has " << to_uint(device_descriptor.bNumConfigurations)
				<< " possible configuration(s)";

	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "device descriptor";
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "bLength            : " << to_uint(device_descriptor.bLength);
	LOG(DEBUG4) << "bDescriptorType    : " << to_uint(device_descriptor.bDescriptorType);
	LOG(DEBUG4) << "bcdUSB             : " << std::bitset<16>( to_uint(device_descriptor.bcdUSB) ).to_string();
	LOG(DEBUG4) << "bDeviceClass       : " << to_uint(device_descriptor.bDeviceClass);
	LOG(DEBUG4) << "bDeviceSubClass    : " << to_uint(device_descriptor.bDeviceSubClass);
	LOG(DEBUG4) << "bDeviceProtocol    : " << to_uint(device_descriptor.bDeviceProtocol);
	LOG(DEBUG4) << "bMaxPacketSize0    : " << to_uint(device_descriptor.bMaxPacketSize0);
	LOG(DEBUG4) << "idVendor           : " << std::hex << to_uint(device_descriptor.idVendor);
	LOG(DEBUG4) << "idProduct          : " << std::hex << to_uint(device_descriptor.idProduct);
	LOG(DEBUG4) << "bcdDevice          : " << std::bitset<16>( to_uint(device_descriptor.bcdDevice) ).to_string();
	LOG(DEBUG4) << "iManufacturer      : " << to_uint(device_descriptor.iManufacturer);
	LOG(DEBUG4) << "iProduct           : " << to_uint(device_descriptor.iProduct);
	LOG(DEBUG4) << "iSerialNumber      : " << to_uint(device_descriptor.iSerialNumber);
	LOG(DEBUG4) << "bNumConfigurations : " << to_uint(device_descriptor.bNumConfigurations);
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "--";
#endif

	for (i = 0; i < to_uint(device_descriptor.bNumConfigurations); i++) {
		struct libusb_config_descriptor * config_descriptor = nullptr;
		ret = libusb_get_config_descriptor(device.usb_device, i, &config_descriptor);
		if ( this->USBError(ret) ) {
			this->buffer_.str("get_config_descriptor failure with index : ");
			this->buffer_ << i;
			GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
			continue;
		}

#if DEBUGGING_ON
		LOG(DEBUG2) << "configuration " << to_uint(config_descriptor->bConfigurationValue)
					<< " has " << to_uint(config_descriptor->bNumInterfaces) << " interface(s)";

		LOG(DEBUG4) << "--";
		LOG(DEBUG4) << "config descriptor";
		LOG(DEBUG4) << "--";
		LOG(DEBUG4) << "bLength             : " << to_uint(config_descriptor->bLength);
		LOG(DEBUG4) << "bDescriptorType     : " << to_uint(config_descriptor->bDescriptorType);
		LOG(DEBUG4) << "wTotalLength        : " << to_uint(config_descriptor->wTotalLength);
		LOG(DEBUG4) << "bNumInterfaces      : " << to_uint(config_descriptor->bNumInterfaces);
		LOG(DEBUG4) << "bConfigurationValue : " << to_uint(config_descriptor->bConfigurationValue);
		LOG(DEBUG4) << "iConfiguration      : " << to_uint(config_descriptor->iConfiguration);
		LOG(DEBUG4) << "bmAttributes        : " << to_uint(config_descriptor->bmAttributes);
		LOG(DEBUG4) << "MaxPower            : " << to_uint(config_descriptor->MaxPower);
		/* TODO extra */
		LOG(DEBUG4) << "extra_length        : " << (int)config_descriptor->extra_length;
		LOG(DEBUG4) << "--";
		LOG(DEBUG4) << "--";
		LOG(DEBUG4) << "--";
#endif

		if ( config_descriptor->bConfigurationValue != this->expected_usb_descriptors_.b_configuration_value ) {
			libusb_free_config_descriptor( config_descriptor ); /* free */
			continue; /* skip non expected configuration */
		}

		for (j = 0; j < to_uint(config_descriptor->bNumInterfaces); j++) {
			const struct libusb_interface *iface = &(config_descriptor->interface[j]);
#if DEBUGGING_ON
			LOG(DEBUG2) << device.getStrID() << " interface " << j << " has " << iface->num_altsetting << " alternate settings";
#endif

			for (k = 0; k < to_uint(iface->num_altsetting); k++) {
				const struct libusb_interface_descriptor * as_descriptor = &(iface->altsetting[k]);

#if DEBUGGING_ON
				LOG(DEBUG3) << device.getStrID() << " interface " << j << " alternate setting " << to_uint(as_descriptor->bAlternateSetting)
							<< " has " << to_uint(as_descriptor->bNumEndpoints) << " endpoints";

				LOG(DEBUG4) << "--";
				LOG(DEBUG4) << "interface descriptor";
				LOG(DEBUG4) << "--";
				LOG(DEBUG4) << "bLength            : " << to_uint(as_descriptor->bLength);
				LOG(DEBUG4) << "bDescriptorType    : " << to_uint(as_descriptor->bDescriptorType);
				LOG(DEBUG4) << "bInterfaceNumber   : " << to_uint(as_descriptor->bInterfaceNumber);
				LOG(DEBUG4) << "bAlternateSetting  : " << to_uint(as_descriptor->bAlternateSetting);
				LOG(DEBUG4) << "bNumEndpoints      : " << to_uint(as_descriptor->bNumEndpoints);
				LOG(DEBUG4) << "bInterfaceClass    : " << to_uint(as_descriptor->bInterfaceClass);
				LOG(DEBUG4) << "bInterfaceSubClass : " << to_uint(as_descriptor->bInterfaceSubClass);
				LOG(DEBUG4) << "bInterfaceProtocol : " << to_uint(as_descriptor->bInterfaceProtocol);
				LOG(DEBUG4) << "iInterface         : " << to_uint(as_descriptor->iInterface);
				/* TODO extra */
				LOG(DEBUG4) << "extra_length       : " << (int)as_descriptor->extra_length;
				LOG(DEBUG4) << "--";
				LOG(DEBUG4) << "--";
				LOG(DEBUG4) << "--";
#endif

				if ( as_descriptor->bInterfaceNumber != this->expected_usb_descriptors_.b_interface_number) {
					continue; /* skip non expected interface */
				}

				if ( as_descriptor->bAlternateSetting != this->expected_usb_descriptors_.b_alternate_setting) {
					continue; /* skip non expected alternate setting */
				}

				if( as_descriptor->bInterfaceClass != LIBUSB_CLASS_HID ) {
#if DEBUGGING_ON
					LOG(WARNING) << "interface " << j << " alternate settings " << k << " is not for HID device, skipping it";
#endif
					continue; /* sanity check */
				}

				if ( as_descriptor->bNumEndpoints != this->expected_usb_descriptors_.b_num_endpoints) {
#if DEBUGGING_ON
					LOG(WARNING) << "skipping settings. num_endpoints: " << to_uint(as_descriptor->bNumEndpoints)
							<< " expected: " << to_uint(this->expected_usb_descriptors_.b_num_endpoints);
#endif
					libusb_free_config_descriptor( config_descriptor ); /* free */
					throw GLogiKExcept("num_endpoints does not match");
				}

				/* specs found */
#if DEBUGGING_ON
				LOG(DEBUG1) << device.getStrID() << " found the expected interface, keep going on this road";
#endif

				int numInt = (int)as_descriptor->bInterfaceNumber;

				try {
					this->detachKernelDriverFromUSBDeviceInterface(device, numInt);
				}
				catch ( const GLogiKExcept & e ) {
					libusb_free_config_descriptor( config_descriptor ); /* free */
					throw;
				}

				/* claiming interface */
#if DEBUGGING_ON
				LOG(DEBUG1) << device.getStrID() << " trying to claim interface " << numInt;
#endif
				ret = libusb_claim_interface(device.usb_handle, numInt);	/* claiming */
				if( this->USBError(ret) ) {
					libusb_free_config_descriptor( config_descriptor ); /* free */
					throw GLogiKExcept("error claiming interface");
				}
				device.to_release.push_back(numInt);	/* claimed */

				/* once that the interface is claimed, check that the right configuration is set */
#if DEBUGGING_ON
				LOG(DEBUG1) << device.getStrID() << " checking current active configuration";
#endif
				int b = -1;
				ret = libusb_get_configuration(device.usb_handle, &b);
				if ( this->USBError(ret) ) {
					libusb_free_config_descriptor( config_descriptor ); /* free */
					throw GLogiKExcept("libusb get_configuration error");
				}

#if DEBUGGING_ON
				LOG(DEBUG2) << device.getStrID() << " current active configuration value : " << b;
#endif
				if ( b != (int)(this->expected_usb_descriptors_.b_configuration_value) ) {
					libusb_free_config_descriptor( config_descriptor ); /* free */
					this->buffer_.str("wrong configuration value ");
					this->buffer_ << b << ", what a pity :(";
					GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
					throw GLogiKExcept("wrong configuration value");
				}

				for (l = 0; l < to_uint(as_descriptor->bNumEndpoints); l++) {
					const struct libusb_endpoint_descriptor * ep = &(as_descriptor->endpoint[l]);

					/* storing endpoint for later usage */
					device.endpoints.push_back(*ep);

					/* In: device-to-host */
					if( to_uint(ep->bEndpointAddress) & LIBUSB_ENDPOINT_IN ) {
						unsigned int addr = to_uint(ep->bEndpointAddress);
#if DEBUGGING_ON
						LOG(DEBUG3) << "found [Keys] endpoint, address 0x" << std::hex << addr
									<< " MaxPacketSize " << to_uint(ep->wMaxPacketSize);
#endif
						device.keys_endpoint = addr & 0xff;
					}

#if DEBUGGING_ON
					LOG(DEBUG3) << "int. " << j << " alt_s. " << to_uint(as_descriptor->bAlternateSetting)
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
					LOG(DEBUG4) << "extra_length     : " << (int)ep->extra_length;
					LOG(DEBUG4) << "--";
					LOG(DEBUG4) << "--";
					LOG(DEBUG4) << "--";
#endif
				}

				if(device.keys_endpoint == 0) {
					libusb_free_config_descriptor( config_descriptor ); /* free */
					const std::string err("[Keys] endpoint not found");
					GKSysLog(LOG_ERR, ERROR, err);
					throw GLogiKExcept(err);
				}

#if DEBUGGING_ON
				LOG(INFO) << device.getStrID() << " all done ! " << device.getName() << " interface " << numInt
							<< " opened and ready for I/O transfers";
#endif

			} /* for ->num_altsetting */
		} /* for ->bNumInterfaces */

		libusb_free_config_descriptor( config_descriptor ); /* free */
	} /* for .bNumConfigurations */

}

void LibUSB::releaseUSBDeviceInterfaces(USBDevice & device) {
	int ret = 0;
	for(auto it = device.to_release.begin(); it != device.to_release.end();) {
		int numInt = (*it);
#if DEBUGGING_ON
		LOG(DEBUG1) << "trying to release claimed interface " << numInt;
#endif
		ret = libusb_release_interface(device.usb_handle, numInt); /* release */
		if( this->USBError(ret) ) {
			this->buffer_.str("failed to release interface ");
			this->buffer_ << numInt;
			GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
		}
		it++;
	}
	device.to_release.clear();

	this->attachUSBDeviceInterfacesToKernelDrivers(device);
}

void LibUSB::sendControlRequest(
	const USBDevice & device,
	uint16_t wValue,
	uint16_t wIndex,
	unsigned char * data,
	uint16_t wLength)
{
	int ret = libusb_control_transfer( device.usb_handle,
		LIBUSB_ENDPOINT_OUT|LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,
		LIBUSB_REQUEST_SET_CONFIGURATION, /* 0x09 */
		wValue, wIndex, data, wLength, 10000 );
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

int LibUSB::performInterruptTransfer(
	USBDevice & device,
	unsigned int timeout
) {
	int ret = libusb_interrupt_transfer(
		device.usb_handle,
		device.keys_endpoint,
		(unsigned char*)device.keys_buffer,
		this->interrupt_buffer_max_length_,
		&(device.last_interrupt_transfer_length),
		timeout
	);

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
	for(auto it = device.to_attach.begin(); it != device.to_attach.end();) {
		int numInt = (*it);
#if DEBUGGING_ON
		LOG(DEBUG1) << device.getStrID() << " trying to attach kernel driver to interface " << numInt;
#endif
		ret = libusb_attach_kernel_driver(device.usb_handle, numInt); /* attaching */
		if( this->USBError(ret) ) {
			this->buffer_.str("failed to attach kernel driver to interface ");
			this->buffer_ << numInt;
			GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
		}
		it++;
	}
	device.to_attach.clear();
}

void LibUSB::detachKernelDriverFromUSBDeviceInterface(USBDevice & device, int numInt) {
	int ret = libusb_kernel_driver_active(device.usb_handle, numInt);
	if( ret < 0 ) {
		this->USBError(ret);
		throw GLogiKExcept("libusb kernel_driver_active error");
	}
	if( ret ) {
#if DEBUGGING_ON
		LOG(DEBUG1) << device.getStrID() << " kernel driver currently attached to the interface " << numInt << ", trying to detach it";
#endif
		ret = libusb_detach_kernel_driver(device.usb_handle, numInt); /* detaching */
		if( this->USBError(ret) ) {
			this->buffer_.str("detaching the kernel driver from USB interface ");
			this->buffer_ << numInt << " failed, this is fatal";
			GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
			throw GLogiKExcept(this->buffer_.str());
		}

		device.to_attach.push_back(numInt);	/* detached */
	}
	else {
#if DEBUGGING_ON
		LOG(DEBUG1) << device.getStrID() << " interface " << numInt << " is currently free :)";
#endif
	}
}

} // namespace GLogiK

