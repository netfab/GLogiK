/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2026  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_BIN_SERVICE_DESKTOP_SERVICE_SYSTRAY_HPP_
#define SRC_BIN_SERVICE_DESKTOP_SERVICE_SYSTRAY_HPP_

#include <string>

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QSystemTrayIcon>

#include <config.h>

#include "include/DeviceID.hpp"

#include "icons.hpp"

namespace GLogiK
{

enum class SystrayDeviceEvent : uint8_t
{
	DEVICE_START = 0,
	DEVICE_STOP,
	DEVICE_RESTART
};

class DesktopServiceSystray
	:	public QMainWindow
{
	public:
		DesktopServiceSystray(void);
		~DesktopServiceSystray(void);

		/* return true is any device event was triggered via systray */
		const bool isDeviceEventTriggered(void) const;
		const std::string & getDeviceEventID(void) const;    /* Event deviceID */
		const SystrayDeviceEvent getDeviceEvent(void) const; /* DeviceEvent (start/stop/restart) */

		void resetDeviceEvent(void);

		void updateContextMenu(const DevicesMap_type & devices);

		const bool wantToRestart(void) const;
		const bool wantToStop(void) const;

	protected:

	private:
		std::string _deviceID;
		SystrayDeviceEvent _deviceEvent;

		QSystemTrayIcon* _trayIcon;
		QMenu* _trayIconMenu;

		Icons _icons;

		bool _deviceEventTriggered;
		bool _wantToRestart;
		bool _wantToStop;

		void restartService(void);
		void stopService(void);

		void startDevice(void);
		void stopDevice(void);
		void restartDevice(void);

		void iconActivated(QSystemTrayIcon::ActivationReason reason);
};

inline const bool DesktopServiceSystray::isDeviceEventTriggered(void) const
{
	return _deviceEventTriggered;
}

inline const std::string & DesktopServiceSystray::getDeviceEventID(void) const
{
	return _deviceID;
}

inline const SystrayDeviceEvent DesktopServiceSystray::getDeviceEvent(void) const
{
	return _deviceEvent;
}

inline const bool DesktopServiceSystray::wantToRestart(void) const
{
	return _wantToRestart;
}

inline const bool DesktopServiceSystray::wantToStop(void) const
{
	return _wantToStop;
}

} // namespace GLogiK

#endif
