/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
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

#include <vector>

#include "keyboard_driver.h"
#include "globals.h"

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

	R_Key(unsigned short i, unsigned char m, Keys k, bool b=false, const char* n=nullptr)
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

		const char* getDriverName() const { return "Logitech G510/G510s driver"; };
		uint16_t getDriverID() const { return GLOGIKD_DRIVER_ID_G510; };

	protected:
	private:
		KeyStatus processKeyEvent(InitializedDevice & device);
		void sendDeviceInitialization(const InitializedDevice & device);
		void setMxKeysLeds(const InitializedDevice & device);
		void setKeyboardColor(const InitializedDevice & device);
		void processKeyEvent8Bytes(InitializedDevice & device);
		void processKeyEvent5Bytes(InitializedDevice & device);
		void processKeyEvent2Bytes(InitializedDevice & device);
		void initializeMacroKeys(const InitializedDevice & device);
		const bool checkMacroKey(InitializedDevice & device);

		std::vector< R_Key > five_bytes_keys_map_ = {
			{1, 0x01, Keys::GK_KEY_G1,  true, "G1"},
			{1, 0x02, Keys::GK_KEY_G2,  true, "G2"},
			{1, 0x04, Keys::GK_KEY_G3,  true, "G3"},
			{1, 0x08, Keys::GK_KEY_G4,  true, "G4"},
			{1, 0x10, Keys::GK_KEY_G5,  true, "G5"},
			{1, 0x20, Keys::GK_KEY_G6,  true, "G6"},
			{1, 0x40, Keys::GK_KEY_G7,  true, "G7"},
			{1, 0x80, Keys::GK_KEY_G8,  true, "G8"},

			{2, 0x01, Keys::GK_KEY_G9,  true, "G9"},
			{2, 0x02, Keys::GK_KEY_G10, true, "G10"},
			{2, 0x04, Keys::GK_KEY_G11, true, "G11"},
			{2, 0x08, Keys::GK_KEY_G12, true, "G12"},
			{2, 0x10, Keys::GK_KEY_G13, true, "G13"},
			{2, 0x20, Keys::GK_KEY_G14, true, "G14"},
			{2, 0x40, Keys::GK_KEY_G15, true, "G15"},
			{2, 0x80, Keys::GK_KEY_G16, true, "G16"},

			{3, 0x01, Keys::GK_KEY_G17, true, "G17"},
			{3, 0x02, Keys::GK_KEY_G18, true, "G18"},
//			{3, 0x04, Keys::GK_KEY_},
			{3, 0x08, Keys::GK_KEY_LIGHT},
			{3, 0x10, Keys::GK_KEY_M1},
			{3, 0x20, Keys::GK_KEY_M2},
			{3, 0x40, Keys::GK_KEY_M3},
			{3, 0x80, Keys::GK_KEY_MR},

			{4, 0x01, Keys::GK_KEY_L1},
			{4, 0x02, Keys::GK_KEY_L2},
			{4, 0x04, Keys::GK_KEY_L3},
			{4, 0x08, Keys::GK_KEY_L4},
			{4, 0x10, Keys::GK_KEY_L5},
			{4, 0x20, Keys::GK_KEY_MUTE_HEADSET},
			{4, 0x40, Keys::GK_KEY_MUTE_MICRO},
//			{4, 0x80, Keys::GK_KEY_},
		};

		std::vector< R_Key > two_bytes_keys_map_ = {
			{1, 0x01, Keys::GK_KEY_AUDIO_NEXT},				/* XF86AudioNext */
			{1, 0x02, Keys::GK_KEY_AUDIO_PREV},				/* XF86AudioPrev */
			{1, 0x04, Keys::GK_KEY_AUDIO_STOP},				/* XF86AudioStop */
			{1, 0x08, Keys::GK_KEY_AUDIO_PLAY},				/* XF86AudioPlay */
			{1, 0x10, Keys::GK_KEY_AUDIO_MUTE},				/* XF86AudioMute */
			{1, 0x20, Keys::GK_KEY_AUDIO_RAISE_VOLUME},		/* XF86AudioRaiseVolume */
			{1, 0x40, Keys::GK_KEY_AUDIO_LOWER_VOLUME},		/* XF86AudioLowerVolume */
//			{1, 0x80, Keys::GK_KEY_},
		};

		std::vector< M_Key_Led_Mask > leds_mask_ = {
			{Leds::GK_LED_M1, 0x80},
			{Leds::GK_LED_M2, 0x40},
			{Leds::GK_LED_M3, 0x20},
			{Leds::GK_LED_MR, 0x10},
		};
};

} // namespace GLogiK

#endif
