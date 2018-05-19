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

#include <config.h>

#include <string>

#include "splashscreen.h"

namespace GLogiK
{

Splashscreen::Splashscreen() {
	this->name_ = "splashscreen";
	this->tempo_ = LCDPluginTempo::TEMPO_750_8;
}

Splashscreen::~Splashscreen() {
}

void Splashscreen::init(void)
{
	std::string dir(PBM_DATA_DIR);
	dir += "/";			// TODO boost::fs:path
	dir += this->name_;

	this->addPBMClearedFrame();
	this->addPBMFrame(dir, "GLogiK01.pbm");
	this->addPBMFrame(dir, "GLogiK02.pbm");
	this->addPBMFrame(dir, "GLogiK03.pbm");
	this->addPBMFrame(dir, "GLogiK04.pbm", 3);

	LCDPlugin::init();
}

} // namespace GLogiK

