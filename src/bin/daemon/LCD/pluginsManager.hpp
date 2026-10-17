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

#include <cstdint>

#include <string>
#include <vector>

#include "fontsManager.hpp"
#include "PixelsData.hpp"

#include "include/LCDPP.hpp"

namespace Managers::LCDPlugins::plugin
{
class LCDPlugin;
}

namespace Managers::LCDPlugins
{

class LCDPluginsManager
{
	private:
		using LCDPPArray_type = ::GLogiK::LCDPPArray_type;
		using LCDPlugin = plugin::LCDPlugin;
		using FontsManager = Fonts::FontsManager;

	public:
		LCDPluginsManager();
		~LCDPluginsManager();

		void init(const std::string & product);

		static const LCDPPArray_type _LCDPluginsPropertiesEmptyArray;

		const LCDPPArray_type & getLCDPluginsProperties() const;

		bool findOneLCDScreenPlugin(std::uint64_t LCDPluginsMask1) const;

		const PixelsData & getNextLCDScreenBuffer(
			const std::string & LCDKey,
			std::uint64_t LCDPluginsMask1
		);
		std::uint16_t getPluginTiming();

		void unlockPlugin();
		std::uint64_t getCurrentPluginID();
		void jumpToNextPlugin();

	protected:

	private:
		std::vector<LCDPlugin*> _plugins;
		std::vector<LCDPlugin*>::iterator _itCurrentPlugin;

		LCDPPArray_type _pluginsPropertiesArray;

		PixelsData _LCDBuffer;
		FontsManager _fontsManager;
		FontsManager* const _pFonts;

		std::uint16_t _frameCounter;
		bool _noPlugins;
		bool _currentPluginLocked;

		void stopLCDPlugins();
		void dumpPBMDataIntoLCDBuffer(const PixelsData & PBMData);
};

} // namespace Managers::LCDPlugins
