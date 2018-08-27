/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2018  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_BIN_DAEMON_LOGITECH_G510_DRIVER_HPP_
#define SRC_BIN_DAEMON_LOGITECH_G510_DRIVER_HPP_

#define GLOGIKD_DRIVER_ID_G510 ( 1 << 0 )

#include <cstdint>

#include <vector>
#include <string>

#include "DeviceID.h"
#include "keyboardDriver.h"

#include "include/enums.h"

namespace GLogiK
{

#define VENDOR_LOGITECH "046d"

/* RKey - Recognized Keys */
struct RKey
{
	const unsigned short index;
	const unsigned char mask;
	const Keys key;
	const bool isMacroKey;
	const char* const name;

	RKey(	const unsigned short i,
			const unsigned char m,
			const Keys k,
			const char* const n=nullptr,
			const bool b=false )
		:	index(i),
			mask(m),
			key(k),
			isMacroKey(b),
			name(n) {}
};

struct MKeyLed
{
	Leds led;
	unsigned char mask;
};

class LogitechG510
	:	public KeyboardDriver
{
	public:
		LogitechG510();
		~LogitechG510();

		const char* getDriverName() const;
		const uint16_t getDriverID() const;
		const std::vector<DeviceID> & getSupportedDevices(void) const;
		const std::vector<std::string> & getMacroKeysNames(void) const;

	protected:
	private:
		static const std::vector<RKey> fiveBytesKeysMap;
		static const std::vector<RKey> twoBytesKeysMap;
		static const std::vector<MKeyLed> ledsMask;
		static const std::vector<DeviceID> knownDevices;

		KeyStatus processKeyEvent(USBDevice & device);
		void sendUSBDeviceInitialization(USBDevice & device);
		void setMxKeysLeds(USBDevice & device);
		void setDeviceBacklightColor(
			USBDevice & device,
			const uint8_t r=0xFF,
			const uint8_t g=0xFF,
			const uint8_t b=0xFF
		);

		void processKeyEvent8Bytes(USBDevice & device);
		void processKeyEvent5Bytes(USBDevice & device);
		void processKeyEvent2Bytes(USBDevice & device);
		const bool checkMacroKey(USBDevice & device);
		const bool checkMediaKey(USBDevice & device);
};

} // namespace GLogiK

#endif
