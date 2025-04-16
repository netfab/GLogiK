
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

#include <stdexcept>
#include <iomanip>

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

void printVersionDeps(const std::string & binaryVersion, const GKDepsMap_type & dependencies)
{
	using namespace NSGKUtils;

	std::ostringstream buffer(std::ios_base::app);
	buffer	<< binaryVersion << "\n"
			<< std::setfill('-') << std::setw(38) << "-" << "\n"
			<< std::setfill(' ') << std::setw(12) << "dependency"
			<< std::setw(2) << "|" << std::setw(14) << "built-against"
			<< std::setw(2) << "|" << std::setw(8) << "runtime" << "\n"
			<< std::setfill('-') << std::setw(38) << "-" << "\n"
			<< std::setfill(' ');

	for(const auto & x : dependencies) {
		for(const auto & v : x.second) {
			buffer
				<< std::setw(12) << v.getDependency()
				<< std::setw(2) << "|" << std::setw(14) << v.getCompileTimeVersion()
				<< std::setw(2) << "|" << std::setw(8) << v.getRunTimeVersion() << "\n";
		}
	}

	LOG(info) << buffer.str() << std::endl;
	//LOG(info) << dependencies[GKBinary::GK_DAEMON].size();
}

/* --- ---- --- *
 * -- GKDBus -- *
 * --- ---- --- */

/* daemon thread */
c_str GLOGIK_DAEMON_DBUS_ROOT_NODE_PATH								= "/com/glogik/Daemon";
c_str GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME						= "com.glogik.Daemon";
	/* -- */
c_str GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH				= "/com/glogik/Daemon/ClientsManager";
c_str GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE					= "com.glogik.Daemon.Client1";
	/* -- */
c_str GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH				= "/com/glogik/Daemon/DevicesManager";
c_str GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE					= "com.glogik.Daemon.Device1";


/* desktop service launcher */
c_str GLOGIK_DESKTOP_SERVICE_LAUNCHER_DBUS_ROOT_NODE_PATH			= "/com/glogik/Launcher";
c_str GLOGIK_DESKTOP_SERVICE_LAUNCHER_DBUS_BUS_CONNECTION_NAME		= "com.glogik.Launcher";
	/* -- */
//c_str GLOGIK_DESKTOP_SERVICE_LAUNCHER_SESSION_DBUS_OBJECT_PATH	= "/com/glogik/Launcher/SessionMessageHandler";
//c_str GLOGIK_DESKTOP_SERVICE_LAUNCHER_SESSION_DBUS_INTERFACE		= "com.glogik.Launcher.SessionMessageHandler1";


/* desktop service */
c_str GLOGIK_DESKTOP_SERVICE_DBUS_ROOT_NODE_PATH					= "/com/glogik/Client";
c_str GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME				= "com.glogik.Client";
	/* -- */
//c_str GLOGIK_DESKTOP_SERVICE_SYSTEM_DBUS_OBJECT_PATH				= "/com/glogik/Client/SystemMessageHandler";
//c_str GLOGIK_DESKTOP_SERVICE_SYSTEM_DBUS_INTERFACE				= "com.glogik.Client.SystemMessageHandler1";
	/* -- */
c_str GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH				= "/com/glogik/Client/SessionMessageHandler";
c_str GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE					= "com.glogik.Client.SessionMessageHandler1";

/* Qt5 gui */
c_str GLOGIK_DESKTOP_QT5_DBUS_ROOT_NODE_PATH						= "/com/glogik/qt5gui";
c_str GLOGIK_DESKTOP_QT5_DBUS_BUS_CONNECTION_NAME					= "com.glogik.qt5gui";
	/* -- */
c_str GLOGIK_DESKTOP_QT5_SESSION_DBUS_OBJECT_PATH					= "/com/glogik/qt5gui/GUISessionMessageHandler";
c_str GLOGIK_DESKTOP_QT5_SESSION_DBUS_INTERFACE						= "com.glogik.qt5gui.GUISessionMessageHandler";

/* systemd-logind */
c_str LOGIND_DBUS_BUS_CONNECTION_NAME								= "org.freedesktop.login1";
	/* -- */
c_str LOGIND_MANAGER_DBUS_OBJECT_PATH								= "/org/freedesktop/login1";
c_str LOGIND_MANAGER_DBUS_INTERFACE									= "org.freedesktop.login1.Manager";

c_str LOGIND_SESSION_DBUS_INTERFACE									= "org.freedesktop.login1.Session";

/* freedesktop standard interfaces */
c_str FREEDESKTOP_DBUS_PROPERTIES_STANDARD_INTERFACE				= "org.freedesktop.DBus.Properties";


