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

#pragma once

#include "config.h"

#include <string>

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QSystemTrayIcon>

#include "include/DeviceID.hpp"

#include "icons.hpp"

namespace GLogiK
{

enum class SystrayDeviceEvent : std::uint8_t
{
	DEVICE_START = 0,
	DEVICE_STOP,
	DEVICE_RESTART
};

enum class SystrayServiceEvent : std::uint8_t
{
	SERVICE_RUN = 0,
	SERVICE_RESTART,
	SERVICE_STOP
};

class DesktopServiceSystray
	:	public QMainWindow
{
	public:
		DesktopServiceSystray();
		~DesktopServiceSystray();

		/* return true is any device event was triggered via systray */
		const bool isDeviceEventTriggered() const;
		const std::string & getDeviceEventID() const;    /* Event deviceID */
		const SystrayDeviceEvent getDeviceEvent() const; /* DeviceEvent (start/stop/restart) */

		const SystrayServiceEvent getServiceEvent(); /* ServiceEvent (stop/restart) */

		void resetDeviceEvent();

		void updateContextMenu(const DevicesMap_type & devices);

	protected:

	private:
		std::string _deviceID;
		SystrayDeviceEvent _deviceEvent;
		SystrayServiceEvent _serviceEvent;

		QSystemTrayIcon* _trayIcon;
		QMenu* _trayIconMenu;

		Icons _icons;

		bool _deviceEventTriggered;

		void restartService();
		void stopService();

		void startDevice();
		void stopDevice();
		void restartDevice();

		void iconActivated(QSystemTrayIcon::ActivationReason reason);
};

inline const bool DesktopServiceSystray::isDeviceEventTriggered() const
{
	return _deviceEventTriggered;
}

inline const std::string & DesktopServiceSystray::getDeviceEventID() const
{
	return _deviceID;
}

inline const SystrayDeviceEvent DesktopServiceSystray::getDeviceEvent() const
{
	return _deviceEvent;
}

inline const SystrayServiceEvent DesktopServiceSystray::getServiceEvent()
{
	const SystrayServiceEvent event = _serviceEvent;
	_serviceEvent = SystrayServiceEvent::SERVICE_RUN;
	return event;
}

} // namespace GLogiK
