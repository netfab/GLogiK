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

#ifndef SRC_BIN_GUI_QT_DAEMON_AND_SERVICE_TAB_HPP_
#define SRC_BIN_GUI_QT_DAEMON_AND_SERVICE_TAB_HPP_

#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QWidget>

#include "Tab.hpp"

namespace GLogiK
{

class DaemonAndServiceTab
	:	public Tab,
		public QWidget
{
	public:
		DaemonAndServiceTab(
			NSGKDBus::GKDBus* pDBus,
			const QString & name
		);
		~DaemonAndServiceTab();

		void updateTab(void);
		const bool isServiceStarted(void) const;
		const bool isServiceRegistered(void) const;

	private:
		DaemonAndServiceTab() = delete;

		QLabel* _daemonVersionLabel;
		QLabel* _serviceVersionLabel;
		QLabel* _serviceStatusLabel;
		bool _serviceStarted;
		bool _serviceRegistered;

		QPushButton* _pStartButton;

		void startSignal(void);
};

} // namespace GLogiK

#endif
