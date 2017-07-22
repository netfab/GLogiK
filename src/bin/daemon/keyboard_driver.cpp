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

void KeyboardDriver::initializeLibusb(void) {
	LOG(DEBUG4) << "initializing libusb";
	int ret_value = libusb_init( &(this->context_) );
	this->handleLibusbError(ret_value, "libusb initialization failure");

	KeyboardDriver::libusb_status_ = true;

	libusb_device **list;
	int num_devices = libusb_get_device_list(this->context_, &(list));
	if( num_devices <= 0 )
		this->handleLibusbError(num_devices, "error getting USB devices list (or zero!)");

	for (int idx = 0; idx < num_devices; ++idx) {
		libusb_device *device = list[idx];
		LOG(DEBUG4) << "bus num: " << (int)libusb_get_bus_number(device);
		LOG(DEBUG4) << "device address: " << (int)libusb_get_device_address(device);
	}


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

} // namespace GLogiKd

