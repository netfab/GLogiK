/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
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

#include <syslog.h>
#include "lib/utils/utils.h"
#include "include/log.h"

namespace GLogiK
{

VirtualKeyboard::VirtualKeyboard(const char* device_name) : buffer_("", std::ios_base::app) {
	LOG(DEBUG3) << "initializing " << device_name;

	this->dev = libevdev_new();
	libevdev_set_name(this->dev, device_name);

	this->enableEventType(EV_KEY);

	{	/* linux/input-event-codes.h */
		unsigned int i = 0;

		for (i = KEY_ESC; i <= KEY_KPDOT; i++)
			this->enableEventCode(EV_KEY, i);
		for (i = KEY_ZENKAKUHANKAKU; i <= KEY_F24; i++)
			this->enableEventCode(EV_KEY, i);
		for (i = KEY_PLAYCD; i <= KEY_MICMUTE; i++)
			this->enableEventCode(EV_KEY, i);
	}

	int err = libevdev_uinput_create_from_device(this->dev,
				LIBEVDEV_UINPUT_OPEN_MANAGED, &this->uidev);

	if (err < 0) {
		this->buffer_.str("virtual device creation failure : ");
		this->buffer_ << strerror( -(err) );
		this->freeDeviceAndThrow();
	}
}

VirtualKeyboard::~VirtualKeyboard() {
	LOG(DEBUG3) << "destroying " << libevdev_get_name(this->dev);

	libevdev_uinput_destroy(this->uidev);
	libevdev_free(this->dev);
}

void VirtualKeyboard::freeDeviceAndThrow(void) {
	libevdev_free(this->dev);
	throw GLogiKExcept(this->buffer_.str());
}

void VirtualKeyboard::enableEventType(unsigned int type) {
	if ( libevdev_enable_event_type(this->dev, type) != 0 ) {
		this->buffer_.str("enable event type failure : ");
		this->buffer_ << type;
		this->freeDeviceAndThrow();
	}
}

void VirtualKeyboard::enableEventCode(unsigned int type, unsigned int code) {
	if ( libevdev_enable_event_code(this->dev, type, code, nullptr) != 0 ) {
		this->buffer_.str("enable event code failure : type ");
		this->buffer_ << type << " code " << code;
		this->freeDeviceAndThrow();
	}
}

void VirtualKeyboard::handleLibevdevError(int ret) {
	switch(ret) {
		default:
			this->buffer_.str("warning : libevdev error (");
			this->buffer_ << -ret << ") : " << strerror(-ret);
			LOG(WARNING) << this->buffer_.str();
			syslog(LOG_WARNING, this->buffer_.str().c_str());
			break;
	}
}

void VirtualKeyboard::sendKeyEvent(const KeyEvent & key) {
	if( key.interval > 20 ) { // FIXME
#if DEBUGGING_ON
		LOG(DEBUG) << "sleeping for : " << key.interval << "ms";
#endif
		std::this_thread::sleep_for(std::chrono::milliseconds(key.interval));
	}
	int ret = libevdev_uinput_write_event(uidev, EV_KEY, key.event_code, static_cast<int>(key.event));
	if(ret == 0) {
		ret = libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
		if(ret != 0 ) {
			this->handleLibevdevError(ret);
		}
	}
	else {
		this->handleLibevdevError(ret);
	}
}

} // namespace GLogiK

