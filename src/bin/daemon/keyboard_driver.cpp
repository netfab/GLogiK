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

#include <config.h>

#include "exception.h"
#include "include/log.h"

#include "keyboard_driver.h"


namespace GLogiKd
{

bool KeyboardDriver::libusb_status_ = false;
unsigned int KeyboardDriver::drivers_cnt_ = 0;

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
	this->handleLibusbError(ret_value, "libusb initialization failure");

	KeyboardDriver::libusb_status_ = true;

	libusb_device **list;
	int num_devices = libusb_get_device_list(this->context_, &(list));
	if( num_devices <= 0 )
		this->handleLibusbError(num_devices, "error getting USB devices list (or zero!)");

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
		throw GLogiKExcept(this->buffer_.str());
	}

	LOG(DEBUG3) << "libusb found device " << current_device.num << " on bus " << current_device.bus;

	// TODO open device

	libusb_free_device_list(list, 1);
}

void KeyboardDriver::closeLibusb(void) {
	LOG(DEBUG4) << "closing libusb";
	libusb_exit(this->context_);
	KeyboardDriver::libusb_status_ = false;
}

int KeyboardDriver::handleLibusbError(int error_code, const char* except_msg) {
	switch(error_code) {
		case LIBUSB_SUCCESS:
			break;
		default:
			LOG(ERROR) << "handleLibusbError: (" <<  libusb_error_name(error_code) << ") "
						<< libusb_strerror((libusb_error)error_code);
			throw GLogiKExcept(except_msg);
			break;
	}

	return error_code;
}

void KeyboardDriver::initializeDevice(const KeyboardDevice &device, const unsigned int bus, const unsigned int num) {
	LOG(DEBUG3) << "trying to initialize " << device.name << "("
				<< device.vendor_id << ":" << device.product_id << "), device "
				<< num << " on bus " << bus;

	InitializedDevice current_device = { device, bus, num, nullptr, nullptr };

	this->initializeLibusb(current_device);

	this->buffer_.str("Virtual ");
	this->buffer_ << device.name << " b" << bus << "d" << num;

	try {
		current_device.virtual_keyboard = this->initializeVirtualKeyboard(this->buffer_.str().c_str());
	}
	catch (const std::bad_alloc& e) {
		throw GLogiKExcept("virtual keyboard allocation failure");
	}

	this->initialized_devices_.push_back( current_device );
}

void KeyboardDriver::closeDevice(const KeyboardDevice &device, const unsigned int bus, const unsigned int num) {
	LOG(DEBUG3) << "trying to close " << device.name << "("
				<< device.vendor_id << ":" << device.product_id << "), device "
				<< num << " on bus " << bus;

	bool found = false;
	for(auto it = this->initialized_devices_.begin(); it != this->initialized_devices_.end();) {
		if( ((*it).bus == bus) and ((*it).num == num) ) {
			delete (*it).virtual_keyboard;
			(*it).virtual_keyboard = nullptr;
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

