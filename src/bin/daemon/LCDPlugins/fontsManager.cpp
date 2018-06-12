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

#include "lib/utils/utils.h"

#include "fontsManager.h"
#include "fontMonospace8.h"

namespace GLogiK
{

using namespace NSGKUtils;

FontsManager::FontsManager()
{
}

FontsManager::~FontsManager()
{
	for(const auto & font_pair : this->fonts_) {
		delete font_pair.second;
	}
	this->fonts_.clear();
}

void FontsManager::initializeFont(const FontID fontID)
{
	try {
		this->fonts_.at(fontID);
	}
	catch (const std::out_of_range& oor) {
		PBMFont* font = nullptr;
		try {
			switch(fontID) {
				case FontID::MONOSPACE8:
					font = new fontMonospace8();
					break;
				default:
					throw GLogiKExcept("unknown font ID");
			}
		}
		catch (const std::bad_alloc& e) { /* handle new() failure */
			throw GLogiKBadAlloc("font bad allocation");
		}

		this->fonts_[fontID] = font;
	}
}

void FontsManager::setFontPosition(
	const FontID fontID,
	const std::string & c)
{
	try {
		this->fonts_.at(fontID)->setCurrentPosition(c);
	}
	catch (const std::out_of_range& oor) {
		std::string warn("uninitialized font : ");
		warn += to_type(fontID);
		GKSysLog(LOG_WARNING, WARNING, warn);
		throw GLogiKExcept("unknown font ID");
	}
}

} // namespace GLogiK

