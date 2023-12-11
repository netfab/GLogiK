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

#ifndef SRC_BIN_DAEMON_LCDPLUGINS_PBM_HPP_
#define SRC_BIN_DAEMON_LCDPLUGINS_PBM_HPP_

#include <vector>

/* LCD screen real sizes in pixels */
#define				  LCD_SCREEN_HEIGHT		43
#define				   LCD_SCREEN_WIDTH		160

#define				 DEFAULT_PBM_HEIGHT		48
#define				  DEFAULT_PBM_WIDTH		160
#define		DEFAULT_PBM_HEIGHT_IN_BYTES		(DEFAULT_PBM_HEIGHT/8)
#define		 DEFAULT_PBM_WIDTH_IN_BYTES		(DEFAULT_PBM_WIDTH/8)

#define		  DEFAULT_PBM_DATA_IN_BYTES		(DEFAULT_PBM_WIDTH_IN_BYTES * DEFAULT_PBM_HEIGHT)

/* LCD header length */
#define			 LCD_DATA_HEADER_OFFSET		32

// formula when PBM_HEIGHT not multiple of 8
// TODO do we need it ?
//#define PBM_HEIGHT_IN_BYTES ((PBM_HEIGHT + ((8 - (PBM_HEIGHT % 8)) % 8)) / 8)

namespace GLogiK
{

typedef std::vector<unsigned char> PixelsData;

} // namespace GLogiK

#endif
