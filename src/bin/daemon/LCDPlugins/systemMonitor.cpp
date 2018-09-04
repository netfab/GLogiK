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

#include <cmath>
#include <cfenv>

#include <cstdint>
#include <unistd.h>
#include <limits.h>

#include <iomanip>
#include <sstream>
#include <string>

#include <config.h>

#include "lib/utils/utils.hpp"

#include "netsnap/netSnapshots.hpp"
#include "systemMonitor.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

SystemMonitor::SystemMonitor()
	:	_lastRxStringSize(0)
{
	_pluginName = "systemMonitor";
	_pluginTempo = LCDPluginTempo::TEMPO_500_20;
}

SystemMonitor::~SystemMonitor() {
}

void SystemMonitor::init(FontsManager* const pFonts)
{
	fs::path PBMDirectory(PBM_DATA_DIR);
	PBMDirectory /= _pluginName;

	pFonts->initializeFont(FontID::MONOSPACE85);

	this->addPBMFrame(PBMDirectory, "skeleton.pbm");

	std::string host;
	{
		char hostname[HOST_NAME_MAX];
		gethostname(hostname, HOST_NAME_MAX);
		/* make sure we have a null character */
		hostname[HOST_NAME_MAX-1] = '\0';
		host.assign(hostname);
	}
	this->writeStringOnLastFrame(pFonts, FontID::MONOSPACE85, host, 6, 1);

	LCDPlugin::init(pFonts);
}

const PBMDataArray & SystemMonitor::getNextPBMFrame(
	FontsManager* const pFonts,
	const std::string & LCDKey)
{
	/* -- -- -- */
	std::string usedPhysicalMemory("");

	auto getPaddedPercentString = [] (unsigned int i) -> const std::string {
		std::ostringstream out("", std::ios_base::app);
		out << std::setw(3) << std::to_string(i) << " %";
		return out.str();
	};

	{
		struct sysinfo memInfo;
		sysinfo(&(memInfo));

		uint64_t usedPMem, totalPMem = 0;
		usedPMem  = memInfo.freeram;
		usedPMem *= memInfo.mem_unit;
		usedPMem *= 100;

		totalPMem  = memInfo.totalram;
		totalPMem *= memInfo.mem_unit;

		usedPMem /= totalPMem;
		usedPMem = 100 - usedPMem;

		usedPhysicalMemory = getPaddedPercentString(usedPMem);
	}

	/* -- -- -- */
	std::string usedCPUActiveTotal("");

	{
		CPUSnapshot s2;

		/* see cpu-stat - CPUStatsPrinter::GetPercActiveTotal() */
		const float ACTIVE_TIME		= s2.GetActiveTimeTotal() - _snapshot1.GetActiveTimeTotal();
		const float IDLE_TIME		= s2.GetIdleTimeTotal() - _snapshot1.GetIdleTimeTotal();
		const float TOTAL_TIME		= ACTIVE_TIME + IDLE_TIME;

		float cpuPercentTotal = 100.f * ACTIVE_TIME / TOTAL_TIME;

		std::fesetround(FE_TONEAREST);
		cpuPercentTotal = std::nearbyint(cpuPercentTotal);

		usedCPUActiveTotal = getPaddedPercentString(cpuPercentTotal);

		_snapshot1 = s2;
	}

	/* -- -- -- */
	auto getPaddedNetRateString = [] (
		const std::string & netRate,
		std::size_t & lastSize ) -> const std::string
	{
		std::string paddedNetRateString(netRate);

		std::size_t currentSize = netRate.size();
		if(lastSize > currentSize) {
			paddedNetRateString.assign( (lastSize - currentSize), ' ');
			paddedNetRateString += netRate;
		}

		lastSize = currentSize;
		return paddedNetRateString;
	};

	std::string paddedRXRateString("error");
	try {
		NetSnapshots n;
		const std::string rxRateString(n.getRxRateString());
		paddedRXRateString = getPaddedNetRateString(rxRateString, _lastRxStringSize);
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << "network calculations error : " << e.what();
	}

	/* -- -- -- */
	/* FontID::MONOSPACE85 char width is 5 pixels */
	const unsigned short FONT_CHAR_WIDTH = 5;

	/* padded percentage string size is always 5 chars */
	const unsigned int PERC_POS_X = (PBM_WIDTH - 1) - (5 * FONT_CHAR_WIDTH);
	const unsigned int NET_POS_X = (PBM_WIDTH - 1) - (paddedRXRateString.size() * FONT_CHAR_WIDTH);

	/* percent - max 5 chars */
	/* net rate - max 12 chars */
	this->writeStringOnFrame(pFonts, FontID::MONOSPACE85, usedCPUActiveTotal, PERC_POS_X, 14);
	this->writeStringOnFrame(pFonts, FontID::MONOSPACE85, paddedRXRateString, NET_POS_X, 23);
	this->writeStringOnFrame(pFonts, FontID::MONOSPACE85, usedPhysicalMemory, PERC_POS_X, 32);

	return LCDPlugin::getCurrentPBMFrame();
}

} // namespace GLogiK

