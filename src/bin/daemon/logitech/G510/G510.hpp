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

#include "config.h"

#include <cstdint>

#include <vector>
#include <string>

#include "src/bin/daemon/keyboardDriver.hpp"
#include "src/bin/daemon/USBDeviceID.hpp"
#include "src/bin/daemon/USBDevice.hpp"

#include "G510Base.hpp"

namespace GLogiK
{

template <typename USBAPI>
class LogitechG510
	:	public USBKeyboardDriver<USBAPI>,
		public D_G510::G510Base
{
	public:
		LogitechG510();
		~LogitechG510();

	protected:
		const char* getDriverName(void) const override
		{
			return G510Base::getDriverName();
		}
		const std::uint16_t getDriverID(void) const override
		{
			return G510Base::getDriverID();
		}
		const std::vector<USBDeviceID> & getSupportedDevices(void) const override
		{
			return G510Base::getSupportedDevices();
		}
		const MKeysIDArray_type getMKeysIDArray(void) const override
		{
			return G510Base::getMKeysIDArray();
		}
		const GKeysIDArray_type getGKeysIDArray(void) const override
		{
			return G510Base::getGKeysIDArray();
		}

	private:
		void sendUSBDeviceFeatureReport(
			USBDevice & device,
			const unsigned char * data,
			std::uint16_t wLength
		) override
		{
			USBKeyboardDriver<USBAPI>::sendUSBDeviceFeatureReport(device, data, wLength);
		}

		void setDeviceBacklightColor(
			USBDevice & device,
			const std::uint8_t r=0xFF,
			const std::uint8_t g=0xFF,
			const std::uint8_t b=0xFF
		) override
		{
			G510Base::setDeviceBacklightColor(device, r, g,  b);
		}

		void setDeviceMxKeysLeds(USBDevice & device) override
		{
			G510Base::setDeviceMxKeysLeds(device);
		}

		const bool updateDeviceMxKeysLedsMask(
			USBDevice & device,
			bool disableMR=false
		) override
		{
			return G510Base::updateDeviceMxKeysLedsMask(device, disableMR);
		}

		void sendUSBDeviceInitialization(USBDevice & device) override
		{
			return G510Base::sendUSBDeviceInitialization(device);
		}

		/* return true if any G-Key (G1-G18) was pressed  */
		const bool checkDevicePressedAnyGKey(USBDevice & device) override
		{
			return G510Base::checkDevicePressedAnyGKey(device);
		}

		/* return true if any media key was pressed */
		const bool checkDevicePressedAnyMediaKey(USBDevice & device) override
		{
			return G510Base::checkDevicePressedAnyMediaKey(device);
		}

		/* return true if any LCD key was pressed */
		const bool checkDevicePressedAnyLCDKey(USBDevice & device) override
		{
			return G510Base::checkDevicePressedAnyLCDKey(device);
		}

		/* return true if any Mx key was pressed */
		virtual const bool checkDevicePressedAnyMxKey(USBDevice & device) override
		{
			return G510Base::checkDevicePressedAnyMxKey(device);
		}

		/* return true if MR key is enabled */
		virtual const bool isDeviceMRKeyEnabled(USBDevice & device) override
		{
			return G510Base::isDeviceMRKeyEnabled(device);
		}

		void fillDeviceStandardKeysEvents(USBDevice & device) override
		{
			USBKeyboardDriver<USBAPI>::fillDeviceStandardKeysEvents(device);
		}

		KeyStatus processDeviceKeyEvent(USBDevice & device) override
		{
			return G510Base::processDeviceKeyEvent(device);
		}

#if DEBUGGING_ON && DEBUG_KEYS
		const std::string getDeviceBytes(const USBDevice & device) const override
		{
			return USBKeyboardDriver<USBAPI>::getDeviceBytes(device);
		}
#endif

};


template <typename USBAPI>
LogitechG510<USBAPI>::LogitechG510()
	:	USBKeyboardDriver<USBAPI>()
{
}

template <typename USBAPI>
LogitechG510<USBAPI>::~LogitechG510()
{
}

} // namespace GLogiK
