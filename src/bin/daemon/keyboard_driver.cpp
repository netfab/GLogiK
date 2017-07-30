/*
 *
 *	This file is part of GLogiK project.
 *	GLogiKd, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2017  Fabrice Delliaux <netbox253@gmail.com>
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

#include <iostream>
#include <bitset>

#include <config.h>

#include <syslog.h>

#include "exception.h"
#include "include/log.h"

#include "keyboard_driver.h"


namespace GLogiKd
{

bool KeyboardDriver::libusb_status_ = false;
uint8_t KeyboardDriver::drivers_cnt_ = 0;

KeyboardDriver::KeyboardDriver() : buffer_("", std::ios_base::app), context_(nullptr), reattach_driver_(false) {
	this->expected_usb_descriptors_ = { 0, 0, 0 };
	KeyboardDriver::drivers_cnt_++;
}

KeyboardDriver::~KeyboardDriver() {
	KeyboardDriver::drivers_cnt_--;
	if (KeyboardDriver::libusb_status_ and KeyboardDriver::drivers_cnt_ == 0)
		this->closeLibusb();
}

std::vector<KeyboardDevice> KeyboardDriver::getSupportedDevices(void) const {
	return this->supported_devices_;
}

void KeyboardDriver::initializeLibusb(InitializedDevice & current_device) {
	LOG(DEBUG4) << "initializing libusb";
	int ret_value = libusb_init( &(this->context_) );
	if ( this->handleLibusbError(ret_value) ) {
		throw GLogiKExcept("libusb initialization failure");
	}

	KeyboardDriver::libusb_status_ = true;

	libusb_device **list;
	int num_devices = libusb_get_device_list(this->context_, &(list));
	if( num_devices < 0 ) {
		this->handleLibusbError(num_devices);
		throw GLogiKExcept("error getting USB devices list");
	}

	for (int idx = 0; idx < num_devices; ++idx) {
		current_device.usb_device = list[idx];
		if( (int)libusb_get_bus_number(current_device.usb_device) == current_device.bus and
			(int)libusb_get_device_address(current_device.usb_device) == current_device.num ) {
			break;
		}
		current_device.usb_device = nullptr;
	}

	if( current_device.usb_device == nullptr ) {
		this->buffer_.str("libusb cannot find device ");
		this->buffer_ << current_device.num << " on bus " << current_device.bus;
		libusb_free_device_list(list, 1);
		throw GLogiKExcept(this->buffer_.str());
	}

	LOG(DEBUG3) << "libusb found device " << (unsigned int)current_device.num
				<< " on bus " << (unsigned int)current_device.bus;

	ret_value = libusb_open( current_device.usb_device, &(current_device.usb_handle) );
	if( this->handleLibusbError(ret_value) ) {
		libusb_free_device_list(list, 1);
		throw GLogiKExcept("opening device failure");
	}

	libusb_free_device_list(list, 1);
}

void KeyboardDriver::closeLibusb(void) {
	LOG(DEBUG4) << "closing libusb";
	unsigned int s = this->initialized_devices_.size();
	if( s != 0 ) { /* sanity check */
		LOG(WARNING) << "closing libusb while " << s << " device(s) not closed !";
	}
	libusb_exit(this->context_);
	KeyboardDriver::libusb_status_ = false;
}

int KeyboardDriver::handleLibusbError(int error_code) {
	switch(error_code) {
		case LIBUSB_SUCCESS:
			break;
		default:
			this->buffer_.str("libusb error ( ");
			this->buffer_ << libusb_error_name(error_code) << ") : "
					<< libusb_strerror((libusb_error)error_code);
			LOG(ERROR) << this->buffer_.str();
			syslog(LOG_ERR, this->buffer_.str().c_str());
			break;
	}

	return error_code;
}

