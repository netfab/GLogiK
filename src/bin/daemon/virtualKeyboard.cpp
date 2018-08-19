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

#include <iostream>
#include <thread>
#include <chrono>

#include <cstring>

#include "lib/utils/utils.h"

#include "virtualKeyboard.h"

namespace GLogiK
{

using namespace NSGKUtils;

VirtualKeyboard::VirtualKeyboard(const char* deviceName) : buffer_("", std::ios_base::app) {
#if DEBUGGING_ON
	LOG(DEBUG3) << "initializing " << deviceName;
#endif

	_pDevice = libevdev_new();
	libevdev_set_name(_pDevice, deviceName);

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

	int err = libevdev_uinput_create_from_device(
		_pDevice, LIBEVDEV_UINPUT_OPEN_MANAGED, &_pUInputDevice);

	if (err < 0) {
		this->buffer_.str("virtual device creation failure : ");
		this->buffer_ << strerror( -(err) );
		this->freeDeviceAndThrow();
	}
}

VirtualKeyboard::~VirtualKeyboard() {
#if DEBUGGING_ON
	LOG(DEBUG3) << "destroying " << libevdev_get_name(_pDevice);
#endif

	libevdev_uinput_destroy(_pUInputDevice);
	libevdev_free(_pDevice);
}

void VirtualKeyboard::freeDeviceAndThrow(void) {
	libevdev_free(_pDevice);
	throw GLogiKExcept(this->buffer_.str());
}

void VirtualKeyboard::enableEventType(unsigned int type) {
	if ( libevdev_enable_event_type(_pDevice, type) != 0 ) {
		this->buffer_.str("enable event type failure : ");
		this->buffer_ << type;
		this->freeDeviceAndThrow();
	}
}

void VirtualKeyboard::enableEventCode(unsigned int type, unsigned int code) {
	if ( libevdev_enable_event_code(_pDevice, type, code, nullptr) != 0 ) {
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
			GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
			break;
	}
}

void VirtualKeyboard::sendKeyEvent(const KeyEvent & key) {
#if DEBUGGING_ON
	LOG(DEBUG1) << "key event : ";
	LOG(DEBUG2)	<< "event_code : " << to_uint(key.event_code);
	LOG(DEBUG2)	<< "event : " << key.event;
	LOG(DEBUG2)	<< "interval : " << key.interval << " ms";
#endif

	if( key.interval > 20 ) { // FIXME
#if DEBUGGING_ON
		LOG(DEBUG3) << "sleeping for : " << key.interval << "ms";
#endif
		std::this_thread::sleep_for(std::chrono::milliseconds(key.interval));
	}
	int ret = libevdev_uinput_write_event(_pUInputDevice, EV_KEY, key.event_code, static_cast<int>(key.event));
	if(ret == 0) {
		ret = libevdev_uinput_write_event(_pUInputDevice, EV_SYN, SYN_REPORT, 0);
		if(ret != 0 ) {
			this->handleLibevdevError(ret);
		}
	}
	else {
		this->handleLibevdevError(ret);
	}
}

} // namespace GLogiK

