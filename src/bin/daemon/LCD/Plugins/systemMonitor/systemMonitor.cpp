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
#include <limits>
#include <string>
#include <string_view>
#include <vector>
#include <map>

#include "include/enums.hpp"

#include "lib/shared/glogik.hpp"
#include "lib/utils/utils.hpp"

#include "bin/daemon/LCD/Fonts/fonts.hpp"
#include "systemMonitor.hpp"

namespace Managers::LCDPlugins::plugin
{

using namespace NSGKUtils;

SystemMonitor::SystemMonitor()
	:	_lastRateStringSize(0),
		_currentRate(NetDirection::NET_RX)
{
	using LCDScreenPlugin = ::GLogiK::LCDScreenPlugin;
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
	GK_LOG_FUNC

	fs::path PBMDirectory(PBM_DATA_DIR);
	PBMDirectory /= _plugin.getName();

	this->addFrameFromPBM(PBMDirectory, "skeleton.pbm");

	std::string host;
	{
		char hostname[HOST_NAME_MAX];
		gethostname(hostname, HOST_NAME_MAX);
		/* make sure we have a null character */
		hostname[HOST_NAME_MAX-1] = '\0';
		host.assign(hostname);
	}

	using FontID = Fonts::FontID;
	this->writeStringOnLastFrame(pFonts, FontID::MONOSPACE85, host, 16, 1);

	LCDPlugin::init(pFonts, product);
}

const PixelsData & SystemMonitor::getNextFrame(
	FontsManager* const pFonts,
	const std::string & LCDKey,
	const bool lockedPlugin)
{
	GK_LOG_FUNC

	this->drawPadlockOnFrame(lockedPlugin);

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
		std::map<std::string_view, const std::uint64_t> memShot;

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

					for(const std::string_view & item : SystemMonitor::memItems)
					{
						const std::string_view s(words[0]);
						// don't want trailing ':'
						const std::string_view::size_type count = s.size() - 1;


						if( s.substr(0, count) == item )
						{
							try
							{
								memShot.emplace(item, toUInt64(words[1]));
							}
							catch (const GLogiKExcept & e)
							{
								GKSysLogWarning("meminfo item conversion failure : ", e.what());
							}
						}
					}
				}

				if( memShot.size() == SystemMonitor::memItems.size() )
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
			totalPMem = memShot.at("MemTotal");

			// Linux Kernel 3.14+
			if( memShot.count("MemAvailable") == 1 )
				freePMem  = memShot["MemAvailable"];
			else
			{
				// lambda
				auto add = [&freePMem](std::uint64_t value) -> void
				{
					constexpr auto max = std::numeric_limits<std::uint64_t>::max();
					if(freePMem > max - value)
						throw GLogiKExcept("sum overflow");
					freePMem += value;
				};

				add( memShot.at("MemFree") );
				add( memShot.at("Buffers") );
				add( memShot.at("Cached") );
			}
		}
		catch (const std::out_of_range& oor)
		{
			GKSysLogWarning("meminfo parsing problem : ", oor.what());
			freePMem = 0;
		}

		double freeMem = static_cast<double>(freePMem) / static_cast<double>(totalPMem) * 100.0;
		std::fesetround(FE_TONEAREST);
		freeMem = std::nearbyint(freeMem);

		std::uint16_t usedMem = static_cast<std::uint16_t>(freeMem);
		usedMem = 100 - usedMem;

		this->drawProgressBarOnFrame(usedMem, 24, 33);
		usedPhysicalMemory = get_padded_percent_string(usedMem);
	}

	/* -- -- -- */
	std::string usedCPUActive("");

	{
		CPUSnapshot s2;

		/* see cpu-stat - CPUStatsPrinter::GetPercActiveTotal() */
		const float ACTIVE_TIME		= static_cast<float>(s2.GetActiveTimeTotal() - _snapshot1.GetActiveTimeTotal());
		const float IDLE_TIME		= static_cast<float>(s2.GetIdleTimeTotal() - _snapshot1.GetIdleTimeTotal());
		const float TOTAL_TIME		= ACTIVE_TIME + IDLE_TIME;

		float cpuPercentTotal = 100.f * ACTIVE_TIME / TOTAL_TIME;

		std::fesetround(FE_TONEAREST);
		cpuPercentTotal = std::nearbyint(cpuPercentTotal);

		std::uint16_t cpuPercent = static_cast<std::uint16_t>(cpuPercentTotal);

		this->drawProgressBarOnFrame(cpuPercent, 24, 15);
		usedCPUActive = get_padded_percent_string(cpuPercent);

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
	// lambda
	auto get_posX = [] (unsigned int size) -> std::int16_t
	{
		/* FontID::MONOSPACE85 char width is 5 pixels */
		const unsigned int FONT_CHAR_WIDTH = 5;
		const unsigned int base = (PBM::LCD_width - 1);
		return static_cast<std::int16_t>(base - (size * FONT_CHAR_WIDTH));
	};

	/* padded percentage string size is always 5 chars */
	const std::int16_t PERC_POS_X = get_posX(5);
	const std::int16_t  NET_POS_X = get_posX( static_cast<unsigned int>(paddedRateString.size()) );

	using FontID = Fonts::FontID;

	/* percent - max 5 chars */
	/* net rate - max 12 chars */
	this->writeStringOnFrame(pFonts, FontID::MONOSPACE85, usedCPUActive, PERC_POS_X, 14);
	this->writeStringOnFrame(pFonts, FontID::MONOSPACE85, paddedRateString, NET_POS_X, 23);
	this->writeStringOnFrame(pFonts, FontID::MONOSPACE85, usedPhysicalMemory, PERC_POS_X, 32);

	return LCDPlugin::getCurrentFrame();
}

} // namespace Managers::LCDPlugins::plugin

