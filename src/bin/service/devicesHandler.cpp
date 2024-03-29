/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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

#include <utility>
#include <exception>
#include <stdexcept>
#include <new>

#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

#include "lib/shared/deviceConfigurationFile.hpp"

#include <config.h>

#if HAVE_DESKTOP_NOTIFICATIONS
#include "desktopNotification.hpp"
#endif

#include "devicesHandler.hpp"

#include "include/DeviceID.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

DevicesHandler::DevicesHandler()
	:	_clientID("undefined"),
		_pGKfs(nullptr)
{
	GK_LOG_FUNC

	GKLog(trace, "Devices Handler initialization")

	_configurationRootDirectory = XDGUserDirs::getConfigurationRootDirectory();
	_configurationRootDirectory /= PACKAGE_NAME;

	FileSystem::createDirectory(_configurationRootDirectory, fs::owner_all);
#if DEBUGGING_ON
	FileSystem::traceLastDirectoryCreation();
#endif
}

DevicesHandler::~DevicesHandler()
{
	GK_LOG_FUNC

	GKLog(trace, "Devices Handler destruction")
}

void DevicesHandler::setGKfs(NSGKUtils::FileSystem* pGKfs)
{
	_pGKfs = pGKfs;
}

void DevicesHandler::setClientID(const std::string & id)
{
	_clientID = id;
}

void DevicesHandler::clearDevices(const bool notifications)
{
	devIDSet devicesID;

	for( const auto & devicePair : _startedDevices ) {
		devicesID.insert(devicePair.first);
	}
	for( const auto & devicePair : _stoppedDevices ) {
		devicesID.insert(devicePair.first);
	}

	/* stop each device if not already stopped, and unref it */
	for(const auto & devID : devicesID) {
		this->unplugDevice(devID, notifications);
	}

	/* clear all containers */
	_startedDevices.clear();
	_stoppedDevices.clear();
}

const DevicesFilesMap_type DevicesHandler::getDevicesFilesMap(void)
{
	DevicesFilesMap_type ret;
	for(const auto & dev : _startedDevices) {
		ret.insert( std::pair<std::string, const std::string>(dev.first, dev.second.getConfigFilePath()) );
	}
	for(const auto & dev : _stoppedDevices) {
		ret.insert( std::pair<std::string, const std::string>(dev.first, dev.second.getConfigFilePath()) );
	}
	return ret;
}

const std::vector<std::string> DevicesHandler::getDevicesList(void)
{
	std::vector<std::string> ret;

	try {
		using Size = std::vector<std::string>::size_type;
		/* assuming that we don't have millions of devices connected */
		const Size num( _startedDevices.size() + _stoppedDevices.size() );

		ret.reserve( num * (DEVICE_ID_NUM_PROPERTIES+1) );
	}
	catch( const std::length_error & e ) {
		GKSysLogError("reserve length_error failure : ", e.what());
	}
	catch( const std::bad_alloc & e ) {
		GKSysLogError("reserve bad_alloc failure : ", e.what());
	}

	for(const auto & dev : _startedDevices)
	{
		const auto & devID = dev.first;
		const auto & device = dev.second;

		ret.push_back(devID);
		ret.push_back("started"); // status
		ret.push_back(device.getVendor());
		ret.push_back(device.getProduct());
		ret.push_back(device.getName());
		ret.push_back(device.getConfigFilePath());
	}
	for(const auto & dev : _stoppedDevices)
	{
		const auto & devID = dev.first;
		const auto & device = dev.second;

		ret.push_back(devID);
		ret.push_back("stopped"); // status
		ret.push_back(device.getVendor());
		ret.push_back(device.getProduct());
		ret.push_back(device.getName());
		ret.push_back(device.getConfigFilePath());
	}

	return ret;
}

const bool DevicesHandler::checkDeviceCapability(const DeviceProperties & device, Caps toCheck)
{
	return (device.getCapabilities() & toEnumType(toCheck));
}

