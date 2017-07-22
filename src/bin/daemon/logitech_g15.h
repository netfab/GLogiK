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

#ifndef __GLOGIKD_LOGITECH_G15_DRIVER_H__
#define __GLOGIKD_LOGITECH_G15_DRIVER_H__

#include <sstream>

#include "keyboard_driver.h"
#include "globals.h"

namespace GLogiKd
{

#define VENDOR_LOGITECH "046d"

class LogitechG15 : public KeyboardDriver
{
	public:
		LogitechG15();
		~LogitechG15();

		const char* getDriverName() const { return "Logitech G15"; };
		unsigned int getDriverID() const { return GLOGIKD_DRIVER_ID_G15; };

		void initializeDevice(const char* vendor_id, const char* product_id);
		void closeDevice(void);

	protected:
	private:
		bool initialized;
		std::ostringstream buffer_;
};

} // namespace GLogiKd

#endif
