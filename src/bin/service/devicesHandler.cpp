/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2017  Fabrice Delliaux <netbox253@gmail.com>
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

#include <boost/archive/archive_exception.hpp>
#include <boost/archive/xml_archive_exception.hpp>
#include <boost/filesystem.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <config.h>

#include "lib/shared/deviceConfigurationFile.h"
#include "lib/utils/utils.h"

#include "warningCheck.h"
#include "devicesHandler.h"

namespace fs = boost::filesystem;

namespace GLogiK
{

DevicesHandler::DevicesHandler() : DBus(nullptr), client_id_("undefined"), buffer_("", std::ios_base::app) {
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

void DevicesHandler::setDBus(GKDBus* pDBus) {
	this->DBus = pDBus;
}

void DevicesHandler::setClientID(const std::string & id) {
	this->client_id_ = id;
}

void DevicesHandler::saveDevicesProperties(void) {
	for( const auto & device_pair : this->devices_ ) {
		//const std::string & devID = device_pair.first;
		const DeviceProperties & device = device_pair.second;

		fs::path current_path(this->config_root_directory_);
		current_path /= device.getVendor();

		try {
			FileSystem::createOwnerDirectory(current_path);
		}
		catch ( const GLogiKExcept & e ) {
			LOG(ERROR) << e.what();
			continue;
		}

		current_path = device.getConfFile();

		try {
#if DEBUGGING_ON
			LOG(DEBUG2) << "trying to open configuration file for writing : " << device.getConfFile();
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

#if DEBUGGING_ON
			LOG(DEBUG3) << "success, closing";
#endif
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
		// must add device.getConfFile() to this->used_conf_files_ then.
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
	try {
		this->DBus->initializeRemoteMethodCall(BusConnection::GKDBUS_SYSTEM, GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			this->DBus_DDM_object_path_, this->DBus_DDM_interface_, "SetDeviceBacklightColor");
		this->DBus->appendStringToRemoteMethodCall(this->client_id_);
		this->DBus->appendStringToRemoteMethodCall(devID);
		this->DBus->appendUInt8ToRemoteMethodCall(device.getBLColor_R());
		this->DBus->appendUInt8ToRemoteMethodCall(device.getBLColor_G());
		this->DBus->appendUInt8ToRemoteMethodCall(device.getBLColor_B());

		this->DBus->sendRemoteMethodCall();

		this->DBus->waitForRemoteMethodCallReply();
		const bool ret = this->DBus->getNextBooleanArgument();
		if( ret ) {
#if DEBUGGING_ON
			LOG(DEBUG3) << "successfully setted device " << devID << " properties";
#endif
		}
		else {
			LOG(ERROR) << "failed to set device " << devID << " properties : false";
		}
	}
	catch (const GLogiKExcept & e) {
		std::string warn(__func__);
		warn += " failure : ";
		warn += e.what();
		WarningCheck::warnOrThrows(warn);
	}
}

void DevicesHandler::setDeviceProperties(const std::string & devID, DeviceProperties & device, const std::string & session_state) {
	unsigned int num = 0;

	try {
		this->DBus->initializeRemoteMethodCall(BusConnection::GKDBUS_SYSTEM, GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			this->DBus_DDM_object_path_, this->DBus_DDM_interface_, "GetDeviceProperties");
		this->DBus->appendStringToRemoteMethodCall(this->client_id_);
		this->DBus->appendStringToRemoteMethodCall(devID);

		this->DBus->sendRemoteMethodCall();

		this->DBus->waitForRemoteMethodCallReply();

		try {
			device.setVendor( this->DBus->getNextStringArgument() );
			num++;
			device.setModel( this->DBus->getNextStringArgument() );
			num++;
		}
		catch (const EmptyContainer & e) {
			// nothing to do here
		}

#if DEBUGGING_ON
		LOG(DEBUG3) << "got " << num << " properties for device " << devID;
#endif
	}
	catch (const GLogiKExcept & e) {
		std::string warn(__func__);
		warn += " failure : ";
		warn += e.what();
		WarningCheck::warnOrThrows(warn);
	}

#if DEBUGGING_ON
	LOG(DEBUG2) << "assigning a configuration file to device " << devID;
#endif
	fs::path directory(this->config_root_directory_);
	directory /= device.getVendor();

	try {
		/* trying to find an existing configuration file */
		device.setConfFile( DeviceConfigurationFile::getNextAvailableNewPath(
			this->used_conf_files_, directory, device.getModel(), true)
		);
#if DEBUGGING_ON
		LOG(DEBUG3) << "found : " << device.getConfFile();
#endif
		/* configuration file loaded */
		this->loadDeviceConfigurationFile(device);
		this->used_conf_files_.push_back( device.getConfFile() );

		if(session_state == "active") {
			this->setDeviceState(devID, device);
		}
	}
	catch ( const GLogiKExcept & e ) {
		try {
			/* none found, assign a new configuration file to this device */
			device.setConfFile( DeviceConfigurationFile::getNextAvailableNewPath(
				this->used_conf_files_, directory, device.getModel())
			);
#if DEBUGGING_ON
			LOG(DEBUG3) << "new one : " << device.getConfFile();
#endif
			this->used_conf_files_.push_back( device.getConfFile() );
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

void DevicesHandler::checkStartedDevice(const std::string & devID, const std::string & session_state) {
	try {
		DeviceProperties & device = this->devices_.at(devID);
		device.check();
		if( device.started() ) {
#if DEBUGGING_ON
			LOG(DEBUG1) << "found already registered started device " << devID;
#endif
			return;
		}
		else {
#if DEBUGGING_ON
			LOG(DEBUG1) << "device " << devID << " state has just been started";
#endif
			device.start();
		}
	}
	catch (const std::out_of_range& oor) {
#if DEBUGGING_ON
		LOG(DEBUG1) << "device " << devID << " not found in container, instantiate it";
#endif
		DeviceProperties device;
		/* also load configuration file */
		this->setDeviceProperties(devID, device, session_state);
		device.start();
		this->devices_[devID] = device;
	}
}

void DevicesHandler::checkStoppedDevice(const std::string & devID) {
	try {
		DeviceProperties & device = this->devices_.at(devID);
		device.check();
		if( device.stopped() ) {
#if DEBUGGING_ON
			LOG(DEBUG1) << "found already registered stopped device " << devID;
#endif
			return;
		}
		else {
#if DEBUGGING_ON
			LOG(DEBUG1) << "device " << devID << " state has just been stopped";
#endif
			device.stop();
		}
	}
	catch (const std::out_of_range& oor) {
#if DEBUGGING_ON
		LOG(DEBUG1) << "device " << devID << " not found in container, instantiate it";
#endif
		DeviceProperties device;
		/* also load configuration file */
		this->setDeviceProperties(devID, device);
		device.stop();
		this->devices_[devID] = device;
	}
}

void DevicesHandler::uncheckThemAll(void) {
	for(auto & device_pair : this->devices_) {
		//const std::string & devID = device_pair.first;
		DeviceProperties & device = device_pair.second;
		device.uncheck();
	}
}

void DevicesHandler::deleteUncheckedDevices(void) {
	std::vector<std::string> to_clean;

	for(const auto & device_pair : this->devices_) {
		const std::string & devID = device_pair.first;
		const DeviceProperties & device = device_pair.second;
		if( ! device.checked() )
			to_clean.push_back(devID);
	}

	unsigned int c = to_clean.size();

	if(c == 0)
		return;

	/* saving first */
	this->saveDevicesProperties();

#if DEBUGGING_ON
	LOG(DEBUG2) << "found " << c << " unplugged devices to clean";
#endif

	for(const auto & devID : to_clean) {
		this->devices_.erase(devID);

		try {
			this->DBus->initializeRemoteMethodCall(BusConnection::GKDBUS_SYSTEM, GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				this->DBus_DCM_object_path_, this->DBus_DCM_interface_, "DeleteDeviceConfiguration");
			this->DBus->appendStringToRemoteMethodCall(this->client_id_);
			this->DBus->appendStringToRemoteMethodCall(devID);

			this->DBus->sendRemoteMethodCall();

			this->DBus->waitForRemoteMethodCallReply();
			const bool ret = this->DBus->getNextBooleanArgument();
			if( ret ) {
#if DEBUGGING_ON
				LOG(DEBUG3) << "successfully deleted client device configuration " << devID;
#endif
			}
			else {
				LOG(ERROR) << "failed to delete client device configuration : false";
			}
		}
		catch (const GLogiKExcept & e) {
			std::string warn("DeleteDeviceConfiguration failure : ");
			warn += e.what();
			WarningCheck::warnOrThrows(warn);
		}
	}

	to_clean.clear();
}

const bool DevicesHandler::updateMacro(
	const std::string & devID,
	const std::string & keyName,
	const uint8_t profile)
{
	try {
		DeviceProperties & device = this->devices_.at(devID);

		/* getting recorded macro from daemon */
		this->DBus->initializeRemoteMethodCall(BusConnection::GKDBUS_SYSTEM, GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			this->DBus_DDM_object_path_, this->DBus_DDM_interface_, "GetMacro");

		this->DBus->appendStringToRemoteMethodCall(this->client_id_);
		this->DBus->appendStringToRemoteMethodCall(devID);
		this->DBus->appendStringToRemoteMethodCall(keyName);
		this->DBus->appendUInt8ToRemoteMethodCall(profile);

		this->DBus->sendRemoteMethodCall();

		this->DBus->waitForRemoteMethodCallReply();

		const macro_t macro_array; // FIXME

		device.updateMacro(profile, keyName, macro_array);
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("device not found in container : ");
		this->buffer_ << devID;
		GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
	}
	catch (const GLogiKExcept & e) {
		GKSysLog(LOG_WARNING, WARNING, e.what());
	}
	return false;
}

void DevicesHandler::clearLoadedDevices(void) {
	this->devices_.clear();
	this->used_conf_files_.clear();
}

} // namespace GLogiK

