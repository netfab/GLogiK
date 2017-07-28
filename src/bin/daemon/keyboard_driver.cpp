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

#include "exception.h"
#include "include/log.h"

#include "keyboard_driver.h"


namespace GLogiKd
{

bool KeyboardDriver::libusb_status_ = false;
uint8_t KeyboardDriver::drivers_cnt_ = 0;

KeyboardDriver::KeyboardDriver() : buffer_("", std::ios_base::app), context_(nullptr) {
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
			LOG(ERROR) << "handleLibusbError: (" <<  libusb_error_name(error_code) << ") "
						<< libusb_strerror((libusb_error)error_code);
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

	int ret = 0;

	try {
		int b = -1;
		int * pB = &b;
		ret = libusb_get_configuration(current_device.usb_handle, pB);
		if( ret == 0 ) {
			LOG(DEBUG3) << "current conf: " << b;
		}
		else {
			this->handleLibusbError(ret);
			throw GLogiKExcept("libusb get_configuration error");
		}

		struct libusb_device_descriptor device_descriptor;
		ret = libusb_get_device_descriptor(current_device.usb_device, &device_descriptor);
		if( ret != 0 ) {
			this->handleLibusbError(ret);
			throw GLogiKExcept("libusb get_device_descriptor failure");
		}

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

		unsigned int i = 0;
		for ( i = 0; i < (unsigned int)device_descriptor.bNumConfigurations; i++) {
			struct libusb_config_descriptor * config_descriptor = nullptr;
			ret = libusb_get_config_descriptor(current_device.usb_device, i, &config_descriptor);
			if ( ret != 0 ) {
				this->handleLibusbError(ret);
				this->buffer_.str("get_config_descriptor failure with index : ");
				this->buffer_ << i;
				LOG(ERROR) << this->buffer_.str();
				// FIXME syslog
			}
			else {
				LOG(DEBUG3) << "--";
				LOG(DEBUG3) << "config descriptor";
				LOG(DEBUG3) << "--";
				LOG(DEBUG3) << "bLength             : " << (unsigned int)config_descriptor->bLength;
				LOG(DEBUG3) << "bDescriptorType     : " << (unsigned int)config_descriptor->bDescriptorType;
				LOG(DEBUG3) << "wTotalLength        : " << (unsigned int)config_descriptor->wTotalLength;
				LOG(DEBUG3) << "bNumInterfaces      : " << (unsigned int)config_descriptor->bNumInterfaces;
				LOG(DEBUG3) << "bConfigurationValue : " << (unsigned int)config_descriptor->bConfigurationValue;
				LOG(DEBUG3) << "iConfiguration      : " << (unsigned int)config_descriptor->iConfiguration;
				LOG(DEBUG3) << "bmAttributes        : " << (unsigned int)config_descriptor->bmAttributes;
				LOG(DEBUG3) << "MaxPower            : " << (unsigned int)config_descriptor->MaxPower;
				LOG(DEBUG3) << "--";
				LOG(DEBUG3) << "--";
				LOG(DEBUG3) << "--";

				libusb_free_config_descriptor( config_descriptor );
			}
		}

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

void KeyboardDriver::closeDevice(const KeyboardDevice &device, const uint8_t bus, const uint8_t num) {
	LOG(DEBUG3) << "trying to close " << device.name << "("
				<< device.vendor_id << ":" << device.product_id << "), device "
				<< (unsigned int)num << " on bus " << (unsigned int)bus;

	bool found = false;
	for(auto it = this->initialized_devices_.begin(); it != this->initialized_devices_.end();) {
		if( ((*it).bus == bus) and ((*it).num == num) ) {
			delete (*it).virtual_keyboard;
			(*it).virtual_keyboard = nullptr;
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

