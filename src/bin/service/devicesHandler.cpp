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
#include <fstream>
#include <set>
#include <thread>

#include <boost/archive/archive_exception.hpp>
#include <boost/archive/xml_archive_exception.hpp>
#include <boost/filesystem.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/process.hpp>

#include "devicesHandler.h"

namespace fs = boost::filesystem;
namespace bp = boost::process;

namespace GLogiK
{

using namespace NSGKUtils;

DevicesHandler::DevicesHandler()
	:	pDBus_(nullptr),
		pGKfs_(nullptr),
		system_bus_(NSGKDBus::BusConnection::GKDBUS_SYSTEM),
		client_id_("undefined"),
		buffer_("", std::ios_base::app)
{
#if DEBUGGING_ON
	LOG(DEBUG) << "Devices Handler initialization";
#endif
	this->config_root_directory_ = XDGUserDirs::getConfigDirectory();
	this->config_root_directory_ += "/";
	this->config_root_directory_ += PACKAGE_NAME;

	FileSystem::createOwnerDirectory(this->config_root_directory_);
}

DevicesHandler::~DevicesHandler() {
#if DEBUGGING_ON
	LOG(DEBUG) << "Devices Handler destruction";
#endif
}

void DevicesHandler::setGKfs(NSGKUtils::FileSystem* pGKfs) {
	this->pGKfs_ = pGKfs;
}

void DevicesHandler::setDBus(NSGKDBus::GKDBus* pDBus) {
	this->pDBus_ = pDBus;
}

void DevicesHandler::setClientID(const std::string & id) {
	this->client_id_ = id;
}

void DevicesHandler::clearDevices(void) {
	std::set<std::string> devicesID;

	for( const auto & device_pair : this->started_devices_ ) {
		devicesID.insert(device_pair.first);
	}
	for( const auto & device_pair : this->stopped_devices_ ) {
		devicesID.insert(device_pair.first);
	}

	/* stop each device if not already stopped, and unref it */
	for(const auto & devID : devicesID) {
		this->unplugDevice(devID);
	}

	/* clear all containers */
	this->started_devices_.clear();
	this->stopped_devices_.clear();
}

const devices_files_map_t DevicesHandler::getDevicesMap(void) {
	devices_files_map_t ret;
	for(const auto & dev : this->started_devices_) {
		ret.insert( std::pair<const std::string, const std::string>(dev.first, dev.second.getConfigFileName()) );
	}
	for(const auto & dev : this->stopped_devices_) {
		ret.insert( std::pair<const std::string, const std::string>(dev.first, dev.second.getConfigFileName()) );
	}
	return ret;
}

const bool DevicesHandler::checkDeviceCapability(const DeviceProperties & device, Caps to_check) {
	return (device.getCapabilities() & to_type(to_check));
}

void DevicesHandler::checkDeviceConfigurationFile(const std::string & devID) {
	try {
		DeviceProperties & device = this->started_devices_.at(devID);
		this->loadDeviceConfigurationFile(device);

		this->sendDeviceConfigurationToDaemon(devID, device);
	}
	catch (const std::out_of_range& oor) {
		try {
			DeviceProperties & device = this->stopped_devices_.at(devID);
			this->loadDeviceConfigurationFile(device);

			this->sendDeviceConfigurationToDaemon(devID, device);
		}
		catch (const std::out_of_range& oor) {
			LOG(WARNING) << "device [" << devID << "] not found in containers, giving up";
		}
	}
}

void DevicesHandler::saveDeviceConfigurationFile(
	const std::string & devID,
	const DeviceProperties & device )
{
	try {
		fs::path current_path(this->config_root_directory_);
		current_path /= device.getVendor();

		FileSystem::createOwnerDirectory(current_path);

		current_path /= device.getConfigFileName();

		try {
#if DEBUGGING_ON
			LOG(DEBUG2) << "[" << devID << "] trying to open configuration file for writing : " << current_path.string();
#endif

			std::ofstream ofs;
			ofs.exceptions(std::ofstream::failbit|std::ofstream::badbit);
			ofs.open(current_path.string(), std::ofstream::out|std::ofstream::trunc);

			fs::permissions(current_path, fs::owner_read|fs::owner_write|fs::group_read|fs::others_read);
#if DEBUGGING_ON
			LOG(DEBUG3) << "opened";
#endif

			{
				boost::archive::text_oarchive output_archive(ofs);
				output_archive << device;
			}

			LOG(INFO) << "[" << devID << "] successfully saved configuration file, closing";
			ofs.close();
		}
		catch (const std::ofstream::failure & e) {
			this->buffer_.str("fail to open configuration file : ");
			this->buffer_ << e.what();
			LOG(ERROR) << this->buffer_.str();
		}
		catch (const fs::filesystem_error & e) {
			this->buffer_.str("set permissions failure on configuration file : ");
			this->buffer_ << e.what();
			LOG(ERROR) << this->buffer_.str();
		}
		catch(const boost::archive::archive_exception & e) {
			this->buffer_.str("boost::archive exception : ");
			this->buffer_ << e.what();
			LOG(ERROR) << this->buffer_.str();
		}
		/*
		 * catch std::ios_base::failure on buggy compilers
		 * should be fixed with gcc >= 7.0
		 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145
		 */
		catch( const std::exception & e ) {
			this->buffer_.str("(buggy exception) fail to open configuration file : ");
			this->buffer_ << e.what();
			LOG(ERROR) << this->buffer_.str();
		}
	}
	catch ( const GLogiKExcept & e ) {
		LOG(ERROR) << e.what();
	}
}

void DevicesHandler::loadDeviceConfigurationFile(DeviceProperties & device) {
	fs::path current_path(this->config_root_directory_);
	current_path /= device.getVendor();
	current_path /= device.getConfigFileName();

#if DEBUGGING_ON
	LOG(DEBUG2) << "loading device configuration file " << device.getConfigFileName();
#endif
	try {
		std::ifstream ifs;
		ifs.exceptions(std::ifstream::badbit);
		ifs.open(current_path.string());
#if DEBUGGING_ON
		LOG(DEBUG2) << "configuration file successfully opened for reading";
#endif

		{
			DeviceProperties new_device;
			boost::archive::text_iarchive input_archive(ifs);
			input_archive >> new_device;

			device.setProperties( new_device );
		}

#if DEBUGGING_ON
		LOG(DEBUG3) << "success, closing";
#endif
		ifs.close();
	}
	catch (const std::ifstream::failure & e) {
		this->buffer_.str("fail to open configuration file : ");
		this->buffer_ << e.what();
		LOG(ERROR) << this->buffer_.str();
	}
	catch(const boost::archive::archive_exception & e) {
		std::string err("load: ");
		err += e.what();
		LOG(ERROR) << err;
		// TODO throw GLogiKExcept to create new configuration
		// file and avoid overwriting on close ?
	}
	/*
	 * catch std::ios_base::failure on buggy compilers
	 * should be fixed with gcc >= 7.0
	 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145
	 */
	catch( const std::exception & e ) {
		this->buffer_.str("(buggy exception) fail to open configuration file : ");
		this->buffer_ << e.what();
		LOG(ERROR) << this->buffer_.str();
	}
}

void DevicesHandler::sendDeviceConfigurationToDaemon(const std::string & devID, const DeviceProperties & device) {
	if( this->checkDeviceCapability(device, Caps::GK_BACKLIGHT_COLOR) ) {
		/* set backlight color */
		const std::string remoteMethod("SetDeviceBacklightColor");

		try {
			this->pDBus_->initializeRemoteMethodCall(
				this->system_bus_,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
			this->pDBus_->appendStringToRemoteMethodCall(devID);
			uint8_t r, g, b = 0; device.getRGBBytes(r, g, b);
			this->pDBus_->appendUInt8ToRemoteMethodCall(r);
			this->pDBus_->appendUInt8ToRemoteMethodCall(g);
			this->pDBus_->appendUInt8ToRemoteMethodCall(b);

			this->pDBus_->sendRemoteMethodCall();

			try {
				this->pDBus_->waitForRemoteMethodCallReply();

				const bool ret = this->pDBus_->getNextBooleanArgument();
				if( ret ) {
					LOG(VERB)	<< "[" << devID << "] successfully setted device backlight color : "
								<< getHexRGB(r, g, b);
				}
				else {
					LOG(ERROR) << "[" << devID << "] failed to set device backlight color : false";
				}
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			this->pDBus_->abandonRemoteMethodCall();
			LogRemoteCallFailure
		}
	}

	if( this->checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
		/* set macros banks */
		const std::string remoteMethod = "SetDeviceMacrosBank";

		for( const auto & macros_bank_pair : device.getMacrosProfiles() ) {
			const uint8_t current_profile = to_type(macros_bank_pair.first);
			const macros_bank_t & current_bank = macros_bank_pair.second;

			/* test whether this MacrosBank should be sent */
			bool send_it = false;
			for(const auto & macro_pair : current_bank) {
				const macro_t & current_macro = macro_pair.second;
				if( ! current_macro.empty() ) {
					send_it = true;
					break;
				}
			}

			if( ! send_it ) {
				LOG(VERB) << "[" << devID << "] skipping empty MacrosBank " << to_uint(current_profile);
				continue;
			}

			try {
				this->pDBus_->initializeRemoteMethodCall(
					this->system_bus_,
					GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
					GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
					GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
					remoteMethod.c_str()
				);
				this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
				this->pDBus_->appendStringToRemoteMethodCall(devID);
				this->pDBus_->appendUInt8ToRemoteMethodCall(current_profile);
				this->pDBus_->appendMacrosBankToRemoteMethodCall(current_bank);

				this->pDBus_->sendRemoteMethodCall();

				try {
					this->pDBus_->waitForRemoteMethodCallReply();

					const bool ret = this->pDBus_->getNextBooleanArgument();
					if( ret ) {
						LOG(VERB) << "[" << devID << "] successfully setted device MacrosBank " << to_uint(current_profile);
					}
					else {
						LOG(ERROR) << "[" << devID << "] failed to set device MacrosBank " << to_uint(current_profile) << " : false";
					}
				}
				catch (const GLogiKExcept & e) {
					LogRemoteCallGetReplyFailure
				}
			}
			catch (const GKDBusMessageWrongBuild & e) {
				this->pDBus_->abandonRemoteMethodCall();
				LogRemoteCallFailure
			}
		}
	}

	LOG(INFO) << "[" << devID << "] sent device configuration to daemon";
}

void DevicesHandler::setDeviceProperties(const std::string & devID, DeviceProperties & device) {
	/* initialize device properties */
	std::string remoteMethod("GetDeviceProperties");

	try {
		this->pDBus_->initializeRemoteMethodCall(
			this->system_bus_,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
		this->pDBus_->appendStringToRemoteMethodCall(devID);

		this->pDBus_->sendRemoteMethodCall();

		try {
			this->pDBus_->waitForRemoteMethodCallReply();

			const std::string vendor( this->pDBus_->getNextStringArgument() );
			const std::string model( this->pDBus_->getNextStringArgument() );
			const uint64_t caps( this->pDBus_->getNextUInt64Argument() );
			device.setProperties( vendor, model, caps );

#if DEBUGGING_ON
			LOG(DEBUG3) << "[" << devID << "] got 3 properties";
#endif
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		this->pDBus_->abandonRemoteMethodCall();
		LogRemoteCallFailure
	}

	if( this->checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
		/* initialize macro keys */
		remoteMethod = "GetDeviceMacroKeysNames";

		try {
			this->pDBus_->initializeRemoteMethodCall(
				this->system_bus_,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
			this->pDBus_->appendStringToRemoteMethodCall(devID);

			this->pDBus_->sendRemoteMethodCall();

			try {
				this->pDBus_->waitForRemoteMethodCallReply();

				const std::vector<std::string> keys_names( this->pDBus_->getStringsArray() );
				device.initMacrosProfiles(keys_names);
#if DEBUGGING_ON
				LOG(DEBUG3) << "[" << devID << "] initialized " << keys_names.size() << " macro keys";
#endif
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			this->pDBus_->abandonRemoteMethodCall();
			LogRemoteCallFailure
		}
	}

	/* search a configuration file */

#if DEBUGGING_ON
	LOG(DEBUG2) << "[" << devID << "] assigning a configuration file";
#endif
	fs::path directory(this->config_root_directory_);
	directory /= device.getVendor();

	std::set<std::string> already_used;
	{
		for(const auto & dev : this->started_devices_) {
			already_used.insert( dev.second.getConfigFileName() );
		}
		for(const auto & dev : this->stopped_devices_) {
			already_used.insert( dev.second.getConfigFileName() );
		}
	}

	try {
		try {
			/* trying to find an existing configuration file */
			device.setConfigFileName(
				this->pGKfs_->getNextAvailableFileName(already_used, directory, device.getModel(), "cfg", true)
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
		LOG(INFO)	<< "found device [" << devID << "] - "
					<< device.getVendor() << " " << device.getModel();
		LOG(INFO)	<< "[" << devID << "] configuration file found and loaded";

		/* assuming that directory is readable since we just load
		 * the configuration file */
		this->watchDirectory(device, false);

		this->sendDeviceConfigurationToDaemon(devID, device);
	}
	catch ( const GLogiKExcept & e ) { /* should happen only when configuration file not found */
#if DEBUGGING_ON
		LOG(DEBUG1) << e.what();
#endif
		LOG(INFO) << "[" << devID << "] started device : " << device.getVendor()
				<< " " << device.getModel();
		LOG(INFO) << "[" << devID << "] creating default configuration file";

		try {
			/* none found, assign a new configuration file to this device */
			device.setConfigFileName(
				this->pGKfs_->getNextAvailableFileName(already_used, directory, device.getModel(), "cfg")
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
		this->started_devices_.at(devID);
#if DEBUGGING_ON
		LOG(DEBUG1) << "found already started device: [" << devID << "]";
#endif
		return;
	}
	catch (const std::out_of_range& oor) {
		try {
			DeviceProperties & device = this->stopped_devices_.at(devID);
			LOG(INFO) << "starting device: [" << devID << "]";
			this->started_devices_[devID] = device;
			this->stopped_devices_.erase(devID);
		}
		catch (const std::out_of_range& oor) {
#if DEBUGGING_ON
			LOG(DEBUG) << "initializing device: [" << devID << "]";
#endif
			DeviceProperties device;
			/* also load configuration file */
			this->setDeviceProperties(devID, device);
			this->started_devices_[devID] = device;
		}
	}
}

void DevicesHandler::stopDevice(const std::string & devID) {
	try {
		this->stopped_devices_.at(devID);
#if DEBUGGING_ON
		LOG(DEBUG1) << "found already stopped device: [" << devID << "]";
#endif
		return;
	}
	catch (const std::out_of_range& oor) {
		try {
			DeviceProperties & device = this->started_devices_.at(devID);
			LOG(INFO) << "stopping device: [" << devID << "]";
			this->stopped_devices_[devID] = device;
			this->started_devices_.erase(devID);
		}
		catch (const std::out_of_range& oor) {
			LOG(WARNING) << "device [" << devID << "] not found in containers, giving up";
		}
	}
}

void DevicesHandler::unplugDevice(const std::string & devID) {
	try {
		this->stopped_devices_.at(devID);
#if DEBUGGING_ON
		LOG(DEBUG1) << "found already stopped device: [" << devID << "]";
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
		const DeviceProperties & device = this->stopped_devices_.at(devID);

		this->pGKfs_->removeNotifyWatch( device.getWatchDescriptor() );

		this->stopped_devices_.erase(devID);
#if DEBUGGING_ON
		LOG(DEBUG2) << "[" << devID << "] erased device";
#endif

		std::string remoteMethod("DeleteDeviceConfiguration");

		try {
			this->pDBus_->initializeRemoteMethodCall(
				this->system_bus_,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
			this->pDBus_->appendStringToRemoteMethodCall(devID);

			this->pDBus_->sendRemoteMethodCall();

			try {
				this->pDBus_->waitForRemoteMethodCallReply();
				const bool ret = this->pDBus_->getNextBooleanArgument();
				if( ret ) {
#if DEBUGGING_ON
					LOG(DEBUG3) << "[" << devID << "] successfully deleted remote device configuration ";
#endif
				}
				else {
					LOG(ERROR) << "[" << devID << "] failed to delete remote device configuration : false";
				}
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			this->pDBus_->abandonRemoteMethodCall();
			LogRemoteCallFailure
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(WARNING) << "device not found in stopped device container: [" << devID << "]";
	}
}

const bool DevicesHandler::setDeviceMacro(
	const std::string & devID,
	const std::string & keyName,
	const uint8_t profile)
{
	try {
		DeviceProperties & device = this->started_devices_.at(devID);

		if( this->checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
			std::string remoteMethod("GetDeviceMacro");

			try {
				/* getting recorded macro from daemon */
				this->pDBus_->initializeRemoteMethodCall(
					this->system_bus_,
					GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
					GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
					GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
					remoteMethod.c_str()
				);

				this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
				this->pDBus_->appendStringToRemoteMethodCall(devID);
				this->pDBus_->appendStringToRemoteMethodCall(keyName);
				this->pDBus_->appendUInt8ToRemoteMethodCall(profile);

				this->pDBus_->sendRemoteMethodCall();

				try {
					this->pDBus_->waitForRemoteMethodCallReply();

					/* use helper function to get the macro */
					const macro_t macro_array = this->pDBus_->getNextMacroArgument();

					device.setMacro(profile, keyName, macro_array);
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
				this->pDBus_->abandonRemoteMethodCall();
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
	const uint8_t profile)
{
	try {
		DeviceProperties & device = this->started_devices_.at(devID);

		if( this->checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
			device.clearMacro(profile, keyName);
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
	fs::path directory(this->config_root_directory_);
	directory /= device.getVendor();

	try {
		device.setWatchDescriptor( this->pGKfs_->addNotifyDirectoryWatch( directory.string(), check ) );
	}
	catch ( const GLogiKExcept & e ) {
		LOG(WARNING) << e.what();
		LOG(WARNING) << "configuration file monitoring will be disabled";
	}
}

void DevicesHandler::runDeviceMediaEvent(
	const std::string & devID,
	const std::string & key_event)
{
	try {
		DeviceProperties & device = this->started_devices_.at(devID);
		const std::string cmd( device.getMediaCommand(key_event) );
		if( ! cmd.empty() ) {
			std::thread media_event_thread(&DevicesHandler::runCommand, this, cmd);
			media_event_thread.detach();
		}
		else {
#if DEBUGGING_ON
			LOG(DEBUG2) << "empty command media event " << key_event;
#endif
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(WARNING) << "device not found : " << devID;
	}
}

void DevicesHandler::runCommand(const std::string & command)
{
	std::string line;
	bp::ipstream pipe_stream;
	bp::child c(command, bp::std_out > pipe_stream);

	while (pipe_stream && std::getline(pipe_stream, line) && !line.empty()) {
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

	LOG(INFO) << "command run : " << command << " - exit code : " << c.exit_code();
};

} // namespace GLogiK

