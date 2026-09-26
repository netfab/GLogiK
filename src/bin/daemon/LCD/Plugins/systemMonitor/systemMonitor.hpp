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

#pragma once

#include <array>
#include <cstddef> // std::size_t
#include <string>
#include <string_view>

#include "cpu-stats/CPUSnapshot.h"
#include "netsnap/netSnapshots.hpp"

#include "bin/daemon/LCD/Plugins/LCDPlugin.hpp"
#include "bin/daemon/LCD/PixelsData.hpp"

namespace Managers::LCDPlugins::plugin
{

class FontsManager;

class SystemMonitor
	:	public LCDPlugin
{
	private:
		using PixelsData = ::Managers::LCDPlugins::PixelsData;

	public:
		SystemMonitor(void);
		~SystemMonitor(void);

		void init(
			FontsManager* const pFonts,
			const std::string & product
		);

		const PixelsData & getNextPBMFrame(
			FontsManager* const pFonts,
			const std::string & LCDKey,
			const bool lockedPlugin
		);

	protected:

	private:
		CPUSnapshot _snapshot1;
		std::size_t _lastRateStringSize;
		NetDirection _currentRate;

		const std::array<const std::string_view, 5> _memItems =
			{"MemTotal", "MemFree", "MemAvailable", "Buffers", "Cached"};
};

} // namespace Managers::LCDPlugins::plugin
