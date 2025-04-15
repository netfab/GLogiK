/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2025  Fabrice Delliaux <netbox253@gmail.com>
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

#include "lib/utils/utils.hpp"
#include "lib/shared/glogik.hpp"

#include "client.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

Client::Client(
	const std::string & objectPath,
	DevicesManager* const pDevicesManager)
	:	_sessionState("unknown"),
		_sessionObjectPath(objectPath),
		_check(true),
		_ready(false)
{
	GK_LOG_FUNC

	GKLog(trace, "initializing new client")

	this->initializeDevices(pDevicesManager);
}

Client::~Client()
{
	GK_LOG_FUNC

	GKLog(trace, "destroying client")
}

const std::string & Client::getSessionObjectPath(void) const
{
	return _sessionObjectPath;
}

const std::string & Client::getSessionCurrentState(void) const
{
	return _sessionState;
}

void Client::updateSessionState(const std::string & newState)
{
	_sessionState = newState;
	_check = true;
}

void Client::uncheck(void)
{
	_check = false;
}

const bool Client::isAlive(void) const
{
	return _check;
}

const bool Client::isReady(void) const
{
	return _ready;
}

void Client::setReady(void)
{
	_ready = true;
}

void Client::initializeDevice(
	DevicesManager* const pDevicesManager,
	const std::string & devID)
{
	GK_LOG_FUNC

	try {
		_devices.at(devID);
	}
	catch (const std::out_of_range& oor) {
		GKLog2(trace, devID, " initializing properties")

		clientDevice device;
		device.setProperties(
			pDevicesManager->getDeviceVendor(devID),		/* vendor */
			pDevicesManager->getDeviceProduct(devID),		/* model */
			pDevicesManager->getDeviceName(devID),			/* name */
			pDevicesManager->getDeviceCapabilities(devID)	/* capabilities */
		);

		_devices[devID] = device;
	}
}

const bool Client::deleteDevice(const std::string & devID)
{
	GK_LOG_FUNC

	try {
		_devices.at(devID);
		_devices.erase(devID);
		GKLog2(trace, devID, " deleted device")
		return true;
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
	}

	return false;
}

const bool Client::setDeviceBacklightColor(
	const std::string & devID,
	const uint8_t r,
	const uint8_t g,
	const uint8_t b)
{
	GK_LOG_FUNC

	GKLog2(trace, devID, " setting client backlight color")

	try {
		clientDevice & device = _devices.at(devID);
		device.setRGBBytes(r, g, b);
		return true;
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
	}

	return false;
}

void Client::setDeviceActiveUser(
	const std::string & devID,
	DevicesManager* const pDevicesManager)
{
	GK_LOG_FUNC

	try {
		const clientDevice & device = _devices.at(devID);

		uint8_t r, g, b = 0; device.getRGBBytes(r, g, b);

		GKLog2(trace, devID, " setting active configuration")

		pDevicesManager->setDeviceActiveConfiguration(
			devID, r, g, b, device.getLCDPluginsMask1()
		);
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
	}
}

const bool Client::setDeviceLCDPluginsMask(
	const std::string & devID,
	const uint8_t maskID,
	const uint64_t mask)
{
	GK_LOG_FUNC

	bool ret = false;

	try {
		clientDevice & device = _devices.at(devID);
		device.setLCDPluginsMask(maskID, mask);
		ret = true;
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
	}
	catch (const GLogiKExcept & e) {
		GKSysLogWarning(e.what());
	}

	return ret;
}

void Client::initializeDevices(DevicesManager* const pDevicesManager)
{
	for( const auto & devID : pDevicesManager->getStartedDevices() ) {
		this->initializeDevice(pDevicesManager, devID);
	}

	for( const auto & devID : pDevicesManager->getStoppedDevices() ) {
		this->initializeDevice(pDevicesManager, devID);
	}

	GKLog2(trace, "number of initialized devices configurations : ", _devices.size())
}

} // namespace GLogiK

