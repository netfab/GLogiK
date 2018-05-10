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

#ifndef __GLOGIKD_LCD_SCREEN_PLUGINS_MANAGER_H__
#define __GLOGIKD_LCD_SCREEN_PLUGINS_MANAGER_H__

#include <vector>

#include "LCDPlugins/PBM.h"
#include "LCDPlugins/LCDPlugin.h"

namespace GLogiK
{

class LCDScreenPluginsManager
{
	public:
		LCDScreenPluginsManager(void);
		~LCDScreenPluginsManager(void);

		LCDDataArray & getNextLCDScreenBuffer(void);

	protected:

	private:
		std::vector<LCDPlugin*> plugins_;
		std::vector<LCDPlugin*>::iterator current_plugin_;

		LCDDataArray lcd_buffer_;

		void stopLCDPlugins(void);
		void dumpPBMDataIntoLCDBuffer(LCDDataArray & lcd_buffer, const PBMDataArray & pbm_data);
};

} // namespace GLogiK

#endif
