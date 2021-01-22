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

#ifndef SRC_BIN_DAEMON_LCDPLUGINS_FONTS_MANAGER_HPP_
#define SRC_BIN_DAEMON_LCDPLUGINS_FONTS_MANAGER_HPP_

#include <cstdint>

#include <string>
#include <map>

#include "PBMFont.hpp"
#include "fonts.hpp"

namespace GLogiK
{

class FontsManager
{
	public:
		FontsManager(void);
		~FontsManager(void);

		const uint16_t getCenteredXPos(
			const FontID fontID,
			const std::string & string
		);

		const uint16_t getCenteredYPos(const FontID fontID);

		void printCharacterOnFrame(
			const FontID fontID,
			PixelsData & frame,
			const std::string & c,
			uint16_t & PBMXPos,
			const uint16_t PBMYPos
		);

	protected:

	private:
		std::map<const FontID, PBMFont*> _fonts;

		void initializeFont(const FontID fontID);
};

} // namespace GLogiK

#endif

