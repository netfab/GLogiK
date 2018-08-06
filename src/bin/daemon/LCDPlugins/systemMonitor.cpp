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

#include <cstdint>
#include <unistd.h>
#include <limits.h>

#include <string>

#include <config.h>

#include "systemMonitor.h"

namespace GLogiK
{

SystemMonitor::SystemMonitor() {
	this->name_ = "systemMonitor";
	this->tempo_ = LCDPluginTempo::TEMPO_500_20;
}

SystemMonitor::~SystemMonitor() {
}

void SystemMonitor::init(FontsManager* const pFonts)
{
	fs::path pbm_dir(PBM_DATA_DIR);
	pbm_dir /= this->name_;

	pFonts->initializeFont(FontID::MONOSPACE8_5);

	this->addPBMFrame(pbm_dir, "skeleton.pbm");

	std::string host;
	{
		char hostname[HOST_NAME_MAX];
		gethostname(hostname, HOST_NAME_MAX);
		/* make sure we have a null character */
		hostname[HOST_NAME_MAX-1] = '\0';
		host.assign(hostname);
	}
	this->writeStringOnLastFrame(pFonts, FontID::MONOSPACE8_5, host, 6, 1);

	LCDPlugin::init(pFonts);
}

const PBMDataArray & SystemMonitor::getNextPBMFrame(FontsManager* const pFonts)
{
	sysinfo(&(this->memInfo_));

	uint64_t usedPMem, totalPMem = 0;
	usedPMem  = this->memInfo_.freeram;
	usedPMem *= this->memInfo_.mem_unit;
	usedPMem *= 100;

	totalPMem  = this->memInfo_.totalram;
	totalPMem *= this->memInfo_.mem_unit;

	usedPMem /= totalPMem;
	usedPMem = 100 - usedPMem;
	std::string usedPhysicalMemory( std::to_string(usedPMem) );
	usedPhysicalMemory += " %";


	this->writeStringOnFrame(pFonts, FontID::MONOSPACE8_5, usedPhysicalMemory, 132, 32);

	return LCDPlugin::getCurrentPBMFrame();
}

} // namespace GLogiK

