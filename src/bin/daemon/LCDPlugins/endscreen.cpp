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


#include <string>

#include <boost/filesystem.hpp>

#include <config.h>

#include "include/enums.hpp"

#include "lib/utils/utils.hpp"

#include "endscreen.hpp"

namespace fs = boost::filesystem;

namespace GLogiK
{

using namespace NSGKUtils;

Endscreen::Endscreen()
{
	_plugin.setID( toEnumType(LCDScreenPlugin::GK_LCD_ENDSCREEN) );
	_plugin.setName("endscreen");
	_plugin.setDesc("Endscreen plugin, used when releasing a device");
	//_pluginTempo = LCDPluginTempo::TEMPO_DEFAULT;
}

Endscreen::~Endscreen()
{
}

void Endscreen::init(FontsManager* const pFonts, const std::string & product)
{
	fs::path PBMDirectory(PBM_DATA_DIR);
	PBMDirectory /= _plugin.getName();

	this->addPBMEmptyFrame();	/* frame #0 */
	this->writeStringOnLastPBMFrame(pFonts, FontID::DEJAVUSANSBOLD1616, product, -1, -1);

	//this->addLCDFrame(PBMDirectory, "endscreen.pbm", 1); /* frame #0 */

	LCDPlugin::init(pFonts, product);
}

} // namespace GLogiK

