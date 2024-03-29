/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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

#include <cstdint>

#include <type_traits>

#ifndef SRC_INCLUDE_ENUMS_HPP_
#define SRC_INCLUDE_ENUMS_HPP_

namespace GLogiK
{

const uint64_t one = 1;

enum class Keys : uint64_t
{
					GK_KEY_G1	= one << 0,
					GK_KEY_G2	= one << 1,
					GK_KEY_G3	= one << 2,
					GK_KEY_G4	= one << 3,
					GK_KEY_G5	= one << 4,
					GK_KEY_G6	= one << 5,
					GK_KEY_G7	= one << 6,
					GK_KEY_G8	= one << 7,

					GK_KEY_G9	= one << 8,
					GK_KEY_G10	= one << 9,
					GK_KEY_G11	= one << 10,
					GK_KEY_G12	= one << 11,
					GK_KEY_G13	= one << 12,
					GK_KEY_G14	= one << 13,
					GK_KEY_G15	= one << 14,
					GK_KEY_G16	= one << 15,

					GK_KEY_G17	= one << 16,
					GK_KEY_G18	= one << 17,

				GK_KEY_LIGHT	= one << 18,
					GK_KEY_M1	= one << 19,
					GK_KEY_M2	= one << 20,
					GK_KEY_M3	= one << 21,
					GK_KEY_MR	= one << 22,

					GK_KEY_L1	= one << 23,
					GK_KEY_L2	= one << 24,
					GK_KEY_L3	= one << 25,
					GK_KEY_L4	= one << 26,
					GK_KEY_L5	= one << 27,

			GK_KEY_MUTE_HEADSET	= one << 28,
			GK_KEY_MUTE_MICRO	= one << 29,

			GK_KEY_AUDIO_NEXT	= one << 30,
			GK_KEY_AUDIO_PREV	= one << 31,
			GK_KEY_AUDIO_STOP	= one << 32,
			GK_KEY_AUDIO_PLAY	= one << 33,
			GK_KEY_AUDIO_MUTE	= one << 34,
	GK_KEY_AUDIO_RAISE_VOLUME	= one << 35,
	GK_KEY_AUDIO_LOWER_VOLUME	= one << 36,

};

enum class SpecialKeys : uint8_t
{
	GK_KEY_BACKLIGHT_OFF	= 1 << 2,
	GK_KEY_HEADSET_OFF		= 1 << 3,
	GK_KEY_MICRO_OFF		= 1 << 4,
	GK_ONBOARD_AUDIO_ON		= 1 << 5,	/* this bit is on with onboard audio */
};

enum class Leds : uint8_t
{
	GK_LED_M1 = 1 << 0,
	GK_LED_M2 = 1 << 1,
	GK_LED_M3 = 1 << 2,
	GK_LED_MR = 1 << 3,
};

enum class Caps : uint64_t
{
	GK_BACKLIGHT_COLOR	= one << 0,
	GK_MACROS_KEYS		= one << 1,
	GK_MEDIA_KEYS		= one << 2,
	GK_LCD_SCREEN		= one << 3,
};

inline Caps operator | (Caps lhs, Caps rhs)
{
	using T = std::underlying_type_t<Caps>;
	return (Caps)(static_cast<T>(lhs) | static_cast<T>(rhs));
}

enum class LCDScreenPlugin : uint64_t
{
	GK_LCD_SPLASHSCREEN		= one << 0,
	GK_LCD_SYSTEM_MONITOR	= one << 1,
	GK_LCD_ENDSCREEN		= one << 2,
	GK_LCD_CORETEMP			= one << 3,
	GK_LCD_R4				= one << 60,	/* reserved 4 */
	GK_LCD_R3				= one << 61,	/* reserved 3 */
	GK_LCD_R2				= one << 62,	/* reserved 2 */
	GK_LCD_R1				= one << 63,	/* reserved 1 */
};

enum class LCDPluginsMask : uint8_t
{
	GK_LCD_PLUGINS_MASK_1 = 1,
};

} // namespace GLogiK

#endif
