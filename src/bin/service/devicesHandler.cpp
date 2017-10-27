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

void DevicesHandler::saveDevicesProperties(const std::string & config_directory) {
	for( const auto & device : this->devices_ ) {
		const std::string & devID = device.first;
		const DeviceProperties & props = device.second;

		std::string current_dir(config_directory);
		current_dir += "/";
		current_dir += props.vendor_;

		bool dir_created = false;

		try {
			fs::path path(current_dir);
			dir_created = create_directory(path);
			fs::permissions(path, fs::owner_all);
		}
		catch (const fs::filesystem_error & e) {
			this->buffer_.str("configuration directory creation or set permissions failure : ");
			this->buffer_ << current_dir << " : " << e.what();
			LOG(ERROR) << this->buffer_.str();
			GK_ERR << this->buffer_.str() << "\n";
			continue;
		}

		if( ! dir_created ) {
			LOG(DEBUG) << "configuration directory not created because it seems already exists";
		}
		else {
			LOG(DEBUG) << "configuration directory created";
		}

		current_dir += "/";
		current_dir += devID;
		current_dir += ".cfg";

		try {
			std::ofstream ofs;
			ofs.exceptions(std::ofstream::failbit|std::ofstream::badbit);
			ofs.open(current_dir, std::ofstream::out|std::ofstream::trunc);

			fs::path path(current_dir);
			fs::permissions(path, fs::owner_read|fs::owner_write|fs::group_read|fs::others_read);

			LOG(DEBUG) << "configuration file successfully opened for writing";

			boost::archive::text_oarchive output_archive(ofs);
			output_archive << props;
		}
		catch (const std::ofstream::failure & e) {
			this->buffer_.str("fail to open configuration file : ");
			this->buffer_ << current_dir << " : " << e.what();
			LOG(ERROR) << this->buffer_.str();
			GK_ERR << this->buffer_.str() << "\n";
		}
		catch (const fs::filesystem_error & e) {
			this->buffer_.str("set permissions failure on configuration file : ");
			this->buffer_ << current_dir << " : " << e.what();
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
}

} // namespace GLogiK

