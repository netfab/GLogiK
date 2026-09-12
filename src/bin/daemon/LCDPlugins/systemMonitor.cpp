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

#include "config.h"

#include <cmath>
#include <cfenv>

#include <unistd.h>
#include <cstdint>
#include <climits>

#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include "include/enums.hpp"

#include "lib/shared/glogik.hpp"
#include "lib/utils/utils.hpp"

#include "systemMonitor.hpp"

namespace GLogiK::daemon
{

using namespace NSGKUtils;

SystemMonitor::SystemMonitor()
	:	_lastRateStringSize(0),
		_currentRate(NetDirection::NET_RX)
{
	_plugin.setID( toEnumType(LCDScreenPlugin::GK_LCD_SYSTEM_MONITOR) );
	_plugin.setName("systemMonitor");
	_plugin.setDesc("CPU, network and memory monitoring plugin");
	_pluginTempo = LCDPluginTempo::TEMPO_500_20;
}

SystemMonitor::~SystemMonitor()
{
}

void SystemMonitor::init(
	FontsManager* const pFonts,
	const std::string & product)
{
	fs::path PBMDirectory(PBM_DATA_DIR);
	PBMDirectory /= _plugin.getName();

	this->addPBMFrame(PBMDirectory, "skeleton.pbm");

	std::string host;
	{
		char hostname[HOST_NAME_MAX];
		gethostname(hostname, HOST_NAME_MAX);
		/* make sure we have a null character */
		hostname[HOST_NAME_MAX-1] = '\0';
		host.assign(hostname);
	}
	this->writeStringOnLastPBMFrame(pFonts, FontID::MONOSPACE85, host, 16, 1);

	LCDPlugin::init(pFonts, product);
}

const PixelsData & SystemMonitor::getNextPBMFrame(
	FontsManager* const pFonts,
	const std::string & LCDKey,
	const bool lockedPlugin)
{
	GK_LOG_FUNC

	this->drawPadlockOnPBMFrame(lockedPlugin);

	/* -- -- -- */
	std::string usedPhysicalMemory("");

	// lambda
	auto get_padded_percent_string = [] (unsigned int i) -> const std::string
	{
		std::ostringstream out("", std::ios_base::app);
		out << std::setw(3) << std::to_string(i) << " %";
		return out.str();
	};

	{
		std::map<const std::string, const std::string> memShot;

		std::uint64_t freePMem = 0;
		std::uint64_t totalPMem = 1;

		try
		{
			std::ifstream meminfo("/proc/meminfo");

			for( std::string line; std::getline(meminfo, line); )
			{
				std::istringstream iss(line);
				std::vector<std::string> words;

				for( std::string word; std::getline(iss, word, ' '); )
					if( ! word.empty() )
						words.push_back(word);

				if( ! words.empty() )
				{
					if( words.size() < 2 )
					{
#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
						GKLog2(warning, "problem parsing line: ", words[0])
#endif
						continue;
					}

					for(const auto & item : _memItems)
					{
						const std::string_view s(words[0]);
						// don't want trailing ':'
						const std::string_view::size_type count = s.size() - 1;

						if( s.substr(0, count) == item )
							memShot.insert(
								std::pair<const std::string, const std::string>(item, words[1])
							);
					}
				}

				if( memShot.size() == _memItems.size() )
				{
					//LOG(trace) << "found each item :-)";
					break;
				}
			}
		}
		catch (const std::ifstream::failure & e)
		{
			GKSysLogError("error opening/reading/closing /proc/meminfo : ", e.what());
			throw GLogiKExcept("ifstream error");
		}

		try
		{
			totalPMem = toULL( memShot.at("MemTotal") );

			// Linux Kernel 3.14+
			if( memShot.count("MemAvailable") == 1 )
				freePMem  = toULL( memShot["MemAvailable"] );
			else
			{
				freePMem  = toULL( memShot.at("MemFree") );
				freePMem += toULL( memShot.at("Buffers") );
				freePMem += toULL( memShot.at("Cached") );
			}
		}
		catch (const std::out_of_range& oor)
		{
			GKSysLogWarning("meminfo parsing problem : ", oor.what());
			freePMem = 0;
		}
		catch (const GLogiKExcept & e)
		{
			GKSysLogWarning("meminfo conversion failure : ", e.what());
			freePMem = 0;
		}

		float freeMem = 100 * freePMem / totalPMem;
		std::fesetround(FE_TONEAREST);
		freeMem = std::nearbyint(freeMem);

		std::uint16_t usedMem = static_cast<std::uint16_t>(freeMem);
		usedMem = 100 - usedMem;

		this->drawProgressBarOnPBMFrame(usedMem, 24, 33);
		usedPhysicalMemory = get_padded_percent_string(usedMem);
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

		this->drawProgressBarOnPBMFrame(cpuPercentTotal, 24, 15);
		usedCPUActiveTotal = get_padded_percent_string(cpuPercentTotal);

		_snapshot1 = s2;
	}

	/* -- -- -- */
	// lambda
	auto get_padded_rate_string = [] (const std::string & netRate, std::size_t & lastSize)
		-> const std::string
	{
		std::string paddedNetRateString(netRate);

		std::size_t currentSize = netRate.size();
		if(lastSize > currentSize)
		{
			paddedNetRateString.assign( (lastSize - currentSize), ' ');
			paddedNetRateString += netRate;
		}

		lastSize = currentSize;
		return paddedNetRateString;
	};

	std::string paddedRateString("error");
	try
	{
		NetSnapshots n;

		/* pressed L5, switching network direction */
		if(LCDKey == "L5")
		{
			if(_currentRate == NetDirection::NET_RX)
			{
				_currentRate = NetDirection::NET_TX;
#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
				GKLog2(trace, _plugin.getName(), " switched network rate to upload")
#endif
			}
			else
			{
				_currentRate = NetDirection::NET_RX;
#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
				GKLog2(trace, _plugin.getName(), " switched network rate to download")
#endif
			}
		}

		const std::string rateString(n.getRateString(_currentRate));
		paddedRateString = get_padded_rate_string(rateString, _lastRateStringSize);
	}
	catch (const GLogiKExcept & e)
	{
		paddedRateString = get_padded_rate_string(paddedRateString, _lastRateStringSize);
		GKLog2(error, "network calculations error : ", e.what());
	}

	/* -- -- -- */
	/* FontID::MONOSPACE85 char width is 5 pixels */
	const std::uint16_t FONT_CHAR_WIDTH = 5;

	/* padded percentage string size is always 5 chars */
	const std::uint16_t PERC_POS_X = (LCD_SCREEN_WIDTH - 1) - (5 * FONT_CHAR_WIDTH);
	const std::uint16_t  NET_POS_X =
		(LCD_SCREEN_WIDTH - 1) - (paddedRateString.size() * FONT_CHAR_WIDTH);

	/* percent - max 5 chars */
	/* net rate - max 12 chars */
	this->writeStringOnPBMFrame(pFonts, FontID::MONOSPACE85, usedCPUActiveTotal, PERC_POS_X, 14);
	this->writeStringOnPBMFrame(pFonts, FontID::MONOSPACE85, paddedRateString, NET_POS_X, 23);
	this->writeStringOnPBMFrame(pFonts, FontID::MONOSPACE85, usedPhysicalMemory, PERC_POS_X, 32);

	return LCDPlugin::getCurrentPBMFrame();
}

} // namespace GLogiK::daemon

