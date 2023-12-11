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

#include <new>

#include "lib/utils/utils.hpp"

#include "fontsManager.hpp"

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

const uint16_t FontsManager::getCenteredXPos(
	const FontID fontID,
	const std::string & string)
{
	try {
		return _fonts.at(fontID)->getCenteredXPos(string);
	}
	catch (const std::out_of_range& oor) {
		this->initializeFont(fontID);
		return _fonts.at(fontID)->getCenteredXPos(string);
	}
}

const uint16_t FontsManager::getCenteredYPos(const FontID fontID)
{
	try {
		return _fonts.at(fontID)->getCenteredYPos();
	}
	catch (const std::out_of_range& oor) {
		this->initializeFont(fontID);
		return _fonts.at(fontID)->getCenteredYPos();
	}
}

void FontsManager::printCharacterOnFrame(
	const FontID fontID,
	PixelsData & frame,
	const std::string & c,
	uint16_t & PBMXPos,
	const uint16_t PBMYPos)
{
	try {
		_fonts.at(fontID)->printCharacterOnFrame(frame, c, PBMXPos, PBMYPos);
	}
	catch (const std::out_of_range& oor) {
		this->initializeFont(fontID);
		_fonts.at(fontID)->printCharacterOnFrame(frame, c, PBMXPos, PBMYPos);
	}
}

void FontsManager::initializeFont(const FontID fontID)
{
	GK_LOG_FUNC

	GKLog2(trace, "initializing font ", toUInt(toEnumType(fontID)))

	PBMFont* font = nullptr;
	try {
		switch(fontID) {
			case FontID::MONOSPACE85:
				font = new FontMonospace85();
				break;
			case FontID::MONOSPACE86:
				font = new FontMonospace86();
				break;
			case FontID::DEJAVUSANSBOLD1616:
				font = new FontDejaVuSansBold1616();
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

} // namespace GLogiK

