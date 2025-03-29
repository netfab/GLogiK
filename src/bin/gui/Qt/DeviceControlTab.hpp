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

#ifndef SRC_BIN_GUI_QT_DEVICE_CONTROL_TAB_HPP_
#define SRC_BIN_GUI_QT_DEVICE_CONTROL_TAB_HPP_

#include <string>

#include <QFrame>
#include <QString>
#include <QPushButton>
#include <QLabel>

#include "lib/dbus/GKDBus.hpp"
#include "lib/shared/deviceProperties.hpp"

#include "Tab.hpp"

namespace GLogiK
{

class DeviceControlTab
	:	public Tab
{
	public:
		DeviceControlTab(
			NSGKDBus::GKDBus* pDBus,
			const QString & name
		);
		~DeviceControlTab();

		void disableButtons(void);

		void buildTab(void);
		void updateTab(
			const DeviceProperties & device,
			const std::string & devID
		);

		void disableAndHide(void);

	private:
		DeviceControlTab() = delete;

		std::string _devID;

		QLabel* _deviceStatusLabel;
		QPushButton* _pStartButton;
		QPushButton* _pStopButton;
		QPushButton* _pRestartButton;
		QFrame* _HLineFrame2;
		QFrame* _HLineFrame3;

		void setVisibility(const bool visibility);

		void startSignal(void);
		void stopSignal(void);
		void restartSignal(void);
		void sendStatusSignal(const std::string & signal);
};

} // namespace GLogiK

#endif