void DevicesHandler::reloadDeviceConfigurationFile(const std::string & devID)
{
	GK_LOG_FUNC

	{
		devIDSet::const_iterator it = _ignoredFSNotifications.find(devID);
		/* iterator to device ID was found */
		if( it != _ignoredFSNotifications.cend() ) {
			GKLog2(trace, devID, " ignoring filesystem notification after configuration file save")
			_ignoredFSNotifications.erase(it);
			return;
		}
	}

	try {
		DeviceProperties & device = _startedDevices.at(devID);
		this->loadDeviceConfigurationFile(device);

		this->sendDeviceConfigurationToDaemon(devID, device);

		/* inform GUI that configuration file was reloaded */
		this->sendDeviceConfigurationSavedSignal(devID);
	}
	catch (const std::out_of_range& oor) {
		try {
			DeviceProperties & device = _stoppedDevices.at(devID);
			this->loadDeviceConfigurationFile(device);

			this->sendDeviceConfigurationToDaemon(devID, device);

			/* inform GUI that configuration file was reloaded */
			this->sendDeviceConfigurationSavedSignal(devID);
		}
		catch (const std::out_of_range& oor) {
			LOG(warning) << devID << " device not found in containers, giving up";
		}
	}
}

void DevicesHandler::saveDeviceConfigurationFile(const std::string & devID)
{
	GK_LOG_FUNC

	try {
		DeviceProperties & device = _startedDevices.at(devID);

		this->initializeConfigurationDirectory(device);
		this->saveDeviceConfigurationFile(devID, device);
	}
	catch (const std::out_of_range& oor) {
		LOG(warning) << devID << " device not found in started-devices container";
	}
}

void DevicesHandler::saveDeviceConfigurationFile(
	const std::string & devID,
	const DeviceProperties & device)
{
	GK_LOG_FUNC

	GKLog2(trace, devID, " saving device configuration file")

	auto logError = [&devID] (const std::string & msg, const std::string & what) -> void {
		LOG(error)	<< devID
					<< " saving device configuration file failed : "
					<< msg
					<< " : "
					<< what;
	};

	try {
		fs::path filePath(_configurationRootDirectory);
		filePath /= device.getVendor();
		filePath /= device.getConfigFilePath();

		/* ignore next filesystem notification for this device, this will
		 * abort next reloadDeviceConfigurationFile call for this devID
		 */
		typedef std::pair<devIDSet::iterator, bool> ignoredInsRet;
		ignoredInsRet ret = _ignoredFSNotifications.insert(devID);
		if( ! ret.second ) {
			GKLog2(warning, devID, "device ID already exists in container")
		}

		DeviceConfigurationFile::save(filePath.string(), device);

		try {
			fs::permissions(filePath, fs::owner_read|fs::owner_write|fs::group_read|fs::others_read);
		}
		catch (const fs::filesystem_error & e) {
			logError("set permissions failure", e.what());
		}
		catch (const std::exception & e) {
			logError("set permissions (allocation) failure", e.what());
		}

		this->sendDeviceConfigurationSavedSignal(devID);
	}
	catch ( const GLogiKExcept & e ) {
		logError("catched exception", e.what());
	}
}

void DevicesHandler::sendDeviceConfigurationSavedSignal(const std::string & devID)
{
	GK_LOG_FUNC

	try {
		/* send DeviceConfigurationSaved signal to GUI applications */
		DBus.initializeBroadcastSignal(
			_sessionBus,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
			"DeviceConfigurationSaved"
		);
		DBus.appendStringToBroadcastSignal(devID);
		DBus.sendBroadcastSignal();

		GKLog2(trace, devID, " sent signal on session bus : DeviceConfigurationSaved")
	}
	catch (const GKDBusMessageWrongBuild & e) {
		DBus.abandonBroadcastSignal();
		LOG(error) << devID << " failed to send signal on session bus : " << e.what();
	}
}

void DevicesHandler::loadDeviceConfigurationFile(DeviceProperties & device)
{
	fs::path filePath(_configurationRootDirectory);
	filePath /= device.getVendor();
	filePath /= device.getConfigFilePath();

	DeviceConfigurationFile::load(filePath.string(), device);
}

