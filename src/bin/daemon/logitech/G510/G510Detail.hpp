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

#include <array>
#include <string_view>

#include "include/enums.hpp"

namespace GLogiK
{

namespace D_G510
{

namespace detail
{
	constexpr std::uint16_t G510_DRIVER_ID = ( 1u << 0 );
	constexpr char G510_VENDOR[] = "Logitech";
	constexpr char G510_VENDOR_ID[] = "046d";
	constexpr std::uint64_t bit = 1;

	enum class RKeys : std::uint64_t
	{ // Recognized Keys
		GK_KEY_G1	= bit << 0,
		GK_KEY_G2	= bit << 1,
		GK_KEY_G3	= bit << 2,
		GK_KEY_G4	= bit << 3,
		GK_KEY_G5	= bit << 4,
		GK_KEY_G6	= bit << 5,
		GK_KEY_G7	= bit << 6,
		GK_KEY_G8	= bit << 7,

		GK_KEY_G9	= bit << 8,
		GK_KEY_G10	= bit << 9,
		GK_KEY_G11	= bit << 10,
		GK_KEY_G12	= bit << 11,
		GK_KEY_G13	= bit << 12,
		GK_KEY_G14	= bit << 13,
		GK_KEY_G15	= bit << 14,
		GK_KEY_G16	= bit << 15,

		GK_KEY_G17	= bit << 16,
		GK_KEY_G18	= bit << 17,

	GK_KEY_LIGHT	= bit << 18,
		GK_KEY_M1	= bit << 19,
		GK_KEY_M2	= bit << 20,
		GK_KEY_M3	= bit << 21,
		GK_KEY_MR	= bit << 22,

		GK_KEY_L1	= bit << 23,
		GK_KEY_L2	= bit << 24,
		GK_KEY_L3	= bit << 25,
		GK_KEY_L4	= bit << 26,
		GK_KEY_L5	= bit << 27,

			GK_KEY_MUTE_HEADPHONES	= bit << 28,
				GK_KEY_MUTE_MICRO	= bit << 29,

				GK_KEY_AUDIO_NEXT	= bit << 30,
				GK_KEY_AUDIO_PREV	= bit << 31,
				GK_KEY_AUDIO_STOP	= bit << 32,
				GK_KEY_AUDIO_PLAY	= bit << 33,
				GK_KEY_AUDIO_MUTE	= bit << 34,
		GK_KEY_AUDIO_RAISE_VOLUME	= bit << 35,
		GK_KEY_AUDIO_LOWER_VOLUME	= bit << 36,
	};

	/* RKey - Recognized Keys */
	struct RKey
	{
		constexpr RKey(
			std::string_view name,
			RKeys key,
			std::uint16_t index,
			unsigned char mask
		)	:	name(name),
				key(key),
				index(index),
				mask(static_cast<unsigned char>(mask))
		{}
		std::string_view name;
		RKeys key;
		std::uint16_t index;
		unsigned char mask;
	};

	struct MKeyLed
	{
		constexpr MKeyLed(Leds led, unsigned int mask)
			:	led(led),
				mask(static_cast<unsigned char>(mask))
		{}
		const Leds led;
		const unsigned char mask;
	};

	using R = RKey;

	inline constexpr std::array keys5BytesMap =
	{
	//	R{ "Key",            RKeys::GK_KEY_               , 3, 1u << 2 },
		R{ "KeyLight",       RKeys::GK_KEY_LIGHT          , 3, 1u << 3 },
		R{ "MR",             RKeys::GK_KEY_MR             , 3, 1u << 7 },

		R{ "MuteHeadphones", RKeys::GK_KEY_MUTE_HEADPHONES, 4, 1u << 5 },
		R{ "MuteMicro",      RKeys::GK_KEY_MUTE_MICRO     , 4, 1u << 6 },
	//	R{ "Key"             RKeys::GK_KEY_               , 4, 1u << 7 },
	};

	inline constexpr std::array MKeys5BytesMap =
	{
		R{ "M1", RKeys::GK_KEY_M1, 3, 1u << 4 },
		R{ "M2", RKeys::GK_KEY_M2, 3, 1u << 5 },
		R{ "M3", RKeys::GK_KEY_M3, 3, 1u << 6 },
	};

