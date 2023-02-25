
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

#include <stdexcept>

#include "lib/utils/utils.hpp"

#include "glogik.hpp"

namespace GLogiK
{

c_str CONST_STRING_CLIENT				= "client : ";
c_str CONST_STRING_DEVICE				= "device : ";
c_str CONST_STRING_UNKNOWN_CLIENT		= "unknown client : ";
c_str CONST_STRING_UNKNOWN_DEVICE		= "unknown device : ";
c_str CONST_STRING_METHOD_CALL_FAILURE	= " method call failure : ";
c_str CONST_STRING_METHOD_REPLY_FAILURE	= " method reply failure : ";

/* M Keys */
c_str M_KEY_M0	= "M0"; // virtual key
c_str M_KEY_M1	= "M1";
c_str M_KEY_M2	= "M2";
c_str M_KEY_M3	= "M3";

/* G Keys */
c_str G_KEY_G1	= "G1";
c_str G_KEY_G2	= "G2";
c_str G_KEY_G3	= "G3";
c_str G_KEY_G4	= "G4";
c_str G_KEY_G5	= "G5";
c_str G_KEY_G6	= "G6";
c_str G_KEY_G7	= "G7";
c_str G_KEY_G8	= "G8";
c_str G_KEY_G9	= "G9";
c_str G_KEY_G10 = "G10";
c_str G_KEY_G11 = "G11";
c_str G_KEY_G12 = "G12";
c_str G_KEY_G13 = "G13";
c_str G_KEY_G14 = "G14";
c_str G_KEY_G15 = "G15";
c_str G_KEY_G16 = "G16";
c_str G_KEY_G17 = "G17";
c_str G_KEY_G18 = "G18";

/* LCD Keys */
c_str LCD_KEY_L1 = "L1";
c_str LCD_KEY_L2 = "L2";
c_str LCD_KEY_L3 = "L3";
c_str LCD_KEY_L4 = "L4";
c_str LCD_KEY_L5 = "L5";

/* Media Keys */
c_str XF86_AUDIO_NEXT			= "XF86AudioNext";
c_str XF86_AUDIO_PREV			= "XF86AudioPrev";
c_str XF86_AUDIO_STOP			= "XF86AudioStop";
c_str XF86_AUDIO_PLAY			= "XF86AudioPlay";
c_str XF86_AUDIO_MUTE			= "XF86AudioMute";
c_str XF86_AUDIO_RAISE_VOLUME	= "XF86AudioRaiseVolume";
c_str XF86_AUDIO_LOWER_VOLUME	= "XF86AudioLowerVolume";

/* --- ---- --- */

c_str KEY_LIGHT		= "";
c_str M_KEY_MR		= "";
c_str MUTE_HEADSET	= "";
c_str MUTE_MICRO	= "";

/* --- ---- --- */

const std::map<Keys, c_str> keysNamesMap = {
	{ Keys::GK_KEY_M1, M_KEY_M1 },
	{ Keys::GK_KEY_M2, M_KEY_M2 },
	{ Keys::GK_KEY_M3, M_KEY_M3 },
	/* -- */
	{ Keys::GK_KEY_G1, G_KEY_G1 },
	{ Keys::GK_KEY_G2, G_KEY_G2 },
	{ Keys::GK_KEY_G3, G_KEY_G3 },
	{ Keys::GK_KEY_G4, G_KEY_G4 },
	{ Keys::GK_KEY_G5, G_KEY_G5 },
	{ Keys::GK_KEY_G6, G_KEY_G6 },
	{ Keys::GK_KEY_G7, G_KEY_G7 },
	{ Keys::GK_KEY_G8, G_KEY_G8 },
	{ Keys::GK_KEY_G9, G_KEY_G9 },
	{ Keys::GK_KEY_G10, G_KEY_G10 },
	{ Keys::GK_KEY_G11, G_KEY_G11 },
	{ Keys::GK_KEY_G12, G_KEY_G12 },
	{ Keys::GK_KEY_G13, G_KEY_G13 },
	{ Keys::GK_KEY_G14, G_KEY_G14 },
	{ Keys::GK_KEY_G15, G_KEY_G15 },
	{ Keys::GK_KEY_G16, G_KEY_G16 },
	{ Keys::GK_KEY_G17, G_KEY_G17 },
	{ Keys::GK_KEY_G18, G_KEY_G18 },
	/* -- */
	{ Keys::GK_KEY_L1, LCD_KEY_L1 },
	{ Keys::GK_KEY_L2, LCD_KEY_L2 },
	{ Keys::GK_KEY_L3, LCD_KEY_L3 },
	{ Keys::GK_KEY_L4, LCD_KEY_L4 },
	{ Keys::GK_KEY_L5, LCD_KEY_L5 },
	/* -- */
	{ Keys::GK_KEY_AUDIO_NEXT, 			XF86_AUDIO_NEXT },
	{ Keys::GK_KEY_AUDIO_PREV, 			XF86_AUDIO_PREV },
	{ Keys::GK_KEY_AUDIO_STOP,			XF86_AUDIO_STOP },
	{ Keys::GK_KEY_AUDIO_PLAY,			XF86_AUDIO_PLAY },
	{ Keys::GK_KEY_AUDIO_MUTE,			XF86_AUDIO_MUTE },
	{ Keys::GK_KEY_AUDIO_RAISE_VOLUME,	XF86_AUDIO_RAISE_VOLUME },
	{ Keys::GK_KEY_AUDIO_LOWER_VOLUME,	XF86_AUDIO_LOWER_VOLUME },
	/* -- */
	{ Keys::GK_KEY_LIGHT, 		KEY_LIGHT },
	{ Keys::GK_KEY_MR, 			M_KEY_MR },
	{ Keys::GK_KEY_MUTE_HEADSET, 	MUTE_HEADSET },
	{ Keys::GK_KEY_MUTE_MICRO, 	MUTE_MICRO },
};

const std::map<GKeysID, c_str> GKeysNamesMap = {
	{ GKeysID::GKEY_G1, G_KEY_G1 },
	{ GKeysID::GKEY_G2, G_KEY_G2 },
	{ GKeysID::GKEY_G3, G_KEY_G3 },
	{ GKeysID::GKEY_G4, G_KEY_G4 },
	{ GKeysID::GKEY_G5, G_KEY_G5 },
	{ GKeysID::GKEY_G6, G_KEY_G6 },
	{ GKeysID::GKEY_G7, G_KEY_G7 },
	{ GKeysID::GKEY_G8, G_KEY_G8 },
	{ GKeysID::GKEY_G9, G_KEY_G9 },
	{ GKeysID::GKEY_G10, G_KEY_G10 },
	{ GKeysID::GKEY_G11, G_KEY_G11 },
	{ GKeysID::GKEY_G12, G_KEY_G12 },
	{ GKeysID::GKEY_G13, G_KEY_G13 },
	{ GKeysID::GKEY_G14, G_KEY_G14 },
	{ GKeysID::GKEY_G15, G_KEY_G15 },
	{ GKeysID::GKEY_G16, G_KEY_G16 },
	{ GKeysID::GKEY_G17, G_KEY_G17 },
	{ GKeysID::GKEY_G18, G_KEY_G18 },
};

const std::map<Keys, MKeysID> keys2MKeysIDMap = {
	{ Keys::GK_KEY_M1, MKeysID::MKEY_M1 },
	{ Keys::GK_KEY_M2, MKeysID::MKEY_M2 },
	{ Keys::GK_KEY_M3, MKeysID::MKEY_M3 },
};

const std::map<Keys, GKeysID> keys2GKeysIDMap = {
	{ Keys::GK_KEY_G1, GKeysID::GKEY_G1 },
	{ Keys::GK_KEY_G2, GKeysID::GKEY_G2 },
	{ Keys::GK_KEY_G3, GKeysID::GKEY_G3 },
	{ Keys::GK_KEY_G4, GKeysID::GKEY_G4 },
	{ Keys::GK_KEY_G5, GKeysID::GKEY_G5 },
	{ Keys::GK_KEY_G6, GKeysID::GKEY_G6 },
	{ Keys::GK_KEY_G7, GKeysID::GKEY_G7 },
	{ Keys::GK_KEY_G8, GKeysID::GKEY_G8 },
	{ Keys::GK_KEY_G9, GKeysID::GKEY_G9 },
	{ Keys::GK_KEY_G10, GKeysID::GKEY_G10 },
	{ Keys::GK_KEY_G11, GKeysID::GKEY_G11 },
	{ Keys::GK_KEY_G12, GKeysID::GKEY_G12 },
	{ Keys::GK_KEY_G13, GKeysID::GKEY_G13 },
	{ Keys::GK_KEY_G14, GKeysID::GKEY_G14 },
	{ Keys::GK_KEY_G15, GKeysID::GKEY_G15 },
	{ Keys::GK_KEY_G16, GKeysID::GKEY_G16 },
	{ Keys::GK_KEY_G17, GKeysID::GKEY_G17 },
	{ Keys::GK_KEY_G18, GKeysID::GKEY_G18 },
};

const std::string getKeyName(const Keys key)
{
	using namespace NSGKUtils;

	std::string ret("");
	try {
		ret = keysNamesMap.at(key);
	}
	catch (const std::out_of_range& oor) {
		LOG(error) << "invalid key: " << toEnumType(key);
	}

	return ret;
}

const std::string getGKeyName(const GKeysID keyID)
{
	using namespace NSGKUtils;

	std::string GKey("G0");
	try {
		GKey = GKeysNamesMap.at(keyID);
	}
	catch (const std::out_of_range& oor) {
		LOG(error) << "invalid GKeysID: " << toEnumType(keyID);
	}
	return GKey;
}

const MKeysID getMKeyID(const Keys key)
{
	return keys2MKeysIDMap.at(key);
}

const GKeysID getGKeyID(const Keys key)
{
	return keys2GKeysIDMap.at(key);
}

/* --- ---- --- *
 * -- GKDBus -- *
 * --- ---- --- */

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

} // namespace GLogiK

