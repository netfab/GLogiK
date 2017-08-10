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

#include <vector>

#include "keyboard_driver.h"
#include "globals.h"

#include "include/enums.h"

namespace GLogiKd
{

#define VENDOR_LOGITECH "046d"
#define INTERRUPT_READ_MAX_LENGTH 8

struct R_Key
{
	unsigned short index;
	unsigned char mask;
	Keys key;
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
		KeyStatus processKeyEvent(int64_t * pressed_keys, unsigned int actual_length);
		void sendDeviceInitialization(const InitializedDevice & current_device);
		void setLeds(const InitializedDevice & current_device, uint8_t leds);
		void processKeyEvent5Bytes(int64_t * pressed_keys);
		void processKeyEvent2Bytes(int64_t * pressed_keys);

		std::vector< R_Key > five_bytes_keys_map_ = {
			{1, 0x01, Keys::GK_KEY_G1},
			{1, 0x02, Keys::GK_KEY_G2},
			{1, 0x04, Keys::GK_KEY_G3},
			{1, 0x08, Keys::GK_KEY_G4},
			{1, 0x10, Keys::GK_KEY_G5},
			{1, 0x20, Keys::GK_KEY_G6},
			{1, 0x40, Keys::GK_KEY_G7},
			{1, 0x80, Keys::GK_KEY_G8},

			{2, 0x01, Keys::GK_KEY_G9},
			{2, 0x02, Keys::GK_KEY_G10},
			{2, 0x04, Keys::GK_KEY_G11},
			{2, 0x08, Keys::GK_KEY_G12},
			{2, 0x10, Keys::GK_KEY_G13},
			{2, 0x20, Keys::GK_KEY_G14},
			{2, 0x40, Keys::GK_KEY_G15},
			{2, 0x80, Keys::GK_KEY_G16},

			{3, 0x01, Keys::GK_KEY_G17},
			{3, 0x02, Keys::GK_KEY_G18},
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

} // namespace GLogiKd

#endif
