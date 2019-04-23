/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2019  Fabrice Delliaux <netbox253@gmail.com>
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

#include "splashscreen.hpp"

namespace fs = boost::filesystem;

namespace GLogiK
{

using namespace NSGKUtils;

Splashscreen::Splashscreen()
{
	_plugin.setID( toEnumType(LCDScreenPlugin::GK_LCD_SPLASHSCREEN) );
	_plugin.setName("splashscreen");
	_pluginTempo = LCDPluginTempo::TEMPO_400_15;
}

Splashscreen::~Splashscreen() {
}

void Splashscreen::init(FontsManager* const pFonts)
{
	fs::path PBMDirectory(PBM_DATA_DIR);
	PBMDirectory /= _plugin.getName();

	pFonts->initializeFont(FontID::MONOSPACE85);

	this->addPBMClearedFrame();	/* frame #0 */
	this->addPBMFrame(PBMDirectory, "splashscreen01.pbm", 1); /* #1 */
	this->addPBMFrame(PBMDirectory, "splashscreen01.pbm", 3); /* #2 #3 #4 */

	std::string version(" version "); version += PACKAGE_VERSION;
	this->writeStringOnLastFrame(pFonts, FontID::MONOSPACE85, version, 48, 32);

	LCDPlugin::init(pFonts);
}

} // namespace GLogiK