/* daemon ClientsManager D-Bus object methods */
c_str GK_DBUS_DAEMON_METHOD_REGISTER_CLIENT					= "RegisterClient";
c_str GK_DBUS_DAEMON_METHOD_UNREGISTER_CLIENT				= "UnregisterClient";
c_str GK_DBUS_DAEMON_METHOD_UPDATE_CLIENT_STATE				= "UpdateClientState";
c_str GK_DBUS_DAEMON_METHOD_SET_CLIENT_READY				= "SetClientReady";
c_str GK_DBUS_DAEMON_METHOD_DELETE_DEVICE_CONFIGURATION		= "DeleteDeviceConfiguration";
c_str GK_DBUS_DAEMON_METHOD_GET_DAEMON_DEPENDENCIES_MAP		= "GetDaemonDependenciesMap";

/* daemon DevicesManager D-Bus object methods */
c_str GK_DBUS_DAEMON_METHOD_STOP_DEVICE							= "StopDevice";
c_str GK_DBUS_DAEMON_METHOD_START_DEVICE						= "StartDevice";
c_str GK_DBUS_DAEMON_METHOD_RESTART_DEVICE						= "RestartDevice";
c_str GK_DBUS_DAEMON_METHOD_GET_STARTED_DEVICES					= "GetStartedDevices";
c_str GK_DBUS_DAEMON_METHOD_GET_STOPPED_DEVICES					= "GetStoppedDevices";
c_str GK_DBUS_DAEMON_METHOD_GET_DEVICE_STATUS					= "GetDeviceStatus";
c_str GK_DBUS_DAEMON_METHOD_GET_DEVICE_PROPERTIES				= "GetDeviceProperties";
c_str GK_DBUS_DAEMON_METHOD_GET_DEVICE_LCD_PLUGINS_PROPERTIES	= "GetDeviceLCDPluginsProperties";
c_str GK_DBUS_DAEMON_METHOD_GET_DEVICE_GKEYSID_ARRAY			= "GetDeviceGKeysIDArray";
c_str GK_DBUS_DAEMON_METHOD_GET_DEVICE_MKEYSID_ARRAY			= "GetDeviceMKeysIDArray";
c_str GK_DBUS_DAEMON_METHOD_SET_DEVICE_BACKLIGHT_COLOR			= "SetDeviceBacklightColor";
c_str GK_DBUS_DAEMON_METHOD_SET_DEVICE_LCD_PLUGINS_MASK			= "SetDeviceLCDPluginsMask";

/* service SessionMessageHandler D-Bus object methods */
c_str GK_DBUS_SERVICE_METHOD_GET_DEVICES_LIST					= "GetDevicesList";
c_str GK_DBUS_SERVICE_METHOD_GET_INFORMATIONS					= "GetInformations";
c_str GK_DBUS_SERVICE_METHOD_GET_EXECUTABLES_DEPENDENCIES_MAP	= "GetExecutablesDependenciesMap";

/* D-Bus signals potentially sent to the desktop service */
c_str GK_DBUS_SERVICE_SIGNAL_DEAMON_IS_STOPPING					= "DaemonIsStopping";
c_str GK_DBUS_SERVICE_SIGNAL_DEAMON_IS_STARTING					= "DaemonIsStarting";
c_str GK_DBUS_SERVICE_SIGNAL_REPORT_YOURSELF					= "ReportYourself";
c_str GK_DBUS_SERVICE_SIGNAL_DEVICES_STARTED					= "DevicesStarted";
c_str GK_DBUS_SERVICE_SIGNAL_DEVICES_STOPPED					= "DevicesStopped";
c_str GK_DBUS_SERVICE_SIGNAL_DEVICES_UNPLUGGED					= "DevicesUnplugged";
c_str GK_DBUS_SERVICE_SIGNAL_DEVICE_MBANK_SWITCH				= "DeviceMBankSwitch";
c_str GK_DBUS_SERVICE_SIGNAL_DEVICE_MACRO_RECORDED				= "DeviceMacroRecorded";
c_str GK_DBUS_SERVICE_SIGNAL_DEVICE_MACRO_CLEARED				= "DeviceMacroCleared";
c_str GK_DBUS_SERVICE_SIGNAL_DEVICE_GKEY_EVENT					= "DeviceGKeyEvent";
c_str GK_DBUS_SERVICE_SIGNAL_DEVICE_MEDIA_EVENT					= "DeviceMediaEvent";
c_str GK_DBUS_SERVICE_SIGNAL_DEVICE_STATUS_CHANGE_REQUEST		= "DeviceStatusChangeRequest";

c_str GK_DBUS_LAUNCHER_SIGNAL_SERVICE_START_REQUEST				= "ServiceStartRequest";

c_str GK_DBUS_GUI_SIGNAL_DEVICES_UPDATED						= "DevicesUpdated";
c_str GK_DBUS_GUI_SIGNAL_DEVICE_CONFIGURATION_SAVED				= "DeviceConfigurationSaved";

} // namespace GLogiK
