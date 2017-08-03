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
#include <thread>
#include <chrono>

#include <cstring>

#include "virtual_keyboard.h"

#include "exception.h"
#include "include/log.h"

namespace GLogiKd
{

VirtualKeyboard::VirtualKeyboard(const char* device_name) : buffer_("", std::ios_base::app) {
	LOG(DEBUG3) << "initializing " << device_name;

	this->dev = libevdev_new();
	libevdev_set_name(this->dev, device_name);

	this->enable_event_type(EV_KEY);

	{	/* linux/input-event-codes.h */
		unsigned int i = 0;

		for (i = KEY_ESC; i <= KEY_KPDOT; i++)
			this->enable_event_code(EV_KEY, i);
		for (i = KEY_ZENKAKUHANKAKU; i <= KEY_F24; i++)
			this->enable_event_code(EV_KEY, i);
		for (i = KEY_PLAYCD; i <= KEY_MICMUTE; i++)
			this->enable_event_code(EV_KEY, i);
	}

	int err = libevdev_uinput_create_from_device(this->dev,
				LIBEVDEV_UINPUT_OPEN_MANAGED, &this->uidev);

	if (err < 0) {
		this->buffer_.str("virtual device creation failure : ");
		this->buffer_ << strerror( -(err) );
		this->free_device_and_throw();
	}
}

VirtualKeyboard::~VirtualKeyboard() {
	LOG(DEBUG3) << "destroying " << libevdev_get_name(this->dev);

	libevdev_uinput_destroy(this->uidev);
	libevdev_free(this->dev);
}

void VirtualKeyboard::free_device_and_throw(void) {
	libevdev_free(this->dev);
	throw GLogiKExcept(this->buffer_.str());
}

void VirtualKeyboard::enable_event_type(unsigned int type) {
	if ( libevdev_enable_event_type(this->dev, type) != 0 ) {
		this->buffer_.str("enable event type failure : ");
		this->buffer_ << type;
		this->free_device_and_throw();
	}
}

void VirtualKeyboard::enable_event_code(unsigned int type, unsigned int code) {
	if ( libevdev_enable_event_code(this->dev, type, code, nullptr) != 0 ) {
		this->buffer_.str("enable event code failure : type ");
		this->buffer_ << type << " code " << code;
		this->free_device_and_throw();
	}
}

void VirtualKeyboard::foo(void) {
	LOG(DEBUG3) << "yeah foo !!!";
	libevdev_uinput_write_event(uidev, EV_KEY, KEY_A, 1);
	libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
	libevdev_uinput_write_event(uidev, EV_KEY, KEY_A, 0);
	libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);

	std::this_thread::sleep_for (std::chrono::milliseconds(100));
}

} // namespace GLogiKd

