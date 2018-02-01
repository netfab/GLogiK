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

#include <stdexcept>
#include <iostream>

#include "lib/utils/utils.h"
#include "lib/shared/glogik.h"

#include "client.h"

namespace GLogiK
{

using namespace NSGKUtils;

Client::Client(
	const std::string & object_path,
	DevicesManager* dev_manager
)	:	session_state_("unknown"),
		client_session_object_path_(object_path),
		check_(true),
		ready_(false)
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "initializing new client";
#endif

	for( const auto & devID : dev_manager->getStartedDevices() ) {
		this->getDeviceProperties(devID, dev_manager);
	}

	for( const auto & devID : dev_manager->getStoppedDevices() ) {
		this->getDeviceProperties(devID, dev_manager);
	}

#if DEBUGGING_ON
	LOG(DEBUG3) << "initialized " << this->devices_.size() << " client devices configurations";
#endif
}

Client::~Client() {
#if DEBUGGING_ON
	LOG(DEBUG2) << "destroying client";
#endif
}

const std::string & Client::getSessionObjectPath(void) const {
	return this->client_session_object_path_;
}

const std::string & Client::getSessionCurrentState(void) const {
	return this->session_state_;
}

void Client::updateSessionState(const std::string & new_state) {
	this->session_state_ = new_state;
	this->check_ = true;
}

void Client::uncheck(void) {
	this->check_ = false;
}

const bool Client::isAlive(void) const {
	return this->check_;
}

const bool Client::isReady(void) const {
	return this->ready_;
}

void Client::toggleClientReadyPropertie(void) {
	this->ready_ = ! this->ready_;
}

const std::vector<std::string> Client::getDeviceProperties(const std::string & devID, DevicesManager* dev_manager) {
	const std::vector<std::string> properties = dev_manager->getDeviceProperties(devID);

	try {
		this->devices_.at(devID);
	}
	catch (const std::out_of_range& oor) {
#if DEBUGGING_ON
		LOG(DEBUG3) << "initializing device " << devID << " properties";
#endif
		DeviceProperties device;
		device.setVendor(properties[0]); /* FIXME */
		device.setModel(properties[1]);

		device.initMacrosProfiles( dev_manager->getDeviceMacroKeysNames(devID) );

		this->devices_[devID] = device;
	}

#if DEBUGGING_ON
	LOG(DEBUG3) << "number of initialized devices : " << this->devices_.size();
#endif

	return properties;
}

const bool Client::deleteDevice(const std::string & devID) {
	try {
		this->devices_.at(devID);
		this->devices_.erase(devID);
#if DEBUGGING_ON
		LOG(DEBUG3) << "deleted device : " << devID;
#endif
		return true;
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}

	return false;
}

const bool Client::setDeviceBacklightColor(const std::string & devID,
			const uint8_t r, const uint8_t g, const uint8_t b)
{
#if DEBUGGING_ON
	LOG(DEBUG3) << "setting backlight color for device " << devID;
#endif
	try {
		DeviceProperties & device = this->devices_.at(devID);
		device.setBLColor_R(r);
		device.setBLColor_G(g);
		device.setBLColor_B(b);
		return true;
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}

	return false;
}

void Client::setAllDevicesBacklightColors(DevicesManager* dev_manager) {
#if DEBUGGING_ON
	LOG(DEBUG3) << "setting backlight color for all started devices";
#endif
	for( const auto & devID : dev_manager->getStartedDevices() ) {
		try {
			const DeviceProperties & device = this->devices_.at(devID);
			const uint8_t r = device.getBLColor_R();
			const uint8_t g = device.getBLColor_G();
			const uint8_t b = device.getBLColor_B();
			dev_manager->setDeviceBacklightColor(devID, r, g, b);
		}
		catch (const std::out_of_range& oor) {
			GKSysLog_UnknownDevice
		}
	}
}

void Client::setAllDevicesMacrosProfiles(DevicesManager* dev_manager) {
#if DEBUGGING_ON
	LOG(DEBUG3) << "setting macros profiles for all started devices";
#endif
	for( const auto & devID : dev_manager->getStartedDevices() ) {
		try {
			const DeviceProperties & device = this->devices_.at(devID);
			dev_manager->setDeviceMacrosProfiles( devID, device.getMacrosProfiles() );
		}
		catch (const std::out_of_range& oor) {
			GKSysLog_UnknownDevice
		}
	}
}

void Client::syncDeviceMacrosProfiles(const std::string & devID, const macros_map_t & macros_profiles) {
#if DEBUGGING_ON
	LOG(DEBUG3) << "synchonizing macros profiles for device " << devID;
#endif
	try {
		DeviceProperties & device = this->devices_.at(devID);
		device.setMacrosProfiles(macros_profiles);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
}

const macro_t & Client::getDeviceMacro(
	const std::string & devID,
	const std::string & keyName,
	const uint8_t profile
) {
	try {
		DeviceProperties & device = this->devices_.at(devID);
		return device.getMacro(profile, keyName);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}

	return MacrosBanks::empty_macro_;
}

const bool Client::setDeviceMacrosBank(
	const std::string & devID,
	const uint8_t profile,
	const macros_bank_t & bank
) {
	bool ret = true;

	try {
		DeviceProperties & device = this->devices_.at(devID);
		for(const auto & macro_pair : bank) {
			try {
				device.setMacro(profile, macro_pair.first, macro_pair.second);
			}
			catch (const GLogiKExcept & e) {
				ret = false;
				GKSysLog(LOG_WARNING, WARNING, e.what());
			}
		}
	}
	catch (const std::out_of_range& oor) {
		ret = false;
		GKSysLog_UnknownDevice
	}

	return ret;
}

} // namespace GLogiK

