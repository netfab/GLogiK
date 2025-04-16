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

#ifndef SRC_LIB_SHARED_GLOGIK_HPP_
#define SRC_LIB_SHARED_GLOGIK_HPP_

#include <string>
#include <map>

#include "include/enums.hpp"
#include "include/base.hpp"
#include "include/DepsMap.hpp"

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

/* M Keys */
extern c_str M_KEY_M0;
extern c_str M_KEY_M1;
extern c_str M_KEY_M2;
extern c_str M_KEY_M3;

/* G Keys */
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

/* Media Keys */
extern c_str XF86_AUDIO_NEXT;
extern c_str XF86_AUDIO_PREV;
extern c_str XF86_AUDIO_STOP;
extern c_str XF86_AUDIO_PLAY;
extern c_str XF86_AUDIO_MUTE;
extern c_str XF86_AUDIO_RAISE_VOLUME;
extern c_str XF86_AUDIO_LOWER_VOLUME;

/* --- ---- --- */

extern c_str KEY_LIGHT;
extern c_str M_KEY_MR;
extern c_str MUTE_HEADSET;
extern c_str MUTE_MICRO;

/* --- ---- --- */

extern const std::map<Keys, c_str> keysNamesMap;
extern const std::map<GKeysID, c_str> GKeysNamesMap;
extern const std::map<Keys, MKeysID> keys2MKeysIDMap;
extern const std::map<Keys, GKeysID> keys2GKeysIDMap;

const std::string getKeyName(const Keys key);
const std::string getGKeyName(const GKeysID keyID);
const MKeysID getMKeyID(const Keys key);
const GKeysID getGKeyID(const Keys key);

void printVersionDeps(const std::string & binaryVersion, const GKDepsMap_type & dependencies);

/* --- ---- --- */

/* --- ---- --- *
 * -- GKDBus -- *
 * --- ---- --- */

/* daemon thread */
extern c_str GLOGIK_DAEMON_DBUS_ROOT_NODE_PATH;
extern c_str GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME;
	/* -- */
extern c_str GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH;
extern c_str GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE;
	/* -- */
extern c_str GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH;
extern c_str GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE;

/* desktop service launcher */
extern c_str GLOGIK_DESKTOP_SERVICE_LAUNCHER_DBUS_ROOT_NODE_PATH;
extern c_str GLOGIK_DESKTOP_SERVICE_LAUNCHER_DBUS_BUS_CONNECTION_NAME;
	/* -- */
//extern c_str GLOGIK_DESKTOP_SERVICE_LAUNCHER_SESSION_DBUS_OBJECT_PATH;
//extern c_str GLOGIK_DESKTOP_SERVICE_LAUNCHER_SESSION_DBUS_INTERFACE;

/* desktop service */
extern c_str GLOGIK_DESKTOP_SERVICE_DBUS_ROOT_NODE_PATH;
extern c_str GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME;
	/* -- */
//extern c_str GLOGIK_DESKTOP_SERVICE_SYSTEM_DBUS_OBJECT_PATH;
//extern c_str GLOGIK_DESKTOP_SERVICE_SYSTEM_DBUS_INTERFACE;
	/* -- */
extern c_str GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH;
extern c_str GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE;

/* Qt5 gui */
extern c_str GLOGIK_DESKTOP_QT5_DBUS_ROOT_NODE_PATH;
extern c_str GLOGIK_DESKTOP_QT5_DBUS_BUS_CONNECTION_NAME;
	/* -- */
extern c_str GLOGIK_DESKTOP_QT5_SESSION_DBUS_OBJECT_PATH;
extern c_str GLOGIK_DESKTOP_QT5_SESSION_DBUS_INTERFACE;

/* logind */
extern c_str LOGIND_DBUS_BUS_CONNECTION_NAME;

extern c_str LOGIND_MANAGER_DBUS_OBJECT_PATH;
extern c_str LOGIND_MANAGER_DBUS_INTERFACE;

extern c_str LOGIND_SESSION_DBUS_INTERFACE;

/* freedesktop standard interfaces */
extern c_str FREEDESKTOP_DBUS_PROPERTIES_STANDARD_INTERFACE;

