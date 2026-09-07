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

#include <cstdint>

#include <type_traits>

#ifndef SRC_INCLUDE_ENUMS_HPP_
#define SRC_INCLUDE_ENUMS_HPP_

namespace GLogiK
{

enum class SpecialKeys : std::uint8_t
{
	GK_KEY_BACKLIGHT_OFF	= 1 << 2,
	GK_KEY_HEADSET_OFF		= 1 << 3,
	GK_KEY_MICRO_OFF		= 1 << 4,
	GK_ONBOARD_AUDIO_ON		= 1 << 5,	/* this bit is on with onboard audio */
};

enum class Leds : std::uint8_t
{
	GK_LED_M1 = 1 << 0,
	GK_LED_M2 = 1 << 1,
	GK_LED_M3 = 1 << 2,
	GK_LED_MR = 1 << 3,
};

enum class Caps : std::uint64_t
{
	GK_BACKLIGHT_COLOR	= std::uint64_t{1} << 0,
	GK_MACROS_KEYS		= std::uint64_t{1} << 1,
	GK_MEDIA_KEYS		= std::uint64_t{1} << 2,
	GK_LCD_SCREEN		= std::uint64_t{1} << 3,
};

inline Caps operator | (Caps lhs, Caps rhs)
{
	using T = std::underlying_type_t<Caps>;
	return (Caps)(static_cast<T>(lhs) | static_cast<T>(rhs));
}

enum class LCDScreenPlugin : std::uint64_t
{
	GK_LCD_SPLASHSCREEN		= std::uint64_t{1} << 0,
	GK_LCD_SYSTEM_MONITOR	= std::uint64_t{1} << 1,
	GK_LCD_ENDSCREEN		= std::uint64_t{1} << 2,
	GK_LCD_CORETEMP			= std::uint64_t{1} << 3,
	GK_LCD_R4				= std::uint64_t{1} << 60,	/* reserved 4 */
	GK_LCD_R3				= std::uint64_t{1} << 61,	/* reserved 3 */
	GK_LCD_R2				= std::uint64_t{1} << 62,	/* reserved 2 */
	GK_LCD_R1				= std::uint64_t{1} << 63,	/* reserved 1 */
};

enum class LCDPluginsMask : std::uint8_t
{
	GK_LCD_PLUGINS_MASK_1 = 1,
};

} // namespace GLogiK

#endif
