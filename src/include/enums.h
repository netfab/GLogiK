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

#include <cstdint>

#ifndef __GLOGIKD_ENUMS_H__
#define __GLOGIKD_ENUMS_H__

namespace GLogiKd
{

enum class Keys : int64_t
{
	GK_KEY_G1    = 1 << 0,
	GK_KEY_G2    = 1 << 1,
	GK_KEY_G3    = 1 << 2,
	GK_KEY_G4    = 1 << 3,
	GK_KEY_G5    = 1 << 4,
	GK_KEY_G6    = 1 << 5,
	GK_KEY_G7    = 1 << 6,
	GK_KEY_G8    = 1 << 7,

	GK_KEY_G9    = 1 << 8,
	GK_KEY_G10   = 1 << 9,
	GK_KEY_G11   = 1 << 10,
	GK_KEY_G12   = 1 << 11,
	GK_KEY_G13   = 1 << 12,
	GK_KEY_G14   = 1 << 13,
	GK_KEY_G15   = 1 << 14,
	GK_KEY_G16   = 1 << 15,

	GK_KEY_G17   = 1 << 16,
	GK_KEY_G18   = 1 << 17,

	GK_KEY_LIGHT = 1 << 18,
	GK_KEY_M1    = 1 << 19,
	GK_KEY_M2    = 1 << 20,
	GK_KEY_M3    = 1 << 21,
	GK_KEY_MR    = 1 << 22,

	GK_KEY_L1    = 1 << 23,
	GK_KEY_L2    = 1 << 24,
	GK_KEY_L3    = 1 << 25,
	GK_KEY_L4    = 1 << 26,
	GK_KEY_L5    = 1 << 27,
};

} // namespace GLogiKd

#endif
