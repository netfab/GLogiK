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

#include "include/base.hpp"
#include "include/RKeys.hpp"

namespace GLogiK
{

namespace D_G510
{

class G510Base
{
	public:

	protected:
		G510Base(void) = default;
		virtual ~G510Base(void) = default;

		const char* getDriverName(void) const;
		const std::uint16_t getDriverID(void) const;
		const std::vector<USBDeviceID> & getSupportedDevices(void) const;
		const MKeysIDArray_type getMKeysIDArray(void) const;
		const GKeysIDArray_type getGKeysIDArray(void) const;

		/* return true if any G-Key (G1-G18) was pressed  */
		virtual const bool checkDevicePressedAnyGKey(USBDevice & device);
		/* return true if any media key was pressed */
		virtual const bool checkDevicePressedAnyMediaKey(USBDevice & device);
		/* return true if any LCD key was pressed */
		virtual const bool checkDevicePressedAnyLCDKey(USBDevice & device);
		/* return true if any Mx key was pressed */
		virtual const bool checkDevicePressedAnyMxKey(USBDevice & device);
		/* return true if MR key is enabled */
		virtual const bool isDeviceMRKeyEnabled(USBDevice & device);

		virtual KeyStatus processDeviceKeyEvent(USBDevice & device);

		virtual void sendUSBDeviceInitialization(USBDevice & device);

		virtual void setDeviceBacklightColor(
			USBDevice & device,
			const std::uint8_t r=0xFF,
			const std::uint8_t g=0xFF,
			const std::uint8_t b=0xFF
		);

		virtual void setDeviceMxKeysLeds(USBDevice & device);

		virtual const bool updateDeviceMxKeysLedsMask(
			USBDevice & device,
			bool disableMR=false
		);

	private:
		static const std::map<RKeys, GKeysID>  RKeys2GKeysIDMap;
		static const std::vector<USBDeviceID> knownDevices;

		void processDeviceKeyEvent2Bytes(USBDevice & device);
		void processDeviceKeyEvent5Bytes(USBDevice & device);
		void processDeviceKeyEvent8Bytes(USBDevice & device);

		virtual void sendUSBDeviceFeatureReport(
			USBDevice & device,
			const unsigned char * data,
			std::uint16_t wLength
		) = 0;

		virtual void fillDeviceStandardKeysEvents(USBDevice & device) = 0;

#if DEBUGGING_ON && DEBUG_KEYS
		virtual const std::string getDeviceBytes(const USBDevice & device) const = 0;
#endif

};

} // namespace D_G510

} // namespace GLogiK
