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

#ifndef __GLOGIKD_LOGITECH_G510_DRIVER_H__
#define __GLOGIKD_LOGITECH_G510_DRIVER_H__

#define GLOGIKD_DRIVER_ID_G510 ( 1 << 0 )

#include <cstdint>

#include <vector>

#include "keyboardDriver.h"

#include "include/enums.h"

namespace GLogiK
{

#define VENDOR_LOGITECH "046d"
#define INTERRUPT_READ_MAX_LENGTH 8
#define TRANSFER_LENGTH_FOR_LEDS_UPDATE 5

struct R_Key
{
	unsigned short index;
	unsigned char mask;
	Keys key;
	bool macro_key;
	const char* name;

	R_Key(unsigned short i, unsigned char m, Keys k, const char* n=nullptr, bool b=false)
		: index(i), mask(m), key(k), macro_key(b), name(n) {}
};

struct M_Key_Led_Mask
{
	Leds led;
	unsigned char mask;
};

class LogitechG510 : public KeyboardDriver
{
	public:
		LogitechG510();
		~LogitechG510();

		const char* getDriverName() const;
		const uint16_t getDriverID() const;
		const std::vector<KeyboardDevice> & getSupportedDevices(void) const;
		const std::vector<std::string> & getMacroKeysNames(void) const;

	protected:
	private:
		static const std::vector< R_Key > five_bytes_keys_map_;
		static const std::vector< R_Key > two_bytes_keys_map_;
		static const std::vector< M_Key_Led_Mask > leds_mask_;
		static const std::vector<KeyboardDevice> supported_devices_;

		KeyStatus processKeyEvent(InitializedDevice & device);
		void sendUSBDeviceInitialization(const InitializedDevice & device);
		void setMxKeysLeds(const InitializedDevice & device);
		void setKeyboardColor(const InitializedDevice & device);
		void processKeyEvent8Bytes(InitializedDevice & device);
		void processKeyEvent5Bytes(InitializedDevice & device);
		void processKeyEvent2Bytes(InitializedDevice & device);
		const bool checkMacroKey(InitializedDevice & device);
};

} // namespace GLogiK

#endif
