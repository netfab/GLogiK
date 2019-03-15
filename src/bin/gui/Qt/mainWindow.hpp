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

#ifndef SRC_BIN_GUI_QT_MAIN_WINDOW_HPP_
#define SRC_BIN_GUI_QT_MAIN_WINDOW_HPP_

#include <sys/types.h>

#include <cstdio>

#include <string>
#include <map>

#include <QComboBox>
#include <QMainWindow>
#include <QWidget>
#include <QTabWidget>

#include "lib/dbus/GKDBus.hpp"
#include "lib/shared/deviceFile.hpp"

#define LogRemoteCallFailure \
	FATALERROR << remoteMethod.c_str() << CONST_STRING_METHOD_CALL_FAILURE << e.what();
#define LogRemoteCallGetReplyFailure \
	LOG(ERROR) << remoteMethod.c_str() << CONST_STRING_METHOD_REPLY_FAILURE << e.what();

namespace GLogiK
{

class MainWindow
	:	public QMainWindow
{
	public:
		MainWindow(QWidget *parent = 0);
		~MainWindow();

		void init(void);
		void build(void);

	private:
		NSGKDBus::GKDBus* _pDBus;
		pid_t _pid;
		FILE* _LOGfd;	/* log file descriptor */
		bool _GUIResetThrow;

		QComboBox* _devicesComboBox;

		void setTabWidgetPointers(
			const std::string & name,
			QTabWidget*& tabWidget,
			QWidget*& tab
		);
		void setTabEnabled(const std::string & name, const bool status);

		void initializeQtSignalsSlots(void);

		void updateDevicesList(void);
		void resetInterface(void);
		void updateInterface(int index);
		void checkDBusMessages(void);

		std::map<const std::string, DeviceFile> _devices;
};

} // namespace GLogiK

#endif

