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

#include "PBM.hpp"
#include "PBMFont.hpp"

namespace Managers::LCDPlugins::plugin
{

enum class FontID : std::uint8_t
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
					160,			// PBM width
					48,				// PBM height
					5,				// character width
					10				// character height
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
					160,			// PBM width
					48,				// PBM height
					6,				// character width
					10				// character height
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

} // namespace Managers::LCDPlugins::plugin
