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

#ifndef __GLOGIKD_LOGITECH_G510_DRIVER_H__
#define __GLOGIKD_LOGITECH_G510_DRIVER_H__

#include <cstdint>

#include "keyboard_driver.h"
#include "globals.h"

namespace GLogiKd
{

#define VENDOR_LOGITECH "046d"
#define INTERRUPT_READ_MAX_LENGTH 8

class LogitechG510 : public KeyboardDriver
{
	public:
		LogitechG510();
		~LogitechG510();

		const char* getDriverName() const { return "Logitech G510/G510s driver"; };
		uint16_t getDriverID() const { return GLOGIKD_DRIVER_ID_G510; };

	protected:
	private:
		void processKeyEvent(unsigned int * pressed_keys, unsigned char * buffer, unsigned int actual_length);
};

} // namespace GLogiKd

#endif
