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

#include <array>

#include <linux/input-event-codes.h>

#include <cstdint>

namespace USBKeyboard::keyboard::detail
{

enum class KeyStatus : std::uint8_t
{
	S_KEY_PROCESSED = 0,
	S_KEY_TIMEDOUT,
	S_KEY_SKIPPED,
	S_KEY_UNKNOWN
};

enum class GKModifierKeys : std::uint8_t
{
	GK_KEY_LEFT_CTRL   = 1 << 0,
	GK_KEY_LEFT_SHIFT  = 1 << 1,
	GK_KEY_LEFT_ALT    = 1 << 2,
	GK_KEY_LEFT_META   = 1 << 3,
	GK_KEY_RIGHT_CTRL  = 1 << 4,
	GK_KEY_RIGHT_SHIFT = 1 << 5,
	GK_KEY_RIGHT_ALT   = 1 << 6,
	GK_KEY_RIGHT_META  = 1 << 7
};

struct ModifierKey {
	const std::uint8_t code;			/* event code */
	const GKModifierKeys key;	/* modifier key */
};

/* KEY_FOO from linux/input-event-codes.h */
constexpr std::array<ModifierKey, 8> modifierKeys =
{
	{
		{ KEY_LEFTCTRL  , GKModifierKeys::GK_KEY_LEFT_CTRL   },
		{ KEY_LEFTSHIFT , GKModifierKeys::GK_KEY_LEFT_SHIFT  },
		{ KEY_LEFTALT   , GKModifierKeys::GK_KEY_LEFT_ALT    },
		{ KEY_LEFTMETA  , GKModifierKeys::GK_KEY_LEFT_META   },
		{ KEY_RIGHTCTRL , GKModifierKeys::GK_KEY_RIGHT_CTRL  },
		{ KEY_RIGHTSHIFT, GKModifierKeys::GK_KEY_RIGHT_SHIFT },
		{ KEY_RIGHTALT  , GKModifierKeys::GK_KEY_RIGHT_ALT   },
		{ KEY_RIGHTMETA , GKModifierKeys::GK_KEY_RIGHT_META  }
	}
};

#define unk	KEY_UNKNOWN

/* USB HID Usage Tables as defined in USB specification,
 *        Chapter 10 "Keyboard/Keypad Page (0x07)"
 *
 * https://www.usb.org/document-library/hid-usage-tables-112
 *      -> https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf
 *
 * See linux/drivers/hid/hid-input.c
 * and linux/input-event-codes.h
 */
constexpr unsigned char hidKeyboard[256] = {
	  0,  0,  0,  0, 30, 48, 46, 32, 18, 33, 34, 35, 23, 36, 37, 38,
	 50, 49, 24, 25, 16, 19, 31, 20, 22, 47, 17, 45, 21, 44,  2,  3,
	  4,  5,  6,  7,  8,  9, 10, 11, 28,  1, 14, 15, 57, 12, 13, 26,
	 27, 43, 43, 39, 40, 41, 51, 52, 53, 58, 59, 60, 61, 62, 63, 64,
	 65, 66, 67, 68, 87, 88, 99, 70,119,110,102,104,111,107,109,106,
	105,108,103, 69, 98, 55, 74, 78, 96, 79, 80, 81, 75, 76, 77, 71,
	 72, 73, 82, 83, 86,127,116,117,183,184,185,186,187,188,189,190,
	191,192,193,194,134,138,130,132,128,129,131,137,133,135,136,113,
	115,114,unk,unk,unk,121,unk, 89, 93,124, 92, 94, 95,unk,unk,unk,
	122,123, 90, 91, 85,unk,unk,unk,unk,unk,unk,unk,111,unk,unk,unk,
	unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,
	unk,unk,unk,unk,unk,unk,179,180,unk,unk,unk,unk,unk,unk,unk,unk,
	unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,
	unk,unk,unk,unk,unk,unk,unk,unk,111,unk,unk,unk,unk,unk,unk,unk,
	 29, 42, 56,125, 97, 54,100,126,164,166,165,163,161,115,114,113,
	150,158,159,128,136,177,178,176,142,152,173,140,unk,unk,unk,unk
};

} // namespace detail
