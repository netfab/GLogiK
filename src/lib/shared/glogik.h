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

#ifndef __GLOGIK_H__
#define __GLOGIK_H__

namespace GLogiK
{

extern const char* unknown_device;

/* device */
#define GLOGIK_DEVICE_THREAD_DBUS_BUS_CONNECTION_NAME		"com.glogik.Device"

/* daemon */
#define GLOGIK_DAEMON_DBUS_ROOT_NODE						"/com/glogik/Daemon"
#define GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME				"com.glogik.Daemon"
	/* -- */
#define GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT			"ClientsManager"
#define GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH		GLOGIK_DAEMON_DBUS_ROOT_NODE "/" GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT
#define GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE		"com.glogik.Daemon.Client1"
	/* -- */
#define GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT			"DevicesManager"
#define GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH		GLOGIK_DAEMON_DBUS_ROOT_NODE "/" GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT
#define GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE		"com.glogik.Daemon.Device1"

/* desktop service */
#define GLOGIK_DESKTOP_SERVICE_DBUS_ROOT_NODE				"/com/glogik/Desktop/Service"
#define GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME		"com.glogik.Desktop.Service"
	/* -- */
#define GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT			"SystemMessageHandler"
#define GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT_PATH		GLOGIK_DESKTOP_SERVICE_DBUS_ROOT_NODE "/" GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT
#define GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE		"com.glogik.Desktop.Service.SystemMessageHandler1"

} // namespace GLogiK

#endif
