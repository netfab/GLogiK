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

#ifndef __GLOGIKD_VIRTUAL_KEYBOARD_H__
#define __GLOGIKD_VIRTUAL_KEYBOARD_H__

#include <sstream>

#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

namespace GLogiKd
{

class VirtualKeyboard
{
	public:
		VirtualKeyboard(const char* device_name);
		~VirtualKeyboard();
		void foo(void);

	protected:
	private:
		struct libevdev *dev;
		struct libevdev_uinput *uidev;

		std::ostringstream buffer_;

		void free_device_and_throw(void);
		void enable_event_type(unsigned int type);
		void enable_event_code(unsigned int type, unsigned int code);
};

} // namespace GLogiKd

#endif