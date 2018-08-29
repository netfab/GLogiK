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

#include <new>

#include "lib/utils/utils.hpp"

#include "fontsManager.hpp"
#include "fontMonospace8_5.hpp"
#include "fontMonospace8_6.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

FontsManager::FontsManager()
{
}

FontsManager::~FontsManager()
{
	for(const auto & font_pair : _fonts) {
		delete font_pair.second;
	}
	_fonts.clear();
}

void FontsManager::initializeFont(const FontID fontID)
{
	try {
		_fonts.at(fontID);
	}
	catch (const std::out_of_range& oor) {
		PBMFont* font = nullptr;
		try {
			switch(fontID) {
				case FontID::MONOSPACE8_5:
					font = new fontMonospace8_5();
					break;
				case FontID::MONOSPACE8_6:
					font = new fontMonospace8_6();
					break;
				default:
					throw GLogiKExcept("unknown font ID");
			}
		}
		catch (const std::bad_alloc& e) { /* handle new() failure */
			throw GLogiKBadAlloc("font bad allocation");
		}

		_fonts[fontID] = font;
	}
}

void FontsManager::printCharacterOnFrame(
	const FontID fontID,
	PBMDataArray & frame,
	const std::string & c,
	unsigned int & PBMXPos,
	const unsigned int PBMYPos)
{
	try {
		_fonts.at(fontID)->printCharacterOnFrame(frame, c, PBMXPos, PBMYPos);
	}
	catch (const std::out_of_range& oor) {
		std::string warn("unknown font : ");
		warn += std::to_string(to_type(fontID));
		GKSysLog(LOG_WARNING, WARNING, warn);
		throw GLogiKExcept("unknown font ID");
	}
}

} // namespace GLogiK

