/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2018  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef __GLOGIKD_FONTS_MANAGER_H__
#define __GLOGIKD_FONTS_MANAGER_H__

#include <cstdint>

#include <string>
#include <map>

#include "PBMFont.h"

namespace GLogiK
{

enum class FontID : uint8_t
{
	MONOSPACE8 = 1 << 0,
};

class FontsManager
{
	public:
		FontsManager(void);
		~FontsManager(void);

		void initializeFont(const FontID fontID);

		void printCharacterOnFrame(
			const FontID fontID,
			PBMDataArray & frame,
			const std::string & c,
			unsigned int & PBMXPos,
			const unsigned int PBMYPos
		);

	protected:

	private:
		std::map<const FontID, PBMFont*> fonts_;

};

} // namespace GLogiK

#endif

