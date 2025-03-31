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

#ifndef SRC_BIN_GUI_QT_LICENSE_TAB_HPP_
#define SRC_BIN_GUI_QT_LICENSE_TAB_HPP_

#include "AboutDialogTab.hpp"

#define LICENSE_FILE_SHA1 "8624bcdae55baeef00cd11d5dfcfa60f68710a02"

namespace GLogiK
{

class LicenseTab
	:	public AboutDialogTab
{
	public:
		LicenseTab(void) = default;
		~LicenseTab(void) = default;

		void buildTab(void);

	protected:
	private:
};

} // namespace GLogiK

#endif
