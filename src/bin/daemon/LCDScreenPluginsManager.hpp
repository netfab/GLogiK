/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2019  Fabrice Delliaux <netbox253@gmail.com>
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

#include <string>
#include <vector>

#include "include/LCDPluginProperties.hpp"

#include "LCDPlugins/PBM.hpp"
#include "LCDPlugins/LCDPlugin.hpp"
#include "LCDPlugins/fontsManager.hpp"

namespace GLogiK
{

class LCDScreenPluginsManager
{
	public:
		LCDScreenPluginsManager(void);
		~LCDScreenPluginsManager(void);

		static const LCDPluginsPropertiesArray_type _LCDPluginsPropertiesEmptyArray;

		const LCDPluginsPropertiesArray_type & getLCDPluginsProperties(void) const;

		LCDDataArray & getNextLCDScreenBuffer(
			const std::string & LCDKey,
			const uint64_t defaultLCDPluginsMask1
		);
		const unsigned short getPluginTiming(void);
		void forceNextPlugin(void);

	protected:

	private:
		unsigned short _frameCounter;
		bool _noPlugins;
		std::vector<LCDPlugin*> _plugins;
		std::vector<LCDPlugin*>::iterator _itCurrentPlugin;

		LCDPluginsPropertiesArray_type _pluginsPropertiesArray;

		LCDDataArray _LCDBuffer;
		FontsManager _fontsManager;
		FontsManager* const _pFonts;

		void stopLCDPlugins(void);
		void dumpPBMDataIntoLCDBuffer(LCDDataArray & LCDBuffer, const PBMDataArray & PBMData);
};

} // namespace GLogiK

#endif
