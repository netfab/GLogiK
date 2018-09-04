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

#ifndef SRC_BIN_DAEMON_LCDPLUGINS_SYSTEM_MONITOR_HPP_
#define SRC_BIN_DAEMON_LCDPLUGINS_SYSTEM_MONITOR_HPP_

#include "sys/sysinfo.h"

#include "cpu-stats/CPUSnapshot.h"

#include "LCDPlugin.hpp"

#include "netsnap/netSnapshots.hpp"

namespace GLogiK
{

class SystemMonitor
	:	public LCDPlugin
{
	public:
		SystemMonitor(void);
		~SystemMonitor(void);

		void init(FontsManager* const pFonts);

		const PBMDataArray & getNextPBMFrame(
			FontsManager* const pFonts,
			const std::string & LCDKey
		);

	protected:

	private:
		CPUSnapshot _snapshot1;
		std::size_t _lastRateStringSize;
		NetDirection _currentRate;

};

} // namespace GLogiK

#endif
