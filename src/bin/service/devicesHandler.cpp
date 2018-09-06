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

#include <utility>
#include <exception>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <set>
#include <thread>

#include <boost/archive/archive_exception.hpp>
#include <boost/archive/xml_archive_exception.hpp>
#include <boost/filesystem.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/process.hpp>

#include "global.hpp"

#include "devicesHandler.hpp"

namespace fs = boost::filesystem;
namespace bp = boost::process;

namespace GLogiK
{

using namespace NSGKUtils;

DevicesHandler::DevicesHandler()
	:	_pDBus(nullptr),
		_pGKfs(nullptr),
		_systemBus(NSGKDBus::BusConnection::GKDBUS_SYSTEM),
		_clientID("undefined")
{
#if DEBUGGING_ON
	LOG(DEBUG) << "Devices Handler initialization";
#endif
	_configurationRootDirectory = XDGUserDirs::getConfigDirectory();
	_configurationRootDirectory += "/";
	_configurationRootDirectory += PACKAGE_NAME;

	FileSystem::createOwnerDirectory(_configurationRootDirectory);
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
		ret.insert( std::pair<const std::string, const std::string>(dev.first, dev.second.getConfigFileName()) );
	}
	for(const auto & dev : _stoppedDevices) {
		ret.insert( std::pair<const std::string, const std::string>(dev.first, dev.second.getConfigFileName()) );
	}
	return ret;
}

const bool DevicesHandler::checkDeviceCapability(const DeviceProperties & device, Caps toCheck) {
	return (device.getCapabilities() & to_type(toCheck));
}

void DevicesHandler::checkDeviceConfigurationFile(const std::string & devID) {
	try {
		DeviceProperties & device = _startedDevices.at(devID);
		this->loadDeviceConfigurationFile(device);

		this->sendDeviceConfigurationToDaemon(devID, device);
	}
	catch (const std::out_of_range& oor) {
		try {
			DeviceProperties & device = _stoppedDevices.at(devID);
			this->loadDeviceConfigurationFile(device);

			this->sendDeviceConfigurationToDaemon(devID, device);
		}
		catch (const std::out_of_range& oor) {
			LOG(WARNING) << "device " << devID << " not found in containers, giving up";
		}
	}
}

void DevicesHandler::saveDeviceConfigurationFile(
	const std::string & devID,
	const DeviceProperties & device )
{
	try {
		fs::path filePath(_configurationRootDirectory);
		filePath /= device.getVendor();

		FileSystem::createOwnerDirectory(filePath);

		filePath /= device.getConfigFileName();

		try {
#if DEBUGGING_ON
			LOG(DEBUG2) << devID << " trying to open configuration file for writing : " << filePath.string();
#endif

			std::ofstream ofs;
			ofs.exceptions(std::ofstream::failbit|std::ofstream::badbit);
			ofs.open(filePath.string(), std::ofstream::out|std::ofstream::trunc);

			fs::permissions(filePath, fs::owner_read|fs::owner_write|fs::group_read|fs::others_read);
#if DEBUGGING_ON
			LOG(DEBUG3) << "opened";
#endif

			{
				boost::archive::text_oarchive outputArchive(ofs);
				outputArchive << device;
			}

			LOG(INFO) << devID << " successfully saved configuration file, closing";
			ofs.close();
		}
		catch (const std::ofstream::failure & e) {
			std::ostringstream buffer("fail to open configuration file : ", std::ios_base::app);
			buffer << e.what();
			LOG(ERROR) << buffer.str();
		}
		catch (const fs::filesystem_error & e) {
			std::ostringstream buffer("set permissions failure on configuration file : ", std::ios_base::app);
			buffer << e.what();
			LOG(ERROR) << buffer.str();
		}
		catch(const boost::archive::archive_exception & e) {
			std::ostringstream buffer("boost::archive exception : ", std::ios_base::app);
			buffer << e.what();
			LOG(ERROR) << buffer.str();
		}
		/*
		 * catch std::ios_base::failure on buggy compilers
		 * should be fixed with gcc >= 7.0
		 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145
		 */
		catch( const std::exception & e ) {
			std::ostringstream buffer("(buggy exception) fail to open configuration file : ", std::ios_base::app);
			buffer << e.what();
			LOG(ERROR) << buffer.str();
		}
	}
	catch ( const GLogiKExcept & e ) {
		LOG(ERROR) << e.what();
	}
}

