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

#include <cstdint>

namespace GLogiK
{

namespace detail
{
	constexpr std::uint64_t bit = 1;
}

enum class RKeys : std::uint64_t
{
					GK_KEY_G1	= detail::bit << 0,
					GK_KEY_G2	= detail::bit << 1,
					GK_KEY_G3	= detail::bit << 2,
					GK_KEY_G4	= detail::bit << 3,
					GK_KEY_G5	= detail::bit << 4,
					GK_KEY_G6	= detail::bit << 5,
					GK_KEY_G7	= detail::bit << 6,
					GK_KEY_G8	= detail::bit << 7,

					GK_KEY_G9	= detail::bit << 8,
					GK_KEY_G10	= detail::bit << 9,
					GK_KEY_G11	= detail::bit << 10,
					GK_KEY_G12	= detail::bit << 11,
					GK_KEY_G13	= detail::bit << 12,
					GK_KEY_G14	= detail::bit << 13,
					GK_KEY_G15	= detail::bit << 14,
					GK_KEY_G16	= detail::bit << 15,

					GK_KEY_G17	= detail::bit << 16,
					GK_KEY_G18	= detail::bit << 17,

				GK_KEY_LIGHT	= detail::bit << 18,
					GK_KEY_M1	= detail::bit << 19,
					GK_KEY_M2	= detail::bit << 20,
					GK_KEY_M3	= detail::bit << 21,
					GK_KEY_MR	= detail::bit << 22,

					GK_KEY_L1	= detail::bit << 23,
					GK_KEY_L2	= detail::bit << 24,
					GK_KEY_L3	= detail::bit << 25,
					GK_KEY_L4	= detail::bit << 26,
					GK_KEY_L5	= detail::bit << 27,

			GK_KEY_MUTE_HEADSET	= detail::bit << 28,
			GK_KEY_MUTE_MICRO	= detail::bit << 29,

			GK_KEY_AUDIO_NEXT	= detail::bit << 30,
			GK_KEY_AUDIO_PREV	= detail::bit << 31,
			GK_KEY_AUDIO_STOP	= detail::bit << 32,
			GK_KEY_AUDIO_PLAY	= detail::bit << 33,
			GK_KEY_AUDIO_MUTE	= detail::bit << 34,
	GK_KEY_AUDIO_RAISE_VOLUME	= detail::bit << 35,
	GK_KEY_AUDIO_LOWER_VOLUME	= detail::bit << 36,

};

} // namespace GLogiK
