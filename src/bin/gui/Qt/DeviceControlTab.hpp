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

#ifndef SRC_BIN_GUI_QT_DEVICE_CONTROL_TAB_HPP_
#define SRC_BIN_GUI_QT_DEVICE_CONTROL_TAB_HPP_

#include <QFrame>
#include <QString>
#include <QWidget>
#include <QPushButton>
#include <QLabel>

namespace GLogiK
{

class DeviceControlTab
	:	public QWidget
{
	public:
		DeviceControlTab(const QString & name);
		~DeviceControlTab();

		void disableButtons(void);
		void updateButtonsStates(const bool status);
		void disableAndHide(void);

	private:
		DeviceControlTab() = delete;

		QLabel* _deviceStatus;
		QPushButton* _pStart;
		QPushButton* _pStop;
		QPushButton* _pRestart;
		QFrame* _line2;
		QFrame* _line3;

		void setVisibility(const bool visibility);
};

} // namespace GLogiK

#endif