void DevicesHandler::loadDeviceConfigurationFile(DeviceProperties & device) {
	fs::path filePath(_configurationRootDirectory);
	filePath /= device.getVendor();
	filePath /= device.getConfigFileName();

#if DEBUGGING_ON
	LOG(DEBUG2) << "loading device configuration file " << device.getConfigFileName();
#endif
	try {
		std::ifstream ifs;
		ifs.exceptions(std::ifstream::badbit);
		ifs.open(filePath.string());
#if DEBUGGING_ON
		LOG(DEBUG2) << "configuration file successfully opened for reading";
#endif

		{
			DeviceProperties newDevice;
			boost::archive::text_iarchive inputArchive(ifs);
			inputArchive >> newDevice;

			device.setProperties( newDevice );
		}

#if DEBUGGING_ON
		LOG(DEBUG3) << "success, closing";
#endif
		ifs.close();
	}
	catch (const std::ifstream::failure & e) {
		std::ostringstream buffer("fail to open configuration file : ", std::ios_base::app);
		buffer << e.what();
		LOG(ERROR) << buffer.str();
	}
	catch(const boost::archive::archive_exception & e) {
		std::ostringstream buffer("boost::archive exception : ", std::ios_base::app);
		buffer << e.what();
		LOG(ERROR) << buffer.str();
		// TODO throw GLogiKExcept to create new configuration
		// file and avoid overwriting on close ?
	}
	/*
	 * catch std::ios_base::failure on buggy compilers
	 * should be fixed with gcc >= 7.0
	 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145
	 */
	catch( const std::exception & e ) {
		std::ostringstream buffer("(buggy exception) fail to open configuration file : ", std::ios_base::app);
		buffer << e.what();
		LOG(ERROR) << buffer.str();
	}
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
			const uint8_t bankID = to_type(idBankPair.first);
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

				LOG(VERB) << devID << " resetting empty MacrosBank " << to_uint(bankID);

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
							LOG(VERB) << devID << " successfully resetted device MacrosBank " << to_uint(bankID);
						}
						else {
							LOG(ERROR) << devID << " failed to reset device MacrosBank " << to_uint(bankID) << " : false";
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
						LOG(VERB) << devID << " successfully setted device MacrosBank " << to_uint(bankID);
					}
					else {
						LOG(ERROR) << devID << " failed to set device MacrosBank " << to_uint(bankID) << " : false";
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
			const std::string model( _pDBus->getNextStringArgument() );
			const uint64_t caps( _pDBus->getNextUInt64Argument() );
			device.setProperties( vendor, model, caps );

#if DEBUGGING_ON
			LOG(DEBUG3) << devID << " got 3 properties";
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
			alreadyUsed.insert( dev.second.getConfigFileName() );
		}
		for(const auto & dev : _stoppedDevices) {
			alreadyUsed.insert( dev.second.getConfigFileName() );
		}
	}

	try {
		try {
			/* trying to find an existing configuration file */
			device.setConfigFileName(
				_pGKfs->getNextAvailableFileName(alreadyUsed, directory, device.getModel(), "cfg", true)
			);
		}
		catch ( const GLogiKExcept & e ) {
#if DEBUGGING_ON
			LOG(DEBUG1) << e.what();
#endif
			throw GLogiKExcept("configuration file not found");
		}

#if DEBUGGING_ON
		LOG(DEBUG3) << "found : " << device.getConfigFileName();
#endif
		/* loading configuration file */
		this->loadDeviceConfigurationFile(device);
		LOG(INFO)	<< "found device " << devID << " - "
					<< device.getVendor() << " " << device.getModel();
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
		LOG(INFO) << devID << " started device : " << device.getVendor()
				<< " " << device.getModel();
		LOG(INFO) << devID << " creating default configuration file";

		try {
			/* none found, assign a new configuration file to this device */
			device.setConfigFileName(
				_pGKfs->getNextAvailableFileName(alreadyUsed, directory, device.getModel(), "cfg")
			);

#if DEBUGGING_ON
			LOG(DEBUG3) << "new one : " << device.getConfigFileName();
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
		return;
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
		return;
	}
	catch (const std::out_of_range& oor) {
		try {
			DeviceProperties & device = _startedDevices.at(devID);
			LOG(INFO) << "stopping device: " << devID;
			_stoppedDevices[devID] = device;
			_startedDevices.erase(devID);
		}
		catch (const std::out_of_range& oor) {
			LOG(WARNING) << "device " << devID << " not found in containers, giving up";
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

void DevicesHandler::runDeviceMediaEvent(
	const std::string & devID,
	const std::string & mediaKeyEvent)
{
	try {
		DeviceProperties & device = _startedDevices.at(devID);
		const std::string cmd( device.getMediaCommand(mediaKeyEvent) );
		if( ! cmd.empty() ) {
			std::thread mediaEventThread(&DevicesHandler::runCommand, this, mediaKeyEvent, cmd);
			mediaEventThread.detach();
		}
		else {
#if DEBUGGING_ON
			LOG(DEBUG2) << "empty command media event " << mediaKeyEvent;
#endif
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(WARNING) << "device not found : " << devID;
	}
}

void DevicesHandler::runCommand(
	const std::string & mediaKeyEvent,
	const std::string & command
	)
{
#if DESKTOP_NOTIFICATIONS
	_notification.init(GLOGIKS_DESKTOP_SERVICE_NAME, 5000);
#endif

	std::string line;
	std::string last;
	bp::ipstream pipeStream;
	bp::child c(command, bp::std_out > pipeStream);

	while (pipeStream && std::getline(pipeStream, line) && !line.empty()) {
		last = line;
		LOG(VERB) << line;
	}

	if( c.running() ) {
		try {
			c.wait();
		}
		catch (const bp::process_error & e) {
			LOG(DEBUG1) << e.what();
		}
	}

#if DESKTOP_NOTIFICATIONS
	if( (mediaKeyEvent == std::string(XF86_AUDIO_RAISE_VOLUME)) or
		(mediaKeyEvent == std::string(XF86_AUDIO_LOWER_VOLUME)) ) {
		try {
			int volume = -1;
			try {
				volume = std::stoi(last);
			}
			catch (const std::invalid_argument& ia) {
				throw GLogiKExcept("stoi invalid argument");
			}
			catch (const std::out_of_range& oor) {
				throw GLogiKExcept("stoi out of range");
			}

			std::string icon("");
			if( volume <= 0 )
				icon = "audio-volume-muted-symbolic";
			else if ( volume <= 30 )
				icon = "audio-volume-low-symbolic";
			else if ( volume <= 70 )
				icon = "audio-volume-medium-symbolic";
			else
				icon = "audio-volume-high-symbolic";

			last += " %";
			if( _notification.updateProperties("Volume", last, icon) ) {
				if( ! _notification.show() ) {
					LOG(ERROR) << "notification showing failure";
				}
			}
			else {
				LOG(ERROR) << "notification update properties failure";
			}
		}
		catch (const GLogiKExcept & e) {
			LOG(ERROR) << "volume conversion failure, notification error";
		}
	}
#endif

	LOG(INFO) << "command run : " << command << " - exit code : " << c.exit_code();
};

} // namespace GLogiK

