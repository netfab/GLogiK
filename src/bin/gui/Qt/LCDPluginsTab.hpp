/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_BIN_GUI_QT_LCD_PLUGINS_TAB_HPP_
#define SRC_BIN_GUI_QT_LCD_PLUGINS_TAB_HPP_

#include <cstdint>

#include <string>

#include <QTableWidget>

#include "lib/utils/utils.hpp"
#include "lib/shared/deviceProperties.hpp"

#include "Tab.hpp"

namespace GLogiK
{

class LCDPluginsTab
	:	public Tab
{
	public:
		LCDPluginsTab(
			NSGKDBus::GKDBus* pDBus,
			const QString & name
		);
		~LCDPluginsTab();

		void buildTab(void);
		void updateTab(
			const std::string & devID,
			const DeviceProperties & device
		);

		const uint64_t getAndSetNewLCDPluginsMask(void);

	protected:
	private:
		uint64_t _LCDPluginsMask;
		uint64_t _newLCDPluginsMask;

		const std::string _idProperty = "pluginID";

		QTableWidget* _pPluginsTable;

		void toggleCheckbox(int row, int column);
		void updateNewLCDPluginsMask(int checkboxState);

};

} // namespace GLogiK

#endif