void DevicesHandler::sendDeviceConfigurationToDaemon(
	const std::string & devID,
	const DeviceProperties & device)
{
	GK_LOG_FUNC

	if( this->checkDeviceCapability(device, Caps::GK_BACKLIGHT_COLOR) ) {
		/* set backlight color */
		const std::string remoteMethod("SetDeviceBacklightColor");

		try {
			DBus.initializeRemoteMethodCall(
				_systemBus,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			DBus.appendStringToRemoteMethodCall(_clientID);
			DBus.appendStringToRemoteMethodCall(devID);
			uint8_t r, g, b = 0; device.getRGBBytes(r, g, b);
			DBus.appendUInt8ToRemoteMethodCall(r);
			DBus.appendUInt8ToRemoteMethodCall(g);
			DBus.appendUInt8ToRemoteMethodCall(b);

			DBus.sendRemoteMethodCall();

			try {
				DBus.waitForRemoteMethodCallReply();

				const bool ret = DBus.getNextBooleanArgument();
				if( ! ret ) {
					LOG(error) << devID << " failed to set device backlight color : false";
				}
				else {
					GKLog3(trace, devID, " successfully setted device backlight color : ", getHexRGB(r, g, b))
				}
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			DBus.abandonRemoteMethodCall();
			LogRemoteCallFailure
		}
	}

	// FIXME
	//if( this->checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
	//}

	if( this->checkDeviceCapability(device, Caps::GK_LCD_SCREEN) ) {
		const std::string remoteMethod = "SetDeviceLCDPluginsMask";

		try {
			const uint8_t maskID = toEnumType(LCDPluginsMask::GK_LCD_PLUGINS_MASK_1);
			const uint64_t mask = device.getLCDPluginsMask1();

			DBus.initializeRemoteMethodCall(
				_systemBus,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			DBus.appendStringToRemoteMethodCall(_clientID);
			DBus.appendStringToRemoteMethodCall(devID);
			DBus.appendUInt8ToRemoteMethodCall(maskID);
			DBus.appendUInt64ToRemoteMethodCall(mask);

			DBus.sendRemoteMethodCall();

			try {
				DBus.waitForRemoteMethodCallReply();

				const bool ret = DBus.getNextBooleanArgument();
				if( ! ret ) {
					LOG(error) << devID << " failed to set device LCD Plugins Mask " << toUInt(maskID) << " : false";
				}
				else {
					GKLog3(trace, devID, " successfully setted device LCD Plugins Mask : ", toUInt(maskID))
				}
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			DBus.abandonRemoteMethodCall();
			LogRemoteCallFailure
		}
	}

	LOG(info) << devID << " sent device configuration to daemon";
}

void DevicesHandler::setDeviceProperties(
	const std::string & devID,
	DeviceProperties & device)
{
	GK_LOG_FUNC

	/* initialize device properties */
	std::string remoteMethod("GetDeviceProperties");

	try {
		DBus.initializeRemoteMethodCall(
			_systemBus,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		DBus.appendStringToRemoteMethodCall(_clientID);
		DBus.appendStringToRemoteMethodCall(devID);

		DBus.sendRemoteMethodCall();

		try {
			DBus.waitForRemoteMethodCallReply();

			const std::string vendor( DBus.getNextStringArgument() );
			const std::string product( DBus.getNextStringArgument() );
			const std::string name( DBus.getNextStringArgument() );
			const uint64_t caps( DBus.getNextUInt64Argument() );
			device.setProperties( vendor, product, name, caps );

			GKLog2(trace, devID, " got 4 properties")
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		DBus.abandonRemoteMethodCall();
		LogRemoteCallFailure
	}

	if( this->checkDeviceCapability(device, Caps::GK_LCD_SCREEN) ) {
		/* get LCD plugins properties */
		remoteMethod = "GetDeviceLCDPluginsProperties";

		try {
			DBus.initializeRemoteMethodCall(
				_systemBus,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			DBus.appendStringToRemoteMethodCall(_clientID);
			DBus.appendStringToRemoteMethodCall(devID);

			DBus.sendRemoteMethodCall();

			try {
				DBus.waitForRemoteMethodCallReply();

				const LCDPPArray_type array = DBus.getNextLCDPPArrayArgument();
				device.setLCDPluginsProperties(array);

				GKLog3(trace, devID, " number of LCDPluginsProperties objects : ", array.size())
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			DBus.abandonRemoteMethodCall();
			LogRemoteCallFailure
		}
	}

	if( this->checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
		/* initialize macro keys banks */
		const MKeysIDArray_type MKeysIDArray = this->getDeviceMKeysIDArray(devID);
		if( ! MKeysIDArray.empty() ) {
			const GKeysIDArray_type GKeysIDArray = this->getDeviceGKeysIDArray(devID);
			device.initBanks(MKeysIDArray, GKeysIDArray);
		}
	}

	/* search a configuration file */
	GKLog2(trace, devID, " assigning configuration file")

	fs::path directory(_configurationRootDirectory);
	directory /= device.getVendor();

	std::set<std::string> alreadyUsed;
	{
		for(const auto & dev : _startedDevices) {
			alreadyUsed.insert( dev.second.getConfigFilePath() );
		}
		for(const auto & dev : _stoppedDevices) {
			alreadyUsed.insert( dev.second.getConfigFilePath() );
		}
	}

	try {
		try {
			std::string baseName(device.getProduct());
			baseName += " ";
			baseName += device.getName();

			/* searching for an existing configuration file */
			device.setConfigFilePath(
				_pGKfs->getNextAvailableFileName(alreadyUsed, directory, baseName, "cfg", true)
			);
		}
		catch ( const GLogiKExcept & e ) {
			GKLog(trace, e.what())
			throw GLogiKExcept("configuration file not found");
		}

		GKLog3(trace, devID, " found file : ", device.getConfigFilePath())

		this->initializeConfigurationDirectory(device, false);
		this->loadDeviceConfigurationFile(device);

		LOG(info)	<< "found device " << devID << " - "
					<< device.getVendor() << " "
					<< device.getProduct() << " "
					<< device.getName();
		LOG(info)	<< devID << " configuration file found and loaded";

		this->sendDeviceConfigurationToDaemon(devID, device);
	}
	catch ( const GLogiKExcept & e ) { /* should happen only when configuration file not found */
		GKLog(trace, e.what())

		LOG(info)	<< devID << " started device : "
					<< device.getVendor() << " "
					<< device.getProduct() << " "
					<< device.getName();
		LOG(info)	<< devID << " creating default configuration file";

		try {
			std::string baseName(device.getProduct());
			baseName += " ";
			baseName += device.getName();

			/* none found, assign a new configuration file to this device */
			device.setConfigFilePath(
				_pGKfs->getNextAvailableFileName(alreadyUsed, directory, baseName, "cfg")
			);

			GKLog3(trace, devID, " new file : ", device.getConfigFilePath())

			this->initializeConfigurationDirectory(device, false);
			this->saveDeviceConfigurationFile(devID, device);
		}
		catch ( const GLogiKExcept & e ) {
			LOG(error) << devID << " failed to create default configuration file : " << e.what();
		}
	}
}

void DevicesHandler::startDevice(const std::string & devID, const bool notifications)
{
	GK_LOG_FUNC

	try {
		_startedDevices.at(devID);

		GKLog2(trace, devID, " device already started")
	}
	catch (const std::out_of_range& oor) {
		try {
			DeviceProperties & device = _stoppedDevices.at(devID);
			LOG(info) << devID << " starting device";
			_startedDevices[devID] = device;
			_stoppedDevices.erase(devID);

#if HAVE_DESKTOP_NOTIFICATIONS
			if(notifications)
				this->showNotification(devID, "Device started", device);
#endif
		}
		catch (const std::out_of_range& oor) {
			LOG(info) << devID << " initializing and starting device";
			DeviceProperties device;
			/* also load configuration file */
			this->setDeviceProperties(devID, device);
			_startedDevices[devID] = device;

#if HAVE_DESKTOP_NOTIFICATIONS
			if(notifications)
				this->showNotification(devID, "Device started", device);
#endif
		}
	}
}

void DevicesHandler::stopDevice(const std::string & devID, const bool notifications)
{
	GK_LOG_FUNC

	try {
		_stoppedDevices.at(devID);

		GKLog2(trace, devID, " device already stopped")
	}
	catch (const std::out_of_range& oor) {
		try {
			DeviceProperties & device = _startedDevices.at(devID);
			LOG(info) << devID << " stopping device";
			_stoppedDevices[devID] = device;
			_startedDevices.erase(devID);

#if HAVE_DESKTOP_NOTIFICATIONS
			if(notifications)
				this->showNotification(devID, "Device stopped", device);
#endif
		}
		catch (const std::out_of_range& oor) {
			/* this can happen when GLogiKs start and this device is already stopped
			 * so we must start the device to stop it */
			GKLog2(trace, devID, " device not found in containers, initializing and stopping it")

			this->startDevice(devID, false);
			try {
				DeviceProperties & device = _startedDevices.at(devID);
				LOG(info) << devID << " stopping device";
				_stoppedDevices[devID] = device;
				_startedDevices.erase(devID);

#if HAVE_DESKTOP_NOTIFICATIONS
				if(notifications)
					this->showNotification(devID, "Device stopped", device);
#endif
			}
			catch (const std::out_of_range& oor) {
				LOG(error) << devID << " started device not found, something is really wrong";
			}
		}
	}
}

void DevicesHandler::unplugDevice(const std::string & devID, const bool notifications)
{
	GK_LOG_FUNC

	try {
		_stoppedDevices.at(devID);

		GKLog2(trace, devID, " device already stopped")

		this->unrefDevice(devID);
	}
	catch (const std::out_of_range& oor) {
		this->stopDevice(devID, notifications);
		this->unrefDevice(devID);
	}
}

void DevicesHandler::unrefDevice(const std::string & devID)
{
	GK_LOG_FUNC

	try {
		const DeviceProperties & device = _stoppedDevices.at(devID);

		_pGKfs->removeNotifyWatch( device.getWatchDescriptor() );

		_stoppedDevices.erase(devID);

		GKLog2(trace, devID, " device erased")

		std::string remoteMethod("DeleteDeviceConfiguration");

		try {
			DBus.initializeRemoteMethodCall(
				_systemBus,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			DBus.appendStringToRemoteMethodCall(_clientID);
			DBus.appendStringToRemoteMethodCall(devID);

			DBus.sendRemoteMethodCall();

			try {
				DBus.waitForRemoteMethodCallReply();
				const bool ret = DBus.getNextBooleanArgument();
				if( ! ret ) {
					LOG(error) << devID << " failed to delete remote device configuration : false";
				}
				else {
					GKLog2(trace, devID, " successfully deleted remote device configuration")
				}
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			DBus.abandonRemoteMethodCall();
			LogRemoteCallFailure
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(warning) << devID << " device not found in stopped-devices container";
	}
}

void DevicesHandler::initializeConfigurationDirectory(DeviceProperties & device, const bool check)
{
	GK_LOG_FUNC

	fs::path directory(_configurationRootDirectory);
	directory /= device.getVendor();

	try {
		FileSystem::createDirectory(directory, fs::owner_all);
#if DEBUGGING_ON
		FileSystem::traceLastDirectoryCreation();
#endif

		device.setWatchDescriptor( _pGKfs->addNotifyDirectoryWatch( directory.string(), check ) );
	}
	catch ( const GLogiKExcept & e ) {
		LOG(warning) << e.what();
		LOG(warning) << "configuration file monitoring will be disabled";
	}
}

void DevicesHandler::setDeviceCurrentBankID(const std::string & devID, const MKeysID bankID)
{
	GK_LOG_FUNC

	try {
		DeviceProperties & device = _startedDevices.at(devID);

		GKLog2(trace, devID, " started device")

		device.setCurrentBankID(bankID);
	}
	catch (const std::out_of_range& oor) {
		LOG(warning) << devID << " device not found in started-devices container";
		throw GLogiKExcept("unable to set device bankID");
	}
}

banksMap_type & DevicesHandler::getDeviceBanks(const std::string & devID, MKeysID & bankID)
{
	GK_LOG_FUNC

	try {
		DeviceProperties & device = _startedDevices.at(devID);

		GKLog2(trace, devID, " started device")

		/* setting current bank ID */
		bankID = device.getCurrentBankID();

		return device.getBanks();
	}
	catch (const std::out_of_range& oor) {
		LOG(warning) << devID << " device not found in started-devices container";
		throw GLogiKExcept("unable to get device banks");
	}
}

void DevicesHandler::doDeviceFakeKeyEvent(
	const std::string & devID,
	const std::string & mediaKeyEvent)
{
	GK_LOG_FUNC

	try {
		_startedDevices.at(devID);

		Display* dpy = XOpenDisplay(NULL);
		if(dpy == nullptr)
			throw GLogiKExcept("XOpenDisplay failure");

		KeySym sym = XStringToKeysym( mediaKeyEvent.c_str() );
		if(sym == NoSymbol) {
			std::string error("invalid KeySym : ");
			error += mediaKeyEvent;
			XCloseDisplay(dpy);
			throw GLogiKExcept(error);
		}

		KeyCode code = XKeysymToKeycode(dpy, sym);
		if(code == 0) {
			std::string error("not found KeySym : ");
			error += mediaKeyEvent;
			XCloseDisplay(dpy);
			throw GLogiKExcept(error);
		}

		XTestFakeKeyEvent(dpy, code, True, 0);
		XTestFakeKeyEvent(dpy, code, False, 0);
		XFlush(dpy);

		XCloseDisplay(dpy);

		GKLog3(trace, devID, " done fake media event : ", mediaKeyEvent)
	}
	catch (const std::out_of_range& oor) {
		LOG(warning) << devID << " device not found in started-devices container";
	}
	catch (const GLogiKExcept & e) {
		LOG(error) << devID << " error faking media key event : " << e.what();
	}
}

const LCDPPArray_type &
	DevicesHandler::getDeviceLCDPluginsProperties(
		const std::string & devID)
{
	GK_LOG_FUNC

	try {
		try {
			DeviceProperties & device = _startedDevices.at(devID);

			GKLog2(trace, devID, " started device")

			return device.getLCDPluginsProperties();
		}
		catch (const std::out_of_range& oor) {
			DeviceProperties & device = _stoppedDevices.at(devID);

			GKLog2(trace, devID, " stopped device")

			return device.getLCDPluginsProperties();
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(warning) << devID << " device not found";
	}

	return DeviceProperties::_LCDPluginsPropertiesEmptyArray;
}

const MKeysIDArray_type DevicesHandler::getDeviceMKeysIDArray(const std::string & devID)
{
	MKeysIDArray_type MKeysIDArray;
	const std::string remoteMethod("GetDeviceMKeysIDArray");

	try {
		DBus.initializeRemoteMethodCall(
			_systemBus,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		DBus.appendStringToRemoteMethodCall(_clientID);
		DBus.appendStringToRemoteMethodCall(devID);

		DBus.sendRemoteMethodCall();

		try {
			DBus.waitForRemoteMethodCallReply();

			MKeysIDArray = DBus.getNextMKeysIDArrayArgument();
			GKLog3(trace, devID, " number of M-keys ID : ", MKeysIDArray.size())
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		DBus.abandonRemoteMethodCall();
		LogRemoteCallFailure
	}

	return MKeysIDArray;
}

const GKeysIDArray_type DevicesHandler::getDeviceGKeysIDArray(const std::string & devID)
{
	GKeysIDArray_type GKeysIDArray;
	const std::string remoteMethod("GetDeviceGKeysIDArray");

	try {
		DBus.initializeRemoteMethodCall(
			_systemBus,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		DBus.appendStringToRemoteMethodCall(_clientID);
		DBus.appendStringToRemoteMethodCall(devID);

		DBus.sendRemoteMethodCall();

		try {
			DBus.waitForRemoteMethodCallReply();

			GKeysIDArray = DBus.getNextGKeysIDArrayArgument();
			GKLog3(trace, devID, " number of G-keys ID : ", GKeysIDArray.size())
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		DBus.abandonRemoteMethodCall();
		LogRemoteCallFailure
	}

	return GKeysIDArray;
}

#if HAVE_DESKTOP_NOTIFICATIONS
void DevicesHandler::showNotification(
	const std::string & devID,
	const std::string & summary,
	const DeviceProperties & device)
{
	std::string body(devID);
	body += " ";
	body += device.getVendor();
	body += " ";
	body += device.getProduct();
	body += " ";
	body += device.getName();

	// icon - "dialog-information"
	desktopNotification n(summary, body, "GLogiK");
}
#endif

} // namespace GLogiK

