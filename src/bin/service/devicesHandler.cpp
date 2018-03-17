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

#include <exception>
#include <stdexcept>
#include <fstream>
#include <set>

#include <boost/archive/archive_exception.hpp>
#include <boost/archive/xml_archive_exception.hpp>
#include <boost/filesystem.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "lib/shared/deviceConfigurationFile.h"

#include "devicesHandler.h"

namespace fs = boost::filesystem;

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

void DevicesHandler::saveDeviceProperties(
	const std::string & devID,
	const DeviceProperties & device )
{
	try {
		fs::path current_path(this->config_root_directory_);
		current_path /= device.getVendor();

		FileSystem::createOwnerDirectory(current_path);

		current_path = device.getConfFile();

		try {
#if DEBUGGING_ON
			LOG(DEBUG2) << "[" << devID << "] trying to open configuration file for writing : " << device.getConfFile();
#endif

			std::ofstream ofs;
			ofs.exceptions(std::ofstream::failbit|std::ofstream::badbit);
			ofs.open(device.getConfFile(), std::ofstream::out|std::ofstream::trunc);

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
#if DEBUGGING_ON
	LOG(DEBUG2) << "loading device configuration file " << device.getConfFile();
#endif
	try {
		std::ifstream ifs;
		ifs.exceptions(std::ifstream::badbit);
		ifs.open(device.getConfFile());
#if DEBUGGING_ON
		LOG(DEBUG2) << "configuration file successfully opened for reading";
#endif

		{
			DeviceProperties new_device;
			boost::archive::text_iarchive input_archive(ifs);
			input_archive >> new_device;

			/* do we need to replace vendor and model ? */
			//device.setVendor( new_device.getVendor() );
			//device.setModel( new_device.getModel() );
			device.setMacrosProfiles( new_device.getMacrosProfiles() );
			device.setBLColor_R( new_device.getBLColor_R() );
			device.setBLColor_G( new_device.getBLColor_G() );
			device.setBLColor_B( new_device.getBLColor_B() );
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

void DevicesHandler::setDeviceState(const std::string & devID, const DeviceProperties & device) {
	/* set backlight color */
	std::string remoteMethod("SetDeviceBacklightColor");

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
		this->pDBus_->appendUInt8ToRemoteMethodCall(device.getBLColor_R());
		this->pDBus_->appendUInt8ToRemoteMethodCall(device.getBLColor_G());
		this->pDBus_->appendUInt8ToRemoteMethodCall(device.getBLColor_B());

		this->pDBus_->sendRemoteMethodCall();

		try {
			this->pDBus_->waitForRemoteMethodCallReply();

			const bool ret = this->pDBus_->getNextBooleanArgument();
			if( ret ) {
				const uint8_t & r = device.getBLColor_R();
				const uint8_t & g = device.getBLColor_G();
				const uint8_t & b = device.getBLColor_B();
				LOG(INFO) << "[" << devID
							<< "] successfully setted device backlight color : "
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


	/* set macros banks */
	remoteMethod = "SetDeviceMacrosBank";

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
			LOG(INFO) << "[" << devID << "] skipping empty MacrosBank " << to_uint(current_profile);
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
					LOG(INFO) << "[" << devID << "] successfully setted device MacrosBank " << to_uint(current_profile);
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

void DevicesHandler::setDeviceProperties(const std::string & devID, DeviceProperties & device) {
	unsigned int num = 0;

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

			device.setVendor( this->pDBus_->getNextStringArgument() );
			num++;
			device.setModel( this->pDBus_->getNextStringArgument() );
			num++;

#if DEBUGGING_ON
			LOG(DEBUG3) << "[" << devID << "] got " << num << " properties";
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

	/* search a configuration file */

#if DEBUGGING_ON
	LOG(DEBUG2) << "[" << devID << "] assigning a configuration file";
#endif
	fs::path directory(this->config_root_directory_);
	directory /= device.getVendor();

	std::set<std::string> used_conf_files;
	{
		for(const auto & dev : this->started_devices_) {
			used_conf_files.insert( dev.second.getConfFile() );
		}
		for(const auto & dev : this->stopped_devices_) {
			used_conf_files.insert( dev.second.getConfFile() );
		}
	}

	try {
		/* trying to find an existing configuration file */
		device.setConfFile( DeviceConfigurationFile::getNextAvailableNewPath(
			used_conf_files, directory, device.getModel(), true)
		);
#if DEBUGGING_ON
		LOG(DEBUG3) << "found : " << device.getConfFile();
#endif
		/* configuration file loaded */
		this->loadDeviceConfigurationFile(device);
		device.setWatchDescriptor( this->pGKfs_->notifyWatchFile( device.getConfFile() ) );

		LOG(INFO)	<< "found device [" << devID << "] - "
					<< device.getVendor() << " " << device.getModel();
		LOG(INFO)	<< "[" << devID << "] configuration file found and loaded";

		/* send device configuration to daemon */
		this->setDeviceState(devID, device);
	}
	catch ( const GLogiKExcept & e ) {
		try {
			/* none found, assign a new configuration file to this device */
			device.setConfFile( DeviceConfigurationFile::getNextAvailableNewPath(
				used_conf_files, directory, device.getModel())
			);
#if DEBUGGING_ON
			LOG(DEBUG3) << "new one : " << device.getConfFile();
#endif
			device.setWatchDescriptor( this->pGKfs_->notifyWatchFile( device.getConfFile() ) );
		}
		catch ( const GLogiKExcept & e ) {
			LOG(ERROR) << e.what();
		}
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

		this->pGKfs_->notifyRemoveFile( device.getWatchDescriptor() );

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
				this->saveDeviceProperties(devID, device);
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

		device.clearMacro(profile, keyName);
		this->saveDeviceProperties(devID, device);
		return true;
	}
	catch (const std::out_of_range& oor) {
		LOG(WARNING) << "device not found : " << devID;
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << "clear macro failure: " << e.what();
	}
	return false;
}

} // namespace GLogiK

