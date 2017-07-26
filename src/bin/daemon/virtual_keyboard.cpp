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

#include <thread>
#include <chrono>

#include "virtual_keyboard.h"

#include "exception.h"
#include "include/log.h"

namespace GLogiKd
{

VirtualKeyboard::VirtualKeyboard(const char* device_name) {
	LOG(DEBUG3) << "virtual keyboard constructor";

	this->dev = libevdev_new();
	libevdev_set_name(dev, device_name);

	libevdev_enable_event_type(this->dev, EV_KEY);
	libevdev_enable_event_code(this->dev, EV_KEY, KEY_A, NULL);

	int err = libevdev_uinput_create_from_device(this->dev,
				LIBEVDEV_UINPUT_OPEN_MANAGED, &this->uidev);

	if (err != 0)
		throw GLogiKExcept("virtual device creation failure");
}

VirtualKeyboard::~VirtualKeyboard() {
	LOG(DEBUG3) << "virtual keyboard destructor";

	this->foo();

	libevdev_uinput_destroy(this->uidev);
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