	inline constexpr std::array GKeys5BytesMap =
	{
		R{ "G1",  RKeys::GK_KEY_G1 , 1, 1u << 0 },
		R{ "G2",  RKeys::GK_KEY_G2 , 1, 1u << 1 },
		R{ "G3",  RKeys::GK_KEY_G3 , 1, 1u << 2 },
		R{ "G4",  RKeys::GK_KEY_G4 , 1, 1u << 3 },
		R{ "G5",  RKeys::GK_KEY_G5 , 1, 1u << 4 },
		R{ "G6",  RKeys::GK_KEY_G6 , 1, 1u << 5 },
		R{ "G7",  RKeys::GK_KEY_G7 , 1, 1u << 6 },
		R{ "G8",  RKeys::GK_KEY_G8 , 1, 1u << 7 },

		R{ "G9",  RKeys::GK_KEY_G9 , 2, 1u << 0 },
		R{ "G10", RKeys::GK_KEY_G10, 2, 1u << 1 },
		R{ "G11", RKeys::GK_KEY_G11, 2, 1u << 2 },
		R{ "G12", RKeys::GK_KEY_G12, 2, 1u << 3 },
		R{ "G13", RKeys::GK_KEY_G13, 2, 1u << 4 },
		R{ "G14", RKeys::GK_KEY_G14, 2, 1u << 5 },
		R{ "G15", RKeys::GK_KEY_G15, 2, 1u << 6 },
		R{ "G16", RKeys::GK_KEY_G16, 2, 1u << 7 },

		R{ "G17", RKeys::GK_KEY_G17, 3, 1u << 0 },
		R{ "G18", RKeys::GK_KEY_G18, 3, 1u << 1 },
	};

	inline constexpr std::array LCDKeys5BytesMap =
	{
		R{ "L1", RKeys::GK_KEY_L1, 4, 1u << 0 },
		R{ "L2", RKeys::GK_KEY_L2, 4, 1u << 1 },
		R{ "L3", RKeys::GK_KEY_L3, 4, 1u << 2 },
		R{ "L4", RKeys::GK_KEY_L4, 4, 1u << 3 },
		R{ "L5", RKeys::GK_KEY_L5, 4, 1u << 4 },
	};

	inline constexpr std::array mediaKeys2BytesMap =
	{
		R{ "XF86AudioNext",        RKeys::GK_KEY_AUDIO_NEXT        , 1, 1u << 0 },
		R{ "XF86AudioPrev",        RKeys::GK_KEY_AUDIO_PREV        , 1, 1u << 1 },
		R{ "XF86AudioStop",        RKeys::GK_KEY_AUDIO_STOP        , 1, 1u << 2 },
		R{ "XF86AudioPlay",        RKeys::GK_KEY_AUDIO_PLAY        , 1, 1u << 3 },
		R{ "XF86AudioMute",        RKeys::GK_KEY_AUDIO_MUTE        , 1, 1u << 4 },
		R{ "XF86AudioRaiseVolume", RKeys::GK_KEY_AUDIO_RAISE_VOLUME, 1, 1u << 5 },
		R{ "XF86AudioLowerVolume", RKeys::GK_KEY_AUDIO_LOWER_VOLUME, 1, 1u << 6 },
	//	R{ "",                     RKeys::GK_KEY_, 1, 1u << 7 },
	};

	inline constexpr std::array<MKeyLed, 4> ledsMask =
	{
		{
			{ Leds::GK_LED_M1, 1u << 7 },
			{ Leds::GK_LED_M2, 1u << 6 },
			{ Leds::GK_LED_M3, 1u << 5 },
			{ Leds::GK_LED_MR, 1u << 4 },
		}
	};

	using P = std::pair<RKeys, GKeysID>;

	inline constexpr std::array RKeys2GKeysIDMap
	{
		P{ RKeys::GK_KEY_G1, GKeysID::GKEY_G1 },
		P{ RKeys::GK_KEY_G2, GKeysID::GKEY_G2 },
		P{ RKeys::GK_KEY_G3, GKeysID::GKEY_G3 },
		P{ RKeys::GK_KEY_G4, GKeysID::GKEY_G4 },
		P{ RKeys::GK_KEY_G5, GKeysID::GKEY_G5 },
		P{ RKeys::GK_KEY_G6, GKeysID::GKEY_G6 },
		P{ RKeys::GK_KEY_G7, GKeysID::GKEY_G7 },
		P{ RKeys::GK_KEY_G8, GKeysID::GKEY_G8 },
		P{ RKeys::GK_KEY_G9, GKeysID::GKEY_G9 },
		P{ RKeys::GK_KEY_G10, GKeysID::GKEY_G10 },
		P{ RKeys::GK_KEY_G11, GKeysID::GKEY_G11 },
		P{ RKeys::GK_KEY_G12, GKeysID::GKEY_G12 },
		P{ RKeys::GK_KEY_G13, GKeysID::GKEY_G13 },
		P{ RKeys::GK_KEY_G14, GKeysID::GKEY_G14 },
		P{ RKeys::GK_KEY_G15, GKeysID::GKEY_G15 },
		P{ RKeys::GK_KEY_G16, GKeysID::GKEY_G16 },
		P{ RKeys::GK_KEY_G17, GKeysID::GKEY_G17 },
		P{ RKeys::GK_KEY_G18, GKeysID::GKEY_G18 },
	};

	using P2 = std::pair<RKeys, MKeysID>;

	inline constexpr std::array RKeys2MKeysIDMap
	{
		P2{ RKeys::GK_KEY_M1, MKeysID::MKEY_M1 },
		P2{ RKeys::GK_KEY_M2, MKeysID::MKEY_M2 },
		P2{ RKeys::GK_KEY_M3, MKeysID::MKEY_M3 },
	};

} // namespace detail

} // namespace D_G510

} // namespace GLogiK
