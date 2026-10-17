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

#include <string_view>

#include "lib/utils/utils.hpp"

#include "fontsManager.hpp"

namespace Managers::LCDPlugins::Fonts
{

using namespace NSGKUtils;

FontsManager::FontsManager()
{
}

FontsManager::~FontsManager()
{
}

std::uint16_t FontsManager::getCenteredXPos(
	const FontID fontID,
	const std::string_view sv)
{
	return this->getFont(fontID)->getCenteredXPos(sv);
}

std::uint16_t FontsManager::getCenteredYPos(const FontID fontID)
{
	return this->getFont(fontID)->getCenteredYPos();
}

void FontsManager::printCharacterOnFrame(
	const FontID fontID,
	PixelsData & frame,
	const std::string_view sv,
	std::uint16_t & PBMXPos,
	const std::uint16_t PBMYPos)
{
	this->getFont(fontID)->printCharacterOnFrame(frame, sv, PBMXPos, PBMYPos);
}

inline PBMFont* FontsManager::getFont(FontID fontID) const
{
	return _fonts[static_cast<std::size_t>(fontID)];
}

} // namespace Managers::LCDPlugins::Fonts
