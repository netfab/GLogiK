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

#include "lib/utils/utils.h"

#include "splashscreen.h"

namespace fs = boost::filesystem;

namespace GLogiK
{

using namespace NSGKUtils;

Splashscreen::Splashscreen() {
	this->name_ = "splashscreen";
	this->tempo_ = LCDPluginTempo::TEMPO_750_8;
}

Splashscreen::~Splashscreen() {
}

void Splashscreen::init(FontsManager* const pFonts)
{
	fs::path pbm_dir(PBM_DATA_DIR);
	pbm_dir /= this->name_;

	this->addPBMFrame(pbm_dir, "GLogiK02.pbm", 2);		/* frame #0 */
	this->addPBMFrame(pbm_dir, "GLogiK02.pbm", 4);		/*       #1 */

	pFonts->initializeFont(FontID::MONOSPACE8);
	pFonts->initializeFont(FontID::MONOSPACE8_5);

	LCDPlugin::init(pFonts);
}

const PBMDataArray & Splashscreen::getNextPBMFrame(FontsManager* const pFonts)
{
	if( this->getNextPBMFrameID() == 1 ) {
		std::string version(" version ");
		version += PACKAGE_VERSION;
		this->writeStringOnFrame(pFonts, FontID::MONOSPACE8, version, 32, 32);
		this->writeStringOnFrame(pFonts, FontID::MONOSPACE8_5, version, 32, 16);
	}
	return LCDPlugin::getCurrentPBMFrame();
}

} // namespace GLogiK

