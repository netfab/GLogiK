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

DevicesHandler::DevicesHandler() {
	LOG(DEBUG) << "Devices Handler initialization";
}

DevicesHandler::~DevicesHandler() {
	LOG(DEBUG) << "Devices Handler destruction";
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
		device.start();
		this->devices_[devID] = device;
		/* TODO should load device parameters */
	}
}

} // namespace GLogiK

