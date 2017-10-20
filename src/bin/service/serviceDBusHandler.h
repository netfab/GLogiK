/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2017  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef __GLOGIKS_DESKTOP_SERVICE_DBUS_HANDLER_H__
#define __GLOGIKS_DESKTOP_SERVICE_DBUS_HANDLER_H__

#include <string>

#include "devicesHandler.h"

#include "lib/dbus/GKDBus.h"

#include "include/glogik.h"

#define MAXIMUM_WARNINGS_BEFORE_FATAL_ERROR 10

namespace GLogiK
{

class ServiceDBusHandler
{
	public:
		ServiceDBusHandler(void);
		~ServiceDBusHandler(void);

		void updateSessionState(void);
		void checkDBusMessages(void);

	protected:

	private:
		uint8_t warn_count_;
		GKDBus* DBus;
		DevicesHandler devices;

		/* DCM - Daemon ClientsManager - to contact the ClientsManager */
		const char* DBus_DCM_object_path_	= GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH;
		const char* DBus_DCM_interface_		= GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE;

		/* SMH - System Message Handler - to declare signals we interested in from the daemon */
		const char* DBus_SMH_object_	= GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT;
		const char* DBus_SMH_interface_	= GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE;

		/* DDM - Daemon DevicesManager - to contact the DevicesManager */
		const char* DBus_DDM_object_path_	= GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH;
		const char* DBus_DDM_interface_		= GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE;

		std::string current_session_;	/* current session object path */
		std::string session_state_;		/* session state */

		void setCurrentSessionObjectPath(void);
		const std::string getCurrentSessionState(const bool logoff=false);

		void warnUnhandledSessionState(const std::string & state);
		void warnOrThrows(const std::string & warn);
		void reportChangedState(void);
		void somethingChanged(void);
};

} // namespace GLogiK

#endif