void KeyboardDriver::initializeDevice(const KeyboardDevice &device, const uint8_t bus, const uint8_t num) {
	LOG(DEBUG3) << "trying to initialize " << device.name << "("
				<< device.vendor_id << ":" << device.product_id << "), device "
				<< (unsigned int)num << " on bus " << (unsigned int)bus;

	InitializedDevice current_device = { device, bus, num, nullptr, nullptr, nullptr };

	this->initializeLibusb(current_device); /* device opened */

	try {
		this->setDeviceConfiguration( current_device );

/*
		int b = -1;
		ret = libusb_get_configuration(current_device.usb_handle, &b);
		if ( this->handleLibusbError(ret) )
			throw GLogiKExcept("libusb get_configuration error");

		LOG(INFO) << "current active configuration value : " << b;
		if ( b != (int)(this->expected_usb_descriptors_.b_configuration_value) ) {
			LOG(INFO) << "wanted configuration : " << (int)(this->expected_usb_descriptors_.b_configuration_value);
			LOG(INFO) << "will try to set the active configuration to the wanted value";
			this->setDeviceConfiguration( current_device );
		}
*/

		/* virtual keyboard */
		this->buffer_.str("Virtual ");
		this->buffer_ << device.name << " b" << (unsigned int)bus << "d" << (unsigned int)num;

		current_device.virtual_keyboard = this->initializeVirtualKeyboard(this->buffer_.str().c_str());
	}
	catch ( const GLogiKExcept & e ) {
		libusb_close( current_device.usb_handle );
		throw;
	}
	catch (const std::bad_alloc& e) { /* handle new VirtualKeyboard() failure */
		libusb_close( current_device.usb_handle );
		throw GLogiKExcept("virtual keyboard allocation failure");
	}

	this->initialized_devices_.push_back( current_device );
}

