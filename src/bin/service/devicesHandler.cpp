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

void DevicesHandler::saveDevicesProperties(void) {
	for( const auto & device_pair : this->devices_ ) {
		//const std::string & devID = device_pair.first;
		const DeviceProperties & device = device_pair.second;

		fs::path current_path(this->config_root_directory_);
		current_path /= device.vendor_;

		try {
			FileSystem::createOwnerDirectory(current_path);
		}
		catch ( const GLogiKExcept & e ) {
			LOG(ERROR) << e.what();
			GK_ERR << e.what() << "\n";
			continue;
		}

		current_path = device.conf_file_;

		try {
			LOG(DEBUG2) << "trying to open configuration file for writing : " << device.conf_file_;

			std::ofstream ofs;
			ofs.exceptions(std::ofstream::failbit|std::ofstream::badbit);
			ofs.open(device.conf_file_, std::ofstream::out|std::ofstream::trunc);

			fs::permissions(current_path, fs::owner_read|fs::owner_write|fs::group_read|fs::others_read);
			LOG(DEBUG3) << "opened";

			{
				boost::archive::text_oarchive output_archive(ofs);
				output_archive << device;
			}

			LOG(DEBUG3) << "success, closing";
			ofs.close();
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
		catch(const boost::archive::archive_exception & e) {
			this->buffer_.str("boost::archive exception : ");
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

void DevicesHandler::loadDeviceConfigurationFile(DeviceProperties & device) {
	LOG(DEBUG2) << "loading device configuration file " << device.conf_file_;
	try {
		std::ifstream ifs;
		ifs.exceptions(std::ifstream::badbit);
		ifs.open(device.conf_file_);
		LOG(DEBUG2) << "configuration file successfully opened for reading";

		{
			DeviceProperties new_device;
			boost::archive::text_iarchive input_archive(ifs);
			input_archive >> new_device;

			/* do we need to replace vendor and model ? */
			//device.vendor_ = new_device.vendor_;
			//device.model_ = new_device.model_;
			device.setMacros( new_device.getMacros() );
		}

		LOG(DEBUG3) << "success, closing";
		ifs.close();
	}
	catch (const std::ifstream::failure & e) {
		this->buffer_.str("fail to open configuration file : ");
		this->buffer_ << e.what();
		LOG(ERROR) << this->buffer_.str();
		GK_ERR << this->buffer_.str() << "\n";
	}
	catch(const boost::archive::archive_exception & e) {
		std::string err("load: ");
		err += e.what();
		LOG(ERROR) << err;
		GK_ERR << err << "\n";
		// TODO throw GLogiKExcept to create new configuration
		// file and avoid overwriting on close ?
		// must add device.conf_file_ to this->used_conf_files_ then.
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
		this->loadDeviceConfigurationFile(device);
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

void DevicesHandler::checkStartedDevice(const std::string & devID) {
	try {
		DeviceProperties & device = this->devices_.at(devID);
		if( device.started() ) {
			LOG(DEBUG1) << "found already registered started device " << devID;
			return;
		}
		else {
			LOG(DEBUG1) << "device " << devID << " state has just been started";
			device.start();
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(DEBUG1) << "device " << devID << " not found in container, instantiate it";
		DeviceProperties device;
		/* also load configuration file */
		this->setDeviceProperties(devID, device);
		device.start();
		this->devices_[devID] = device;
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
			device.stop();
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(DEBUG1) << "device " << devID << " not found in container, instantiate it";
		DeviceProperties device;
		/* also load configuration file */
		this->setDeviceProperties(devID, device);
		device.stop();
		this->devices_[devID] = device;
	}
}


void DevicesHandler::clearLoadedDevices(void) {
	this->devices_.clear();
	this->used_conf_files_.clear();
}

} // namespace GLogiK

