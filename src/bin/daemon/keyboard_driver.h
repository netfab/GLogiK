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

#ifndef __GLOGIKD_KEYBOARD_DRIVER_H__
#define __GLOGIKD_KEYBOARD_DRIVER_H__

#include <string>
#include <vector>
#include <sstream>

#include <libusb-1.0/libusb.h>

#include "virtual_keyboard.h"

namespace GLogiKd
{

struct KeyboardDevice {
	std::string name;
	std::string vendor_id;
	std::string product_id;
};

struct InitializedDevice {
	KeyboardDevice device;
	unsigned int bus;
	unsigned int num;
	VirtualKeyboard *virtual_keyboard;
};

class KeyboardDriver
{
	public:
		KeyboardDriver();
		virtual ~KeyboardDriver();

		virtual const char* getDriverName() const = 0;
		virtual unsigned int getDriverID() const = 0;

		std::vector<KeyboardDevice> getSupportedDevices(void) const;

		virtual void initializeDevice(const KeyboardDevice &device, const unsigned int bus, const unsigned int num);
		virtual void closeDevice(const KeyboardDevice &device, const unsigned int bus, const unsigned int num);

	protected:
		std::ostringstream buffer_;
		std::vector<KeyboardDevice> supported_devices_;

		void initializeLibusb(const unsigned int bus, const unsigned int num);
		VirtualKeyboard* initializeVirtualKeyboard( const char* device_name );

	private:
		static bool libusb_status_;			/* is libusb initialized ? */
		static unsigned int drivers_cnt_;	/* initialized drivers counter */
		libusb_context *context_;
		libusb_device **list_;

		std::vector<InitializedDevice> initialized_devices_;

		void closeLibusb(void);
		int handleLibusbError(int error_code, const char* except_msg);
};

} // namespace GLogiKd

#endif
