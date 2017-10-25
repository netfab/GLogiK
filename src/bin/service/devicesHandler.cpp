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

#include "lib/utils/utils.h"

#include "devicesHandler.h"

namespace GLogiK
{

DevicesHandler::DevicesHandler() : DBus(nullptr) {
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

void DevicesHandler::setDeviceProperties(const std::string & devID, DeviceProperties & device) {
	unsigned int num = 0;

	try {
		this->DBus->initializeRemoteMethodCall(BusConnection::GKDBUS_SYSTEM, GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			this->DBus_DDM_object_path_, this->DBus_DDM_interface_, "GetDeviceProperties");
		this->DBus->appendToRemoteMethodCall(devID);

		this->DBus->sendRemoteMethodCall();

		this->DBus->waitForRemoteMethodCallReply();

		try {
			while( true ) { // FIXME
				device.setName( this->DBus->getNextStringArgument() );
				num++;
			}
		}
		catch (const EmptyContainer & e) {
			LOG(DEBUG3) << "got " << num << " properties for device " << devID;
			// nothing to do here
		}
	}
	catch (const GLogiKExcept & e) {
		std::string warn(__func__);
		warn += " failure : ";
		warn += e.what();
		//this->warnOrThrows(warn);
		throw GLogiKExcept(warn); // FIXME
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

} // namespace GLogiK

