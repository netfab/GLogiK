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


#include <string>

#include <boost/filesystem.hpp>

#include <config.h>

#include "splashscreen.hpp"

namespace fs = boost::filesystem;

namespace GLogiK
{

Splashscreen::Splashscreen() {
	_pluginName = "splashscreen";
	_pluginTempo = LCDPluginTempo::TEMPO_500_20;
}

Splashscreen::~Splashscreen() {
}

void Splashscreen::init(FontsManager* const pFonts)
{
	fs::path PBMDirectory(PBM_DATA_DIR);
	PBMDirectory /= _pluginName;

	pFonts->initializeFont(FontID::MONOSPACE85);
	//pFonts->initializeFont(FontID::MONOSPACE86);

	this->addPBMFrame(PBMDirectory, "splashscreen01.pbm", 1);		/* frame #0 */
	this->addPBMFrame(PBMDirectory, "splashscreen02.pbm", 2);		/*       #1 */
	this->addPBMFrame(PBMDirectory, "splashscreen02.pbm", 7);		/*       #2 */

	std::string version(" version "); version += PACKAGE_VERSION;
	this->writeStringOnLastFrame(pFonts, FontID::MONOSPACE85, version, 48, 32);

	LCDPlugin::init(pFonts);
}

const PBMDataArray & Splashscreen::getNextPBMFrame(
	FontsManager* const pFonts,
	const std::string & LCDKey)
{
	//if( this->getNextPBMFrameID() == 2 ) {
	//	std::string version(" version ");
	//	version += PACKAGE_VERSION;
	//	this->writeStringOnFrame(pFonts, FontID::MONOSPACE86, version, 48, 32);
	//}

	return LCDPlugin::getCurrentPBMFrame();
}

} // namespace GLogiK