void KeyboardDriver::setDeviceConfiguration(const InitializedDevice & current_device) {
	unsigned int i, j, k = 0;
	int ret = 0;

	struct libusb_device_descriptor device_descriptor;
	ret = libusb_get_device_descriptor(current_device.usb_device, &device_descriptor);
	if ( this->handleLibusbError(ret) )
		throw GLogiKExcept("libusb get_device_descriptor failure");

#if DEBUGGING_ON
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "device descriptor";
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "bLength            : " << (unsigned int)device_descriptor.bLength;
	LOG(DEBUG4) << "bDescriptorType    : " << (unsigned int)device_descriptor.bDescriptorType;
	LOG(DEBUG4) << "bcdUSB             : " << std::bitset<16>( (unsigned int)device_descriptor.bcdUSB ).to_string();
	LOG(DEBUG4) << "bDeviceClass       : " << (unsigned int)device_descriptor.bDeviceClass;
	LOG(DEBUG4) << "bDeviceSubClass    : " << (unsigned int)device_descriptor.bDeviceSubClass;
	LOG(DEBUG4) << "bDeviceProtocol    : " << (unsigned int)device_descriptor.bDeviceProtocol;
	LOG(DEBUG4) << "bMaxPacketSize0    : " << (unsigned int)device_descriptor.bMaxPacketSize0;
	LOG(DEBUG4) << "idVendor           : " << std::hex << (unsigned int)device_descriptor.idVendor;
	LOG(DEBUG4) << "idProduct          : " << std::hex << (unsigned int)device_descriptor.idProduct;
	LOG(DEBUG4) << "bcdDevice          : " << std::bitset<16>( (unsigned int)device_descriptor.bcdDevice ).to_string();
	LOG(DEBUG4) << "iManufacturer      : " << (unsigned int)device_descriptor.iManufacturer;
	LOG(DEBUG4) << "iProduct           : " << (unsigned int)device_descriptor.iProduct;
	LOG(DEBUG4) << "iSerialNumber      : " << (unsigned int)device_descriptor.iSerialNumber;
	LOG(DEBUG4) << "bNumConfigurations : " << (unsigned int)device_descriptor.bNumConfigurations;
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "--";

	LOG(INFO) << "device has " << (unsigned int)device_descriptor.bNumConfigurations
				<< " possible configuration(s)";
#endif

	for (i = 0; i < (unsigned int)device_descriptor.bNumConfigurations; i++) {
		struct libusb_config_descriptor * config_descriptor = nullptr;
		ret = libusb_get_config_descriptor(current_device.usb_device, i, &config_descriptor);
		if ( this->handleLibusbError(ret) ) {
			this->buffer_.str("get_config_descriptor failure with index : ");
			this->buffer_ << i;
			LOG(ERROR) << this->buffer_.str();
			syslog(LOG_ERR, this->buffer_.str().c_str());
			continue;
		}

#if DEBUGGING_ON
		LOG(DEBUG1) << "configuration " << (unsigned int)config_descriptor->bConfigurationValue
					<< " has " << (unsigned int)config_descriptor->bNumInterfaces << " interface(s)";

		LOG(DEBUG4) << "--";
		LOG(DEBUG4) << "config descriptor";
		LOG(DEBUG4) << "--";
		LOG(DEBUG4) << "bLength             : " << (unsigned int)config_descriptor->bLength;
		LOG(DEBUG4) << "bDescriptorType     : " << (unsigned int)config_descriptor->bDescriptorType;
		LOG(DEBUG4) << "wTotalLength        : " << (unsigned int)config_descriptor->wTotalLength;
		LOG(DEBUG4) << "bNumInterfaces      : " << (unsigned int)config_descriptor->bNumInterfaces;
		LOG(DEBUG4) << "bConfigurationValue : " << (unsigned int)config_descriptor->bConfigurationValue;
		LOG(DEBUG4) << "iConfiguration      : " << (unsigned int)config_descriptor->iConfiguration;
		LOG(DEBUG4) << "bmAttributes        : " << (unsigned int)config_descriptor->bmAttributes;
		LOG(DEBUG4) << "MaxPower            : " << (unsigned int)config_descriptor->MaxPower;
		/* TODO extra extra_length */
		LOG(DEBUG4) << "--";
		LOG(DEBUG4) << "--";
		LOG(DEBUG4) << "--";
#endif

		if ( config_descriptor->bConfigurationValue != this->expected_usb_descriptors_.b_configuration_value ) {
			continue; /* skip non expected configuration */
		}

		for (j = 0; j < (unsigned int)config_descriptor->bNumInterfaces; j++) {
			const struct libusb_interface *iface = &(config_descriptor->interface[j]);
			LOG(DEBUG2) << "interface " << j << " has " << iface->num_altsetting << " alternate settings";

			for (k = 0; k < (unsigned int)iface->num_altsetting; k++) {
				const struct libusb_interface_descriptor * as_descriptor = &(iface->altsetting[k]);

#if DEBUGGING_ON
				LOG(DEBUG3) << "interface " << j << " alternate setting " << (unsigned int)as_descriptor->bAlternateSetting
							<< " has " << (unsigned int)as_descriptor->bNumEndpoints << " endpoints";

				LOG(DEBUG4) << "--";
				LOG(DEBUG4) << "interface descriptor";
				LOG(DEBUG4) << "--";
				LOG(DEBUG4) << "bLength            : " << (unsigned int)as_descriptor->bLength;
				LOG(DEBUG4) << "bDescriptorType    : " << (unsigned int)as_descriptor->bDescriptorType;
				LOG(DEBUG4) << "bInterfaceNumber   : " << (unsigned int)as_descriptor->bInterfaceNumber;
				LOG(DEBUG4) << "bAlternateSetting  : " << (unsigned int)as_descriptor->bAlternateSetting;
				LOG(DEBUG4) << "bNumEndpoints      : " << (unsigned int)as_descriptor->bNumEndpoints;
				LOG(DEBUG4) << "bInterfaceClass    : " << (unsigned int)as_descriptor->bInterfaceClass;
				LOG(DEBUG4) << "bInterfaceSubClass : " << (unsigned int)as_descriptor->bInterfaceSubClass;
				LOG(DEBUG4) << "bInterfaceProtocol : " << (unsigned int)as_descriptor->bInterfaceProtocol;
				LOG(DEBUG4) << "iInterface         : " << (unsigned int)as_descriptor->iInterface;
				/* TODO extra extra_length */
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
					LOG(DEBUG3) << "interface " << j << " alternate settings " << k << " is not for HID device, skipping it";
					continue; /* sanity check */
				}

				/* specs found */
				LOG(INFO) << "found the expected interface, keep going on this road";

				int numInt = (int)as_descriptor->bInterfaceNumber;
				ret = libusb_kernel_driver_active(current_device.usb_handle, numInt);
				if( ret < 0 ) {
					this->handleLibusbError(ret);
					throw GLogiKExcept("libusb kernel_driver_active error");
				}
				if( ret ) {
					LOG(INFO) << "kernel driver currently attached to the interface " << numInt << ", trying to detach it";
					ret = libusb_detach_kernel_driver(current_device.usb_handle, numInt); /* detaching */
					if( this->handleLibusbError(ret) ) {
						this->buffer_.str("detaching the kernel driver from USB interface ");
						this->buffer_ << numInt << " failed, this is fatal";
						LOG(ERROR) << this->buffer_.str();
						syslog(LOG_ERR, this->buffer_.str().c_str());
						throw GLogiKExcept(this->buffer_.str());
					}

					LOG(INFO) << "successfully detached the kernel driver :)";
					this->reattach_driver_ = true;
				}
				else {
					LOG(INFO) << "interface " << numInt << " is currently free :)";
				}

			}
		}

			//ret = libusb_set_configuration(current_device.usb_handle, (int)this->b_config_value_);
			//if ( this->handleLibusbError(ret) )
			//	throw GLogiKExcept("libusb set_configuration failure");
		libusb_free_config_descriptor( config_descriptor );
	}

}

