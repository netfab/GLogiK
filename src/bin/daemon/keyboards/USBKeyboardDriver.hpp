/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2026  Fabrice Delliaux <netbox253@gmail.com>
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

#pragma once

#include "USBAPI/USBDevice.hpp"

#include "KeyboardDriver.hpp"

namespace USBKeyboard
{

template <typename API>
class USBKeyboardDriver
	:	public API,
		public keyboard::KeyboardDriver
{
	private:
		using USBDevice = USBAPI::device::USBDevice;

	public:
		virtual ~USBKeyboardDriver();

		virtual const char* getDriverName() const = 0;

	protected:
		USBKeyboardDriver(void);

	private:
		/* API */
		int performUSBDeviceKeysInterruptTransfer(
			USBDevice & device,
			unsigned int timeout
		) override {
			return API::performUSBDeviceKeysInterruptTransfer(device, timeout);
		}

		int performUSBDeviceLCDScreenInterruptTransfer(
			USBDevice & device,
			const unsigned char * buffer,
			int bufferLength,
			unsigned int timeout
		) override {
			return API::performUSBDeviceLCDScreenInterruptTransfer(
				device, buffer, bufferLength, timeout);
		}

		void openUSBDevice(USBDevice & device) override {
			API::openUSBDevice(device);
		}

		void closeUSBDevice(USBDevice & device) override {
			device.destroyLCDPluginsManager();

			API::closeUSBDevice(device);
		}
		/* --- */
};

template <typename API>
USBKeyboardDriver<API>::USBKeyboardDriver(void)
{
}

template <typename API>
USBKeyboardDriver<API>::~USBKeyboardDriver()
{
}

} // namespace USBKeyboard
