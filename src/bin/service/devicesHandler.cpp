/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2020  Fabrice Delliaux <netbox253@gmail.com>
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
#include <set>

#include <boost/archive/archive_exception.hpp>
#include <boost/archive/xml_archive_exception.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

#include "include/LCDPluginProperties.hpp"

#include "lib/shared/deviceConfigurationFile.hpp"

#include "devicesHandler.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

DevicesHandler::DevicesHandler()
	:	_clientID("undefined"),
		_pDBus(nullptr),
		_pGKfs(nullptr),
		_systemBus(NSGKDBus::BusConnection::GKDBUS_SYSTEM)
{
#if DEBUGGING_ON
	LOG(DEBUG) << "Devices Handler initialization";
#endif
	_configurationRootDirectory = XDGUserDirs::getConfigurationRootDirectory();
	_configurationRootDirectory /= PACKAGE_NAME;

	FileSystem::createDirectory(_configurationRootDirectory, fs::owner_all);
}

DevicesHandler::~DevicesHandler() {
#if DEBUGGING_ON
	LOG(DEBUG) << "Devices Handler destruction";
#endif
}

void DevicesHandler::setGKfs(NSGKUtils::FileSystem* pGKfs) {
	_pGKfs = pGKfs;
}

void DevicesHandler::setDBus(NSGKDBus::GKDBus* pDBus) {
	_pDBus = pDBus;
}

void DevicesHandler::setClientID(const std::string & id) {
	_clientID = id;
}

void DevicesHandler::clearDevices(void) {
	std::set<std::string> devicesID;

	for( const auto & devicePair : _startedDevices ) {
		devicesID.insert(devicePair.first);
	}
	for( const auto & devicePair : _stoppedDevices ) {
		devicesID.insert(devicePair.first);
	}

	/* stop each device if not already stopped, and unref it */
	for(const auto & devID : devicesID) {
		this->unplugDevice(devID);
	}

	/* clear all containers */
	_startedDevices.clear();
	_stoppedDevices.clear();
}

const devices_files_map_t DevicesHandler::getDevicesMap(void) {
	devices_files_map_t ret;
	for(const auto & dev : _startedDevices) {
		ret.insert( std::pair<const std::string, const std::string>(dev.first, dev.second.getConfigFilePath()) );
	}
	for(const auto & dev : _stoppedDevices) {
		ret.insert( std::pair<const std::string, const std::string>(dev.first, dev.second.getConfigFilePath()) );
	}
	return ret;
}

const std::vector<std::string> DevicesHandler::getDevicesList(void) {
	std::vector<std::string> ret;
	for(const auto & dev : _startedDevices) {
		ret.push_back(dev.first);
		ret.push_back("started");
		ret.push_back(dev.second.getVendor());
		ret.push_back(dev.second.getProduct());
		ret.push_back(dev.second.getName());
		ret.push_back(dev.second.getConfigFilePath());
	}
	for(const auto & dev : _stoppedDevices) {
		ret.push_back(dev.first);
		ret.push_back("stopped");
		ret.push_back(dev.second.getVendor());
		ret.push_back(dev.second.getProduct());
		ret.push_back(dev.second.getName());
		ret.push_back(dev.second.getConfigFilePath());
	}
	return ret;
}

const bool DevicesHandler::checkDeviceCapability(const DeviceProperties & device, Caps toCheck) {
	return (device.getCapabilities() & toEnumType(toCheck));
}

void DevicesHandler::reloadDeviceConfigurationFile(const std::string & devID) {
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
			LOG(WARNING) << "device " << devID << " not found in containers, giving up";
		}
	}
}

