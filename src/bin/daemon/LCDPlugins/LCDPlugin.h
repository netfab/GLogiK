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

#ifndef __GLOGIKD_LCD_SCREEN_PLUGIN_H__
#define __GLOGIKD_LCD_SCREEN_PLUGIN_H__

#include <vector>
#include <string>

#include "PBM.h"
#include "PBMFile.h"

namespace GLogiK
{

class LCDPlugin
	:	public PBMFile
{
	public:
		virtual ~LCDPlugin(void);

		virtual void init(void) = 0;

		const bool isInitialized(void) const;

	protected:
		LCDPlugin(void);

		bool initialized_;

	private:

};

} // namespace GLogiK

#endif
