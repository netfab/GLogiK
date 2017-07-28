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

#include <cstdint>

#include <string>
#include <vector>
#include <sstream>

#include <libusb-1.0/libusb.h>

#include "virtual_keyboard.h"

namespace GLogiKd
{

struct KeyboardDevice {
	const char* name;
	const char* vendor_id;
	const char* product_id;
};

struct InitializedDevice {
	KeyboardDevice device;
	uint8_t bus;
	uint8_t num;
	libusb_device *usb_device;
	libusb_device_handle *usb_handle;
	VirtualKeyboard *virtual_keyboard;
};

class KeyboardDriver
{
	public:
		KeyboardDriver();
		virtual ~KeyboardDriver();

		virtual const char* getDriverName() const = 0;
		virtual uint16_t getDriverID() const = 0;

		std::vector<KeyboardDevice> getSupportedDevices(void) const;

		virtual void initializeDevice(const KeyboardDevice &device, const uint8_t bus, const uint8_t num);
		virtual void closeDevice(const KeyboardDevice &device, const uint8_t bus, const uint8_t num);

	protected:
		std::ostringstream buffer_;
		std::vector<KeyboardDevice> supported_devices_;

		void initializeLibusb(InitializedDevice & current_device);
		VirtualKeyboard* initializeVirtualKeyboard( const char* device_name );

	private:
		static bool libusb_status_;			/* is libusb initialized ? */
		static uint8_t drivers_cnt_;		/* initialized drivers counter */
		libusb_context *context_;
		libusb_device **list_;

		std::vector<InitializedDevice> initialized_devices_;

		void closeLibusb(void);
		int handleLibusbError(int error_code);
};

} // namespace GLogiKd

#endif
