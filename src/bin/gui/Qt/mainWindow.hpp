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

#ifndef SRC_BIN_GUI_QT_MAIN_WINDOW_HPP_
#define SRC_BIN_GUI_QT_MAIN_WINDOW_HPP_

#include <sys/types.h>

#include <string>
#include <map>

#include <QComboBox>
#include <QMainWindow>
#include <QWidget>
#include <QTabWidget>

#include <boost/filesystem.hpp>

#include "lib/dbus/GKDBus.hpp"
#include "lib/shared/deviceProperties.hpp"

#include "include/DeviceID.hpp"
#include "include/DepsMap.hpp"

#include "DaemonAndServiceTab.hpp"
#include "DeviceControlTab.hpp"
#include "BacklightColorTab.hpp"
#include "LCDPluginsTab.hpp"
#include "GKeysTab.hpp"

namespace fs = boost::filesystem;

namespace GLogiK
{

enum class TabApplyButton : uint8_t
{
	TAB_BACKLIGHT = 1,
	TAB_LCD_PLUGINS,
	TAB_GKEYS
};

class MainWindow
	:	public QMainWindow
{
	public:
		MainWindow(QWidget *parent = 0);
		~MainWindow();

		void init(const int& argc, char *argv[]);
		void build(void);

	private:
		const NSGKDBus::BusConnection & _sessionBus = NSGKDBus::GKDBus::SessionBus;

		DeviceProperties _openedConfigurationFile;

		fs::path _configurationRootDirectory;
		fs::path _configurationFilePath;

		std::string _devID;

		DevicesMap_type _devices;
		GKDepsMap_type _DepsMap;

		NSGKDBus::GKDBus* _pDBus;

		QComboBox* _devicesComboBox;
		QTabWidget* _tabbedWidgets;

		DaemonAndServiceTab* _daemonAndServiceTab;
		DeviceControlTab* _deviceControlTab;
		BacklightColorTab* _backlightColorTab;
		LCDPluginsTab* _LCDPluginsTab;
		GKeysTab* _GKeysTab;

		const GKDepsMap_type* const _pDepsMap = &_DepsMap;

		int _statusBarTimeout;
		pid_t _pid;

		bool _ignoreNextSignal;

		/* -- -- -- */

		static void handleSignal(int signum);

		QWidget* getTabbedWidget(const std::string & name);

		void parseCommandLine(const int& argc, char *argv[]);

		void setTabEnabled(const std::string & name, const bool status);
		void setCurrentTab(const std::string & name);

		void getExecutablesDependenciesMap(void);
		void aboutDialog(void);

		void aboutToQuit(void);

		void updateDevicesList(void);
		void resetInterface(void);
		void updateInterface(int index);
		void checkDBusMessages(void);
		void saveConfigurationFile(const TabApplyButton tab);
		void saveConfigurationFileAndUpdateInterface(const TabApplyButton tab);

		void configurationFileUpdated(const std::string & devID);
};

} // namespace GLogiK

#endif

