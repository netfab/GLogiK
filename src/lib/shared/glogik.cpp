
/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2018  Fabrice Delliaux <netbox253@gmail.com>
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

#include "glogik.h"

namespace GLogiK
{

c_str s_Client = "client : ";
c_str s_Device = "device : ";
c_str s_UnknownClient = "unknown client : ";
c_str s_UnknownDevice = "unknown device : ";
c_str s_RemoteCallFailure = " call failure : ";
c_str s_RemoteCallGetReplyFailure = " get reply failure : ";

/* --- ---- --- *
 * -- GKDBus -- *
 * --- ---- --- */
/* devices thread */
c_str GLOGIK_DEVICE_THREAD_DBUS_BUS_CONNECTION_NAME					= "com.glogik.Device";

/* daemon thread */
c_str GLOGIK_DAEMON_DBUS_ROOT_NODE									= "/com/glogik/Daemon";
c_str GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME						= "com.glogik.Daemon";
	/* -- */
c_str GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT						= "ClientsManager";
c_str GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH				= "/com/glogik/Daemon/ClientsManager";
c_str GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE					= "com.glogik.Daemon.Client1";
	/* -- */
c_str GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT						= "DevicesManager";
c_str GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH				= "/com/glogik/Daemon/DevicesManager";
c_str GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE					= "com.glogik.Daemon.Device1";

/* desktop service */
c_str GLOGIK_DESKTOP_SERVICE_DBUS_ROOT_NODE							= "/com/glogik/Desktop/Service";
c_str GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME				= "com.glogik.Desktop.Service";
	/* -- */
c_str GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT			= "SystemMessageHandler";
c_str GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT_PATH	= "/com/glogik/Desktop/Service/SystemMessageHandler";
c_str GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE		= "com.glogik.Desktop.Service.SystemMessageHandler1";

} // namespace GLogiK