void DevicesHandler::saveDeviceConfigurationFile(
	const std::string & devID,
	const DeviceProperties & device)
{
#if DEBUGGING_ON
	LOG(DEBUG1) << devID << " saving device configuration file";
#endif

	try {
		fs::path filePath(_configurationRootDirectory);
		filePath /= device.getVendor();

		FileSystem::createDirectory(filePath, fs::owner_all);

		filePath /= device.getConfigFilePath();

		DeviceConfigurationFile::save(filePath.string(), device);

		try {
			fs::permissions(filePath, fs::owner_read|fs::owner_write|fs::group_read|fs::others_read);
		}
		catch (const fs::filesystem_error & e) {
			LOG(ERROR) << "set permissions failure on configuration file : " << e.what();
		}
		catch (const std::exception & e) {
			LOG(ERROR) << "set permissions (allocation) failure on configuration file : " << e.what();
		}

		this->sendDeviceConfigurationSavedSignal(devID);
	}
	catch ( const GLogiKExcept & e ) {
		LOG(ERROR) << e.what();
	}
}

void DevicesHandler::sendDeviceConfigurationSavedSignal(const std::string & devID)
{
	std::string status("DeviceConfigurationSaved signal");
	try {
		/* send DeviceConfigurationSaved signal to GUI applications */
		_pDBus->initializeBroadcastSignal(
			NSGKDBus::BusConnection::GKDBUS_SESSION,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
			"DeviceConfigurationSaved"
		);
		_pDBus->appendStringToBroadcastSignal(devID);
		_pDBus->sendBroadcastSignal();

		status += " sent";
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_pDBus->abandonBroadcastSignal();
		status += " failure";
		LOG(ERROR) << status << " - " << e.what();
	}

#if DEBUGGING_ON
	LOG(DEBUG3) << status << " on session bus";
#endif
}

void DevicesHandler::loadDeviceConfigurationFile(DeviceProperties & device) {
	fs::path filePath(_configurationRootDirectory);
	filePath /= device.getVendor();
	filePath /= device.getConfigFilePath();

	DeviceConfigurationFile::load(filePath.string(), device);
}

