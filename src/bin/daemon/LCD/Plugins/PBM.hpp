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

#include <vector>

namespace Managers::LCDPlugins::PBM
{
	constexpr std::uint16_t default_height = 48;
	constexpr std::uint16_t default_width = 160;

	constexpr std::uint16_t   default_height_in_bytes = default_height / 8;
	constexpr std::uint16_t    default_width_in_bytes = default_width / 8;
	constexpr std::uint16_t        data_size_in_bytes = default_width_in_bytes * default_height;
	constexpr std::uint16_t data_header_size_in_bytes = 32; // LCD header length

	// formula when PBM_HEIGHT not multiple of 8
	// TODO do we need it ?
	//#define PBM_HEIGHT_IN_BYTES ((PBM_HEIGHT + ((8 - (PBM_HEIGHT % 8)) % 8)) / 8)

	/* LCD screen real sizes in pixels */
	constexpr std::uint16_t LCD_height = 43;
	constexpr std::uint16_t LCD_width = 160;

} // namespace Managers::LCDPlugins::PBM

namespace Managers::LCDPlugins::plugin
{

typedef std::vector<unsigned char> PixelsData;

} // namespace Managers::LCDPlugins::plugin
