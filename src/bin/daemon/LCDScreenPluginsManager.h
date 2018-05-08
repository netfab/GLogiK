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

#include <array>
#include <vector>

#include "LCDPlugins/LCDPlugin.h"

#define PBM_HEIGHT 48
#define PBM_WIDTH 160
#define PBM_HEIGHT_IN_BYTES ((PBM_HEIGHT + ((8 - (PBM_HEIGHT % 8)) % 8)) / 8)
#define PBM_WIDTH_IN_BYTES (PBM_WIDTH / 8)
#define PBM_DATA_IN_BYTES ( PBM_WIDTH_IN_BYTES * PBM_HEIGHT )
typedef std::array<unsigned char, PBM_DATA_IN_BYTES> PBMDataArray;

namespace GLogiK
{

class LCDScreenPluginsManager
{
	public:
		LCDScreenPluginsManager(void);
		~LCDScreenPluginsManager(void);

	protected:

	private:
		std::vector<LCDPlugin*> plugins_;

		void stopLCDPlugins(void);
		void dumpPBMDataIntoLCDBuffer(PBMDataArray & lcd_buffer, const PBMDataArray & pbm_data);
};

} // namespace GLogiK

#endif
