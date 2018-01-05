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

#ifndef __GLOGIK_H__
#define __GLOGIK_H__

namespace GLogiK
{

typedef const char* const c_str;

extern c_str s_Client;
extern c_str s_Device;
extern c_str s_UnknownClient;
extern c_str s_UnknownDevice;

/* --- ---- --- *
 * -- GKDBus -- *
 * --- ---- --- */
/* devices thread */
extern c_str GLOGIK_DEVICE_THREAD_DBUS_BUS_CONNECTION_NAME;

/* daemon thread */
extern c_str GLOGIK_DAEMON_DBUS_ROOT_NODE;
extern c_str GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME;
	/* -- */
extern c_str GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT;
extern c_str GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH;
extern c_str GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE;
	/* -- */
extern c_str GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT;
extern c_str GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH;
extern c_str GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE;

/* desktop service */
extern c_str GLOGIK_DESKTOP_SERVICE_DBUS_ROOT_NODE;
extern c_str GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME;
	/* -- */
extern c_str GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT;
extern c_str GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT_PATH;
extern c_str GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE;

} // namespace GLogiK

#endif