void DevicesHandler::sendDeviceConfigurationToDaemon(const std::string & devID, const DeviceProperties & device) {
	if( this->checkDeviceCapability(device, Caps::GK_BACKLIGHT_COLOR) ) {
		/* set backlight color */
		const std::string remoteMethod("SetDeviceBacklightColor");

		try {
			_pDBus->initializeRemoteMethodCall(
				_systemBus,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			_pDBus->appendStringToRemoteMethodCall(_clientID);
			_pDBus->appendStringToRemoteMethodCall(devID);
			uint8_t r, g, b = 0; device.getRGBBytes(r, g, b);
			_pDBus->appendUInt8ToRemoteMethodCall(r);
			_pDBus->appendUInt8ToRemoteMethodCall(g);
			_pDBus->appendUInt8ToRemoteMethodCall(b);

			_pDBus->sendRemoteMethodCall();

			try {
				_pDBus->waitForRemoteMethodCallReply();

				const bool ret = _pDBus->getNextBooleanArgument();
				if( ret ) {
					LOG(VERB)	<< devID << " successfully setted device backlight color : "
								<< getHexRGB(r, g, b);
				}
				else {
					LOG(ERROR) << devID << " failed to set device backlight color : false";
				}
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			_pDBus->abandonRemoteMethodCall();
			LogRemoteCallFailure
		}
	}

	if( this->checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
		/* set macros banks */
		for( const auto & idBankPair : device.getMacrosBanks() ) {
			const uint8_t bankID = toEnumType(idBankPair.first);
			const mBank_type & bank = idBankPair.second;

			/* test whether this MacrosBank is empty */
			bool empty = true;
			for(const auto & keyMacroPair : bank) {
				const macro_type & macro = keyMacroPair.second;
				if( ! macro.empty() ) {
					empty = false;
					break;
				}
			}

			if( empty ) {
				/* resetting empty MacrosBank */
				const std::string remoteMethod = "ResetDeviceMacrosBank";

				try {
					_pDBus->initializeRemoteMethodCall(
						_systemBus,
						GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
						GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
						GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
						remoteMethod.c_str()
					);
					_pDBus->appendStringToRemoteMethodCall(_clientID);
					_pDBus->appendStringToRemoteMethodCall(devID);
					_pDBus->appendUInt8ToRemoteMethodCall(bankID);

					_pDBus->sendRemoteMethodCall();

					try {
						_pDBus->waitForRemoteMethodCallReply();

						const bool ret = _pDBus->getNextBooleanArgument();
						if( ret ) {
							LOG(VERB) << devID << " successfully resetted device MacrosBank " << toUInt(bankID);
						}
						else {
							LOG(ERROR) << devID << " failed to reset device MacrosBank " << toUInt(bankID) << " : false";
						}
					}
					catch (const GLogiKExcept & e) {
						LogRemoteCallGetReplyFailure
					}
				}
				catch (const GKDBusMessageWrongBuild & e) {
					_pDBus->abandonRemoteMethodCall();
					LogRemoteCallFailure
				}

				continue; /* jump to next bank */
			}

			const std::string remoteMethod = "SetDeviceMacrosBank";

			try {
				_pDBus->initializeRemoteMethodCall(
					_systemBus,
					GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
					GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
					GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
					remoteMethod.c_str()
				);
				_pDBus->appendStringToRemoteMethodCall(_clientID);
				_pDBus->appendStringToRemoteMethodCall(devID);
				_pDBus->appendUInt8ToRemoteMethodCall(bankID);
				_pDBus->appendMacrosBankToRemoteMethodCall(bank);

				_pDBus->sendRemoteMethodCall();

				try {
					_pDBus->waitForRemoteMethodCallReply();

					const bool ret = _pDBus->getNextBooleanArgument();
					if( ret ) {
						LOG(VERB) << devID << " successfully setted device MacrosBank " << toUInt(bankID);
					}
					else {
						LOG(ERROR) << devID << " failed to set device MacrosBank " << toUInt(bankID) << " : false";
					}
				}
				catch (const GLogiKExcept & e) {
					LogRemoteCallGetReplyFailure
				}
			}
			catch (const GKDBusMessageWrongBuild & e) {
				_pDBus->abandonRemoteMethodCall();
				LogRemoteCallFailure
			}
		}
	}

	if( this->checkDeviceCapability(device, Caps::GK_LCD_SCREEN) ) {
		const std::string remoteMethod = "SetDeviceLCDPluginsMask";

		try {
			const uint8_t maskID = toEnumType(LCDPluginsMask::GK_LCD_PLUGINS_MASK_1);
			const uint64_t mask = device.getLCDPluginsMask1();

			_pDBus->initializeRemoteMethodCall(
				_systemBus,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			_pDBus->appendStringToRemoteMethodCall(_clientID);
			_pDBus->appendStringToRemoteMethodCall(devID);
			_pDBus->appendUInt8ToRemoteMethodCall(maskID);
			_pDBus->appendUInt64ToRemoteMethodCall(mask);

			_pDBus->sendRemoteMethodCall();

			try {
				_pDBus->waitForRemoteMethodCallReply();

				const bool ret = _pDBus->getNextBooleanArgument();
				if( ret ) {
					LOG(VERB) << devID << " successfully setted device LCD Plugins Mask " << toUInt(maskID);
				}
				else {
					LOG(ERROR) << devID << " failed to set device LCD Plugins Mask " << toUInt(maskID) << " : false";
				}
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			_pDBus->abandonRemoteMethodCall();
			LogRemoteCallFailure
		}
	}

	LOG(INFO) << devID << " sent device configuration to daemon";
}

void DevicesHandler::setDeviceProperties(const std::string & devID, DeviceProperties & device) {
	/* initialize device properties */
	std::string remoteMethod("GetDeviceProperties");

	try {
		_pDBus->initializeRemoteMethodCall(
			_systemBus,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		_pDBus->appendStringToRemoteMethodCall(_clientID);
		_pDBus->appendStringToRemoteMethodCall(devID);

		_pDBus->sendRemoteMethodCall();

		try {
			_pDBus->waitForRemoteMethodCallReply();

			const std::string vendor( _pDBus->getNextStringArgument() );
			const std::string product( _pDBus->getNextStringArgument() );
			const std::string name( _pDBus->getNextStringArgument() );
			const uint64_t caps( _pDBus->getNextUInt64Argument() );
			device.setProperties( vendor, product, name, caps );

#if DEBUGGING_ON
			LOG(DEBUG3) << devID << " got 4 properties";
#endif
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_pDBus->abandonRemoteMethodCall();
		LogRemoteCallFailure
	}

	if( this->checkDeviceCapability(device, Caps::GK_LCD_SCREEN) ) {
		/* get LCD plugins properties */
		remoteMethod = "GetDeviceLCDPluginsProperties";

		try {
			_pDBus->initializeRemoteMethodCall(
				_systemBus,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			_pDBus->appendStringToRemoteMethodCall(_clientID);
			_pDBus->appendStringToRemoteMethodCall(devID);

			_pDBus->sendRemoteMethodCall();

			try {
				_pDBus->waitForRemoteMethodCallReply();

				const LCDPluginsPropertiesArray_type array = _pDBus->getNextLCDPluginsArrayArgument();
				device.setLCDPluginsProperties(array);
#if DEBUGGING_ON
				LOG(DEBUG3) << devID << " got " << array.size() << " LCDPluginsProperties objects";
#endif
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			_pDBus->abandonRemoteMethodCall();
			LogRemoteCallFailure
		}
	}

	if( this->checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
		/* initialize macro keys */
		remoteMethod = "GetDeviceMacroKeysNames";

		try {
			_pDBus->initializeRemoteMethodCall(
				_systemBus,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			_pDBus->appendStringToRemoteMethodCall(_clientID);
			_pDBus->appendStringToRemoteMethodCall(devID);

			_pDBus->sendRemoteMethodCall();

			try {
				_pDBus->waitForRemoteMethodCallReply();

				const std::vector<std::string> keysNames( _pDBus->getStringsArray() );
				device.initMacrosBanks(keysNames);
#if DEBUGGING_ON
				LOG(DEBUG3) << devID << " initialized " << keysNames.size() << " macro keys";
#endif
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			_pDBus->abandonRemoteMethodCall();
			LogRemoteCallFailure
		}
	}

	/* search a configuration file */

#if DEBUGGING_ON
	LOG(DEBUG2) << devID << " assigning a configuration file";
#endif
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
#if DEBUGGING_ON
			LOG(DEBUG1) << e.what();
#endif
			throw GLogiKExcept("configuration file not found");
		}

#if DEBUGGING_ON
		LOG(DEBUG3) << "found : " << device.getConfigFilePath();
#endif
		/* loading configuration file */
		this->loadDeviceConfigurationFile(device);
		LOG(INFO)	<< "found device " << devID << " - "
					<< device.getVendor() << " "
					<< device.getProduct() << " "
					<< device.getName();
		LOG(INFO)	<< devID << " configuration file found and loaded";

		/* assuming that directory is readable since we just load
		 * the configuration file */
		this->watchDirectory(device, false);

		this->sendDeviceConfigurationToDaemon(devID, device);
	}
	catch ( const GLogiKExcept & e ) { /* should happen only when configuration file not found */
#if DEBUGGING_ON
		LOG(DEBUG1) << e.what();
#endif
		LOG(INFO)	<< devID << " started device : "
					<< device.getVendor() << " "
					<< device.getProduct() << " "
					<< device.getName();
		LOG(INFO) << devID << " creating default configuration file";

		try {
			std::string baseName(device.getProduct());
			baseName += " ";
			baseName += device.getName();

			/* none found, assign a new configuration file to this device */
			device.setConfigFilePath(
				_pGKfs->getNextAvailableFileName(alreadyUsed, directory, baseName, "cfg")
			);

#if DEBUGGING_ON
			LOG(DEBUG3) << "new one : " << device.getConfigFilePath();
#endif
			/* we need to create the directory before watching it */
			this->saveDeviceConfigurationFile(devID, device);

			/* assuming that directory is readable since we just save
			 * the configuration file and set permissions on directory */
			this->watchDirectory(device, false);
		}
		catch ( const GLogiKExcept & e ) {
			LOG(ERROR) << e.what();
		}
	}
}

void DevicesHandler::startDevice(const std::string & devID) {
	try {
		_startedDevices.at(devID);
#if DEBUGGING_ON
		LOG(DEBUG1) << "found already started device: " << devID;
#endif
	}
	catch (const std::out_of_range& oor) {
		try {
			DeviceProperties & device = _stoppedDevices.at(devID);
			LOG(INFO) << "starting device: " << devID;
			_startedDevices[devID] = device;
			_stoppedDevices.erase(devID);
		}
		catch (const std::out_of_range& oor) {
#if DEBUGGING_ON
			LOG(DEBUG) << "initializing device: " << devID;
#endif
			DeviceProperties device;
			/* also load configuration file */
			this->setDeviceProperties(devID, device);
			_startedDevices[devID] = device;
		}
	}
}

void DevicesHandler::stopDevice(const std::string & devID) {
	try {
		_stoppedDevices.at(devID);
#if DEBUGGING_ON
		LOG(DEBUG1) << "found already stopped device: " << devID;
#endif
	}
	catch (const std::out_of_range& oor) {
		try {
			DeviceProperties & device = _startedDevices.at(devID);
			LOG(INFO) << "stopping device: " << devID;
			_stoppedDevices[devID] = device;
			_startedDevices.erase(devID);
		}
		catch (const std::out_of_range& oor) {
			/* this can happen when GLogiKs start and this device is already stopped
			 * so we must start the device to stop it */
#if DEBUGGING_ON
			LOG(DEBUG) << "device " << devID << " not found in containers, initializing and stopping it";
#endif
			this->startDevice(devID);
			LOG(INFO) << "stopping device: " << devID;
			try {
				_stoppedDevices[devID] = _startedDevices.at(devID);
				_startedDevices.erase(devID);
			}
			catch (const std::out_of_range& oor) {
				LOG(ERROR) << "started device not found, something is really wrong";
			}
		}
	}
}

void DevicesHandler::unplugDevice(const std::string & devID) {
	try {
		_stoppedDevices.at(devID);
#if DEBUGGING_ON
		LOG(DEBUG1) << "found already stopped device: " << devID;
#endif
		this->unrefDevice(devID);
	}
	catch (const std::out_of_range& oor) {
		this->stopDevice(devID);
		this->unrefDevice(devID);
	}
}

void DevicesHandler::unrefDevice(const std::string & devID) {
	try {
		const DeviceProperties & device = _stoppedDevices.at(devID);

		_pGKfs->removeNotifyWatch( device.getWatchDescriptor() );

		_stoppedDevices.erase(devID);
#if DEBUGGING_ON
		LOG(DEBUG2) << devID << " erased device";
#endif

		std::string remoteMethod("DeleteDeviceConfiguration");

		try {
			_pDBus->initializeRemoteMethodCall(
				_systemBus,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			_pDBus->appendStringToRemoteMethodCall(_clientID);
			_pDBus->appendStringToRemoteMethodCall(devID);

			_pDBus->sendRemoteMethodCall();

			try {
				_pDBus->waitForRemoteMethodCallReply();
				const bool ret = _pDBus->getNextBooleanArgument();
				if( ret ) {
#if DEBUGGING_ON
					LOG(DEBUG3) << devID << " successfully deleted remote device configuration ";
#endif
				}
				else {
					LOG(ERROR) << devID << " failed to delete remote device configuration : false";
				}
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			_pDBus->abandonRemoteMethodCall();
			LogRemoteCallFailure
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(WARNING) << "device not found in stopped device container: " << devID;
	}
}

const bool DevicesHandler::setDeviceMacro(
	const std::string & devID,
	const std::string & keyName,
	const uint8_t bankID)
{
	try {
		DeviceProperties & device = _startedDevices.at(devID);

		if( this->checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
			std::string remoteMethod("GetDeviceMacro");

			try {
				/* getting recorded macro from daemon */
				_pDBus->initializeRemoteMethodCall(
					_systemBus,
					GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
					GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
					GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
					remoteMethod.c_str()
				);

				_pDBus->appendStringToRemoteMethodCall(_clientID);
				_pDBus->appendStringToRemoteMethodCall(devID);
				_pDBus->appendStringToRemoteMethodCall(keyName);
				_pDBus->appendUInt8ToRemoteMethodCall(bankID);

				_pDBus->sendRemoteMethodCall();

				try {
					_pDBus->waitForRemoteMethodCallReply();

					/* use helper function to get the macro */
					const macro_type macro = _pDBus->getNextMacroArgument();

					device.setMacro(bankID, keyName, macro);

					this->saveDeviceConfigurationFile(devID, device);
					this->watchDirectory(device);
					return true;
				}
				catch (const GLogiKExcept & e) {
					/* setMacro can also throws */
					LogRemoteCallGetReplyFailure
				}
			}
			catch (const GKDBusMessageWrongBuild & e) {
				_pDBus->abandonRemoteMethodCall();
				LogRemoteCallFailure
			}
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(WARNING) << "device not found : " << devID;
	}

	return false;
}

const bool DevicesHandler::clearDeviceMacro(
	const std::string & devID,
	const std::string & keyName,
	const uint8_t bankID)
{
	try {
		DeviceProperties & device = _startedDevices.at(devID);

		if( this->checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
			device.clearMacro(bankID, keyName);

			this->saveDeviceConfigurationFile(devID, device);
			this->watchDirectory(device);
			return true;
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(WARNING) << "device not found : " << devID;
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << "clear macro failure: " << e.what();
	}
	return false;
}

void DevicesHandler::watchDirectory(DeviceProperties & device, const bool check) {
	fs::path directory(_configurationRootDirectory);
	directory /= device.getVendor();

	try {
		device.setWatchDescriptor( _pGKfs->addNotifyDirectoryWatch( directory.string(), check ) );
	}
	catch ( const GLogiKExcept & e ) {
		LOG(WARNING) << e.what();
		LOG(WARNING) << "configuration file monitoring will be disabled";
	}
}

void DevicesHandler::doDeviceFakeKeyEvent(
	const std::string & devID,
	const std::string & mediaKeyEvent)
{
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
#if DEBUGGING_ON
		LOG(DEBUG1) << "done fake media event : " << mediaKeyEvent;
#endif

		XCloseDisplay(dpy);
	}
	catch (const std::out_of_range& oor) {
		LOG(WARNING) << "device not found : " << devID;
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << "error simulating media key event : " << e.what();
	}
}

const LCDPluginsPropertiesArray_type & DevicesHandler::getDeviceLCDPluginsProperties(
	const std::string & devID)
{
	try {
		try {
			DeviceProperties & device = _startedDevices.at(devID);
#if DEBUGGING_ON
			LOG(DEBUG3) << "found started device: " << devID;
#endif
			return device.getLCDPluginsProperties();
		}
		catch (const std::out_of_range& oor) {
			DeviceProperties & device = _stoppedDevices.at(devID);
#if DEBUGGING_ON
			LOG(DEBUG3) << "found stopped device: " << devID;
#endif
			return device.getLCDPluginsProperties();
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(WARNING) << "device not found : " << devID;
	}

	return DeviceProperties::_LCDPluginsPropertiesEmptyArray;
}

} // namespace GLogiK