enum class SessionFramework : uint8_t
{
	FW_UNKNOWN = 0,
	FW_LOGIND,
};


/* daemon ClientsManager D-Bus object methods */
extern c_str GK_DBUS_DAEMON_METHOD_REGISTER_CLIENT;
extern c_str GK_DBUS_DAEMON_METHOD_UNREGISTER_CLIENT;
extern c_str GK_DBUS_DAEMON_METHOD_UPDATE_CLIENT_STATE;
extern c_str GK_DBUS_DAEMON_METHOD_SET_CLIENT_READY;
extern c_str GK_DBUS_DAEMON_METHOD_DELETE_DEVICE_CONFIGURATION;
extern c_str GK_DBUS_DAEMON_METHOD_GET_DAEMON_DEPENDENCIES_MAP;

/* daemon DevicesManager D-Bus object methods */
extern c_str GK_DBUS_DAEMON_METHOD_STOP_DEVICE;
extern c_str GK_DBUS_DAEMON_METHOD_START_DEVICE;
extern c_str GK_DBUS_DAEMON_METHOD_RESTART_DEVICE;
extern c_str GK_DBUS_DAEMON_METHOD_GET_STARTED_DEVICES;
extern c_str GK_DBUS_DAEMON_METHOD_GET_STOPPED_DEVICES;
extern c_str GK_DBUS_DAEMON_METHOD_GET_DEVICE_STATUS;
extern c_str GK_DBUS_DAEMON_METHOD_GET_DEVICE_PROPERTIES;
extern c_str GK_DBUS_DAEMON_METHOD_GET_DEVICE_LCD_PLUGINS_PROPERTIES;
extern c_str GK_DBUS_DAEMON_METHOD_GET_DEVICE_GKEYSID_ARRAY;
extern c_str GK_DBUS_DAEMON_METHOD_GET_DEVICE_MKEYSID_ARRAY;
extern c_str GK_DBUS_DAEMON_METHOD_SET_DEVICE_BACKLIGHT_COLOR;
extern c_str GK_DBUS_DAEMON_METHOD_SET_DEVICE_LCD_PLUGINS_MASK;

/* service SessionMessageHandler D-Bus object methods */
extern c_str GK_DBUS_SERVICE_METHOD_GET_DEVICES_LIST;
extern c_str GK_DBUS_SERVICE_METHOD_GET_INFORMATIONS;
extern c_str GK_DBUS_SERVICE_METHOD_GET_EXECUTABLES_DEPENDENCIES_MAP;

/* D-Bus signals potentially sent to the desktop service */
extern c_str GK_DBUS_SERVICE_SIGNAL_DEAMON_IS_STOPPING;
extern c_str GK_DBUS_SERVICE_SIGNAL_DEAMON_IS_STARTING;
extern c_str GK_DBUS_SERVICE_SIGNAL_REPORT_YOURSELF;
extern c_str GK_DBUS_SERVICE_SIGNAL_DEVICES_STARTED;
extern c_str GK_DBUS_SERVICE_SIGNAL_DEVICES_STOPPED;
extern c_str GK_DBUS_SERVICE_SIGNAL_DEVICES_UNPLUGGED;
extern c_str GK_DBUS_SERVICE_SIGNAL_DEVICE_MBANK_SWITCH;
extern c_str GK_DBUS_SERVICE_SIGNAL_DEVICE_MACRO_RECORDED;
extern c_str GK_DBUS_SERVICE_SIGNAL_DEVICE_MACRO_CLEARED;
extern c_str GK_DBUS_SERVICE_SIGNAL_DEVICE_GKEY_EVENT;
extern c_str GK_DBUS_SERVICE_SIGNAL_DEVICE_MEDIA_EVENT;
extern c_str GK_DBUS_SERVICE_SIGNAL_DEVICE_STATUS_CHANGE_REQUEST;

extern c_str GK_DBUS_LAUNCHER_SIGNAL_SERVICE_START_REQUEST;

extern c_str GK_DBUS_GUI_SIGNAL_DEVICES_UPDATED;
extern c_str GK_DBUS_GUI_SIGNAL_DEVICE_CONFIGURATION_SAVED;

} // namespace GLogiK

#endif
