/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2025  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_BIN_DAEMON_LCD_SCREEN_PLUGINS_MANAGER_HPP_
#define SRC_BIN_DAEMON_LCD_SCREEN_PLUGINS_MANAGER_HPP_

#include <cstdint>

#include <string>
#include <vector>

#include "LCDPlugins/PBM.hpp"
#include "LCDPlugins/LCDPlugin.hpp"
#include "LCDPlugins/fontsManager.hpp"

#include "include/LCDPP.hpp"

namespace GLogiK
{

class LCDScreenPluginsManager
{
	public:
		LCDScreenPluginsManager(const std::string & product);
		~LCDScreenPluginsManager(void);

		static const LCDPPArray_type _LCDPluginsPropertiesEmptyArray;

		const LCDPPArray_type & getLCDPluginsProperties(void) const;

		const bool findOneLCDScreenPlugin(const uint64_t LCDPluginsMask1) const;

		const PixelsData & getNextLCDScreenBuffer(
			const std::string & LCDKey,
			const uint64_t LCDPluginsMask1
		);
		const uint16_t getPluginTiming(void);

		void unlockPlugin(void);
		const uint64_t getCurrentPluginID(void);
		void jumpToNextPlugin(void);

	protected:

	private:
		std::vector<LCDPlugin*> _plugins;
		std::vector<LCDPlugin*>::iterator _itCurrentPlugin;

		LCDPPArray_type _pluginsPropertiesArray;

		PixelsData _LCDBuffer;
		FontsManager _fontsManager;
		FontsManager* const _pFonts;

		uint16_t _frameCounter;
		bool _noPlugins;
		bool _currentPluginLocked;

		void stopLCDPlugins(void);
		void dumpPBMDataIntoLCDBuffer(PixelsData & LCDBuffer, const PixelsData & PBMData);
};

} // namespace GLogiK

#endif
