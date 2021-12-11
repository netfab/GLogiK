
/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
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

#include "glogik.hpp"

namespace GLogiK
{

c_str CONST_STRING_CLIENT				= "client : ";
c_str CONST_STRING_DEVICE				= "device : ";
c_str CONST_STRING_UNKNOWN_CLIENT		= "unknown client : ";
c_str CONST_STRING_UNKNOWN_DEVICE		= "unknown device : ";
c_str CONST_STRING_METHOD_CALL_FAILURE	= " method call failure : ";
c_str CONST_STRING_METHOD_REPLY_FAILURE	= " method reply failure : ";

c_str LCD_KEY_L1 = "L1";
c_str LCD_KEY_L2 = "L2";
c_str LCD_KEY_L3 = "L3";
c_str LCD_KEY_L4 = "L4";
c_str LCD_KEY_L5 = "L5";

/* --- ---- --- *
 * -- GKDBus -- *
 * --- ---- --- */
/* devices thread */
c_str GLOGIK_DEVICE_THREAD_DBUS_BUS_CONNECTION_NAME					= "com.glogik.Device";

/* daemon thread */
c_str GLOGIK_DAEMON_DBUS_ROOT_NODE									= "Daemon";
c_str GLOGIK_DAEMON_DBUS_ROOT_NODE_PATH								= "/com/glogik/Daemon";
c_str GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME						= "com.glogik.Daemon";
	/* -- */
c_str GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT						= "ClientsManager";
c_str GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH				= "/com/glogik/Daemon/ClientsManager";
c_str GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE					= "com.glogik.Daemon.Client1";
	/* -- */
c_str GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT						= "DevicesManager";
c_str GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH				= "/com/glogik/Daemon/DevicesManager";
c_str GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE					= "com.glogik.Daemon.Device1";


/* desktop service launcher */
c_str GLOGIK_DESKTOP_SERVICE_LAUNCHER_DBUS_ROOT_NODE				= "Launcher";
c_str GLOGIK_DESKTOP_SERVICE_LAUNCHER_DBUS_ROOT_NODE_PATH			= "/com/glogik/Launcher";
c_str GLOGIK_DESKTOP_SERVICE_LAUNCHER_DBUS_BUS_CONNECTION_NAME		= "com.glogik.Launcher";
	/* -- */
//c_str GLOGIK_DESKTOP_SERVICE_LAUNCHER_SESSION_DBUS_OBJECT			= "SessionMessageHandler";
//c_str GLOGIK_DESKTOP_SERVICE_LAUNCHER_SESSION_DBUS_OBJECT_PATH	= "/com/glogik/Launcher/SessionMessageHandler";
//c_str GLOGIK_DESKTOP_SERVICE_LAUNCHER_SESSION_DBUS_INTERFACE		= "com.glogik.Launcher.SessionMessageHandler1";


/* desktop service */
c_str GLOGIK_DESKTOP_SERVICE_DBUS_ROOT_NODE							= "Client";
c_str GLOGIK_DESKTOP_SERVICE_DBUS_ROOT_NODE_PATH					= "/com/glogik/Client";
c_str GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME				= "com.glogik.Client";
	/* -- */
//c_str GLOGIK_DESKTOP_SERVICE_SYSTEM_DBUS_OBJECT					= "SystemMessageHandler";
//c_str GLOGIK_DESKTOP_SERVICE_SYSTEM_DBUS_OBJECT_PATH				= "/com/glogik/Client/SystemMessageHandler";
//c_str GLOGIK_DESKTOP_SERVICE_SYSTEM_DBUS_INTERFACE				= "com.glogik.Client.SystemMessageHandler1";
	/* -- */
c_str GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT					= "SessionMessageHandler";
c_str GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH				= "/com/glogik/Client/SessionMessageHandler";
c_str GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE					= "com.glogik.Client.SessionMessageHandler1";

/* Qt5 gui */
c_str GLOGIK_DESKTOP_QT5_DBUS_ROOT_NODE								= "qt5gui";
c_str GLOGIK_DESKTOP_QT5_DBUS_ROOT_NODE_PATH						= "/com/glogik/qt5gui";
c_str GLOGIK_DESKTOP_QT5_DBUS_BUS_CONNECTION_NAME					= "com.glogik.qt5gui";
	/* -- */
c_str GLOGIK_DESKTOP_QT5_SESSION_DBUS_OBJECT						= "GUISessionMessageHandler";
c_str GLOGIK_DESKTOP_QT5_SESSION_DBUS_OBJECT_PATH					= "/com/glogik/qt5gui/GUISessionMessageHandler";
c_str GLOGIK_DESKTOP_QT5_SESSION_DBUS_INTERFACE						= "com.glogik.qt5gui.GUISessionMessageHandler";

c_str XF86_AUDIO_NEXT			= "XF86AudioNext";
c_str XF86_AUDIO_PREV			= "XF86AudioPrev";
c_str XF86_AUDIO_STOP			= "XF86AudioStop";
c_str XF86_AUDIO_PLAY			= "XF86AudioPlay";
c_str XF86_AUDIO_MUTE			= "XF86AudioMute";
c_str XF86_AUDIO_RAISE_VOLUME	= "XF86AudioRaiseVolume";
c_str XF86_AUDIO_LOWER_VOLUME	= "XF86AudioLowerVolume";

} // namespace GLogiK

