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

#include <stdexcept>
#include <fstream>

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

DevicesHandler::DevicesHandler() : DBus(nullptr), buffer_("", std::ios_base::app) {
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

/*
void ServiceDBusHandler::loadDevicesProperties(void) {
	try {
		std::ifstream ifs;
		ifs.exceptions(std::ifstream::failbit|std::ifstream::badbit);
		ifs.open(this->cfgfile_fullpath_);

		LOG(DEBUG) << "configuration file successfully opened for reading";

		boost::archive::text_iarchive input_archive(ifs);
		input_archive >> this->devices_;
	}
	catch (const std::ofstream::failure & e) {
		this->buffer_.str("fail to open configuration file : ");
		this->buffer_ << this->cfgfile_fullpath_ << " : " << e.what();
		LOG(ERROR) << this->buffer_.str();
		GK_ERR << this->buffer_.str() << "\n";
	}
	*
	 * catch std::ios_base::failure on buggy compilers
	 * should be fixed with gcc >= 7.0
	 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145
	 *
	catch( const std::exception & e ) {
		this->buffer_.str("(buggy exception) fail to open configuration file : ");
		this->buffer_ << e.what();
		LOG(ERROR) << this->buffer_.str();
		GK_ERR << this->buffer_.str() << "\n";
	}
}
*/

void DevicesHandler::saveDevicesProperties(void) {
	for( const auto & device : this->devices_ ) {
		//const std::string & devID = device.first;
		const DeviceProperties & props = device.second;

		fs::path current_path(this->config_root_directory_);
		current_path /= props.vendor_;

		try {
			FileSystem::createOwnerDirectory(current_path);
		}
		catch ( const GLogiKExcept & e ) {
			LOG(ERROR) << e.what();
			GK_ERR << e.what() << "\n";
			continue;
		}

		current_path = props.conf_file_;

		try {
			LOG(DEBUG2) << "trying to open configuration file for writing : " << props.conf_file_;

			std::ofstream ofs;
			ofs.exceptions(std::ofstream::failbit|std::ofstream::badbit);
			ofs.open(props.conf_file_, std::ofstream::out|std::ofstream::trunc);

			fs::permissions(current_path, fs::owner_read|fs::owner_write|fs::group_read|fs::others_read);

			LOG(DEBUG3) << "opened";
			boost::archive::text_oarchive output_archive(ofs);
			output_archive << props;
			LOG(DEBUG3) << "success, closing";
		}
		catch (const std::ofstream::failure & e) {
			this->buffer_.str("fail to open configuration file : ");
			this->buffer_ << e.what();
			LOG(ERROR) << this->buffer_.str();
			GK_ERR << this->buffer_.str() << "\n";
		}
		catch (const fs::filesystem_error & e) {
			this->buffer_.str("set permissions failure on configuration file : ");
			this->buffer_ << e.what();
			LOG(ERROR) << this->buffer_.str();
			GK_ERR << this->buffer_.str() << "\n";
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
			GK_ERR << this->buffer_.str() << "\n";
		}
	}
}

void DevicesHandler::setDeviceProperties(const std::string & devID, DeviceProperties & device) {
	unsigned int num = 0;

	try {
		this->DBus->initializeRemoteMethodCall(BusConnection::GKDBUS_SYSTEM, GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			this->DBus_DDM_object_path_, this->DBus_DDM_interface_, "GetDeviceProperties");
		this->DBus->appendToRemoteMethodCall(devID);

		this->DBus->sendRemoteMethodCall();

		this->DBus->waitForRemoteMethodCallReply();

		try {
			device.vendor_ = this->DBus->getNextStringArgument();
			num++;
			device.model_  = this->DBus->getNextStringArgument();
			num++;
		}
		catch (const EmptyContainer & e) {
			// nothing to do here
		}

		LOG(DEBUG3) << "got " << num << " properties for device " << devID;
	}
	catch (const GLogiKExcept & e) {
		std::string warn(__func__);
		warn += " failure : ";
		warn += e.what();
		WarningCheck::warnOrThrows(warn);
	}

	LOG(DEBUG2) << "assigning a configuration file to device " << devID;
	fs::path directory(this->config_root_directory_);
	directory /= device.vendor_;

	try {
		/* trying to find an existing configuration file */
		device.conf_file_ = DeviceConfigurationFile::getNextAvailableNewPath(this->used_conf_files_, directory, device.model_, true);
		this->used_conf_files_.push_back(device.conf_file_);
		LOG(DEBUG3) << "found : " << device.conf_file_;
	}
	catch ( const GLogiKExcept & e ) {
		try {
			/* none found, assign a new configuration file to this device */
			device.conf_file_ = DeviceConfigurationFile::getNextAvailableNewPath(this->used_conf_files_, directory, device.model_);
			this->used_conf_files_.push_back(device.conf_file_);
			LOG(DEBUG3) << "new one : " << device.conf_file_;
		}
		catch ( const GLogiKExcept & e ) {
			LOG(ERROR) << e.what();
			GK_ERR << e.what() << "\n";
		}
	}
}

void DevicesHandler::checkStartedDevice(const std::string & devID) {
	try {
		DeviceProperties & device = this->devices_.at(devID);
		if( device.started() ) {
			LOG(DEBUG1) << "found already registered started device " << devID;
			return;
		}
		else {
			LOG(DEBUG1) << "device " << devID << " state has just been started";
			/* TODO should load device parameters */
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(DEBUG1) << "device " << devID << " not found in container, instantiate it";
		DeviceProperties device;
		this->setDeviceProperties(devID, device);
		device.start();
		this->devices_[devID] = device;
		/* TODO should load device parameters */
	}
}

void DevicesHandler::checkStoppedDevice(const std::string & devID) {
	try {
		DeviceProperties & device = this->devices_.at(devID);
		if( device.stopped() ) {
			LOG(DEBUG1) << "found already registered stopped device " << devID;
			return;
		}
		else {
			LOG(DEBUG1) << "device " << devID << " state has just been stopped";
			/* TODO should load device parameters */
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(DEBUG1) << "device " << devID << " not found in container, instantiate it";
		DeviceProperties device;
		this->setDeviceProperties(devID, device);
		device.stop();
		this->devices_[devID] = device;
		/* TODO should load device parameters */
	}
}


void DevicesHandler::clearLoadedDevices(void) {
	this->devices_.clear();
	this->used_conf_files_.clear();
}

} // namespace GLogiK