void KeyboardDriver::closeDevice(const KeyboardDevice &device, const uint8_t bus, const uint8_t num) {
	LOG(DEBUG3) << "trying to close " << device.name << "("
				<< device.vendor_id << ":" << device.product_id << "), device "
				<< (unsigned int)num << " on bus " << (unsigned int)bus;

	bool found = false;
	for(auto it = this->initialized_devices_.begin(); it != this->initialized_devices_.end();) {
		if( ((*it).bus == bus) and ((*it).num == num) ) {
			delete (*it).virtual_keyboard;
			(*it).virtual_keyboard = nullptr;

			if( this->reattach_driver_ ) {
				int numInt = (int)this->expected_usb_descriptors_.b_interface_number;
				LOG(INFO) << "trying to attach kernel driver to interface " << numInt;
				int ret = libusb_attach_kernel_driver((*it).usb_handle, numInt); /* re-attach */
				if( this->handleLibusbError(ret) ) {
					this->buffer_.str("failed to reattach kernel driver to interface ");
					this->buffer_ << numInt;
					LOG(ERROR) << this->buffer_.str();
					syslog(LOG_ERR, this->buffer_.str().c_str());
				}
				else {
					LOG(INFO) << "successfully attached kernel driver :)";
				}
			}

			libusb_close( (*it).usb_handle );

			found = true;
			it = this->initialized_devices_.erase(it);
			break;
		}
		else {
			++it;
		}
	}
	if ( ! found ) {
		this->buffer_.str("device ");
		this->buffer_ << num << " on bus " << bus;
		this->buffer_ << " not found in initialized devices";
		throw GLogiKExcept(this->buffer_.str());
	}
}

VirtualKeyboard* KeyboardDriver::initializeVirtualKeyboard( const char* device_name ) {
	return new VirtualKeyboard(device_name);
}

} // namespace GLogiKd

