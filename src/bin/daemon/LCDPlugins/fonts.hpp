/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_BIN_DAEMON_LCDPLUGINS_FONTS_HPP_
#define SRC_BIN_DAEMON_LCDPLUGINS_FONTS_HPP_

#include <cstdint>

#include "PBM.hpp"
#include "PBMFont.hpp"

#define DEFAULT_PBM_FONT_HEIGHT DEFAULT_PBM_HEIGHT
#define  DEFAULT_PBM_FONT_WIDTH DEFAULT_PBM_WIDTH

namespace GLogiK
{

enum class FontID : uint8_t
{
			MONOSPACE85	= 1 << 0,
			MONOSPACE86	= 1 << 1,
	 DEJAVUSANSBOLD1616	= 1 << 2,
};

/* -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
/* -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */

class FontMonospace85
	:	public PBMFont
{
	public:
		FontMonospace85(void)
			:	PBMFont(
					"monospace85",
					DEFAULT_PBM_FONT_WIDTH,
					DEFAULT_PBM_FONT_HEIGHT,
					5,
					10
				) {};
		~FontMonospace85() = default;

	protected:

	private:

};

class FontMonospace86
	:	public PBMFont
{
	public:
		FontMonospace86(void)
			:	PBMFont(
					"monospace86",
					DEFAULT_PBM_FONT_WIDTH,
					DEFAULT_PBM_FONT_HEIGHT,
					6,
					10
				) {};
		~FontMonospace86() = default;

	protected:

	private:

};

class FontDejaVuSansBold1616
	:	public PBMFont
{
	public:
		FontDejaVuSansBold1616(void)
			:	PBMFont(
					"DejaVuSansBold1616",	// font name
					416,					// PBM width
					64,						// PBM height
					16,						// character width
					16,						// character height
					3,						// font left shift
					2						// extra left shift
				) {};
		~FontDejaVuSansBold1616() = default;

	protected:

	private:

};

} // namespace GLogiK

#endif

