/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_BIN_SERVICE_VIRTUAL_KEYBOARD_HPP_
#define SRC_BIN_SERVICE_VIRTUAL_KEYBOARD_HPP_

#include <string>

#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

#include "include/base.hpp"

namespace GLogiK
{

class VirtualKeyboard
{
	public:
		VirtualKeyboard(void);
		~VirtualKeyboard(void);

		void sendKeyEvent(const KeyEvent & key);

	protected:
	private:
		libevdev *_pDevice;
		libevdev_uinput *_pUInputDevice;

		void enableEventType(unsigned int type);
		void enableEventCode(unsigned int type, unsigned int code);
};

} // namespace GLogiK

#endif
