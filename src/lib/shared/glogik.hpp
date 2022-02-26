/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2022  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_LIB_SHARED_GLOGIK_HPP_
#define SRC_LIB_SHARED_GLOGIK_HPP_

#define GLOGIKS_DESKTOP_SERVICE_NAME "GLogiKs"

namespace GLogiK
{

typedef const char* const c_str;

extern c_str CONST_STRING_CLIENT;
extern c_str CONST_STRING_DEVICE;
extern c_str CONST_STRING_UNKNOWN_CLIENT;
extern c_str CONST_STRING_UNKNOWN_DEVICE;
extern c_str CONST_STRING_METHOD_CALL_FAILURE;
extern c_str CONST_STRING_METHOD_REPLY_FAILURE;

/*   G Keys */
extern c_str G_KEY_G1;
extern c_str G_KEY_G2;
extern c_str G_KEY_G3;
extern c_str G_KEY_G4;
extern c_str G_KEY_G5;
extern c_str G_KEY_G6;
extern c_str G_KEY_G7;
extern c_str G_KEY_G8;
extern c_str G_KEY_G9;
extern c_str G_KEY_G10;
extern c_str G_KEY_G11;
extern c_str G_KEY_G12;
extern c_str G_KEY_G13;
extern c_str G_KEY_G14;
extern c_str G_KEY_G15;
extern c_str G_KEY_G16;
extern c_str G_KEY_G17;
extern c_str G_KEY_G18;

/* LCD Keys */
extern c_str LCD_KEY_L1;
extern c_str LCD_KEY_L2;
extern c_str LCD_KEY_L3;
extern c_str LCD_KEY_L4;
extern c_str LCD_KEY_L5;

/* --- ---- --- *
 * -- GKDBus -- *
 * --- ---- --- */
/* devices thread */
extern c_str GLOGIK_DEVICE_THREAD_DBUS_BUS_CONNECTION_NAME;

/* daemon thread */
extern c_str GLOGIK_DAEMON_DBUS_ROOT_NODE;
extern c_str GLOGIK_DAEMON_DBUS_ROOT_NODE_PATH;
extern c_str GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME;
	/* -- */
extern c_str GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT;
extern c_str GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH;
extern c_str GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE;
	/* -- */
extern c_str GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT;
extern c_str GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH;
extern c_str GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE;

/* desktop service launcher */
extern c_str GLOGIK_DESKTOP_SERVICE_LAUNCHER_DBUS_ROOT_NODE;
extern c_str GLOGIK_DESKTOP_SERVICE_LAUNCHER_DBUS_ROOT_NODE_PATH;
extern c_str GLOGIK_DESKTOP_SERVICE_LAUNCHER_DBUS_BUS_CONNECTION_NAME;
	/* -- */
//extern c_str GLOGIK_DESKTOP_SERVICE_LAUNCHER_SESSION_DBUS_OBJECT;
//extern c_str GLOGIK_DESKTOP_SERVICE_LAUNCHER_SESSION_DBUS_OBJECT_PATH;
//extern c_str GLOGIK_DESKTOP_SERVICE_LAUNCHER_SESSION_DBUS_INTERFACE;

/* desktop service */
extern c_str GLOGIK_DESKTOP_SERVICE_DBUS_ROOT_NODE;
extern c_str GLOGIK_DESKTOP_SERVICE_DBUS_ROOT_NODE_PATH;
extern c_str GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME;
	/* -- */
//extern c_str GLOGIK_DESKTOP_SERVICE_SYSTEM_DBUS_OBJECT;
//extern c_str GLOGIK_DESKTOP_SERVICE_SYSTEM_DBUS_OBJECT_PATH;
//extern c_str GLOGIK_DESKTOP_SERVICE_SYSTEM_DBUS_INTERFACE;
	/* -- */
extern c_str GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT;
extern c_str GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH;
extern c_str GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE;

/* Qt5 gui */
extern c_str GLOGIK_DESKTOP_QT5_DBUS_ROOT_NODE;
extern c_str GLOGIK_DESKTOP_QT5_DBUS_ROOT_NODE_PATH;
extern c_str GLOGIK_DESKTOP_QT5_DBUS_BUS_CONNECTION_NAME;
	/* -- */
extern c_str GLOGIK_DESKTOP_QT5_SESSION_DBUS_OBJECT;
extern c_str GLOGIK_DESKTOP_QT5_SESSION_DBUS_OBJECT_PATH;
extern c_str GLOGIK_DESKTOP_QT5_SESSION_DBUS_INTERFACE;

/* --- ---- --- */

extern c_str XF86_AUDIO_NEXT;
extern c_str XF86_AUDIO_PREV;
extern c_str XF86_AUDIO_STOP;
extern c_str XF86_AUDIO_PLAY;
extern c_str XF86_AUDIO_MUTE;
extern c_str XF86_AUDIO_RAISE_VOLUME;
extern c_str XF86_AUDIO_LOWER_VOLUME;

} // namespace GLogiK

#endif
