/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
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
	DevicesManager* pDevicesManager)
	:	_sessionState("unknown"),
		_sessionObjectPath(objectPath),
		_check(true),
		_ready(false)
{
	GK_LOG_FUNC

#if DEBUGGING_ON
	LOG(DEBUG2) << "initializing new client";
#endif

	this->initializeDevices(pDevicesManager);

#if DEBUGGING_ON
	LOG(DEBUG3) << "initialized " << _devices.size() << " client devices configurations";
#endif
}

Client::~Client()
{
	GK_LOG_FUNC

#if DEBUGGING_ON
	LOG(DEBUG2) << "destroying client";
#endif
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

void Client::toggleClientReadyPropertie(void)
{
	_ready = ! _ready;
}

void Client::initializeDevice(
	DevicesManager* pDevicesManager,
	const std::string & devID)
{
	GK_LOG_FUNC

	try {
		_devices.at(devID);
	}
	catch (const std::out_of_range& oor) {
#if DEBUGGING_ON
		LOG(DEBUG3) << "initializing device " << devID << " properties";
#endif
		DeviceProperties device;
		device.setProperties(
			pDevicesManager->getDeviceVendor(devID),		/* vendor */
			pDevicesManager->getDeviceProduct(devID),		/* model */
			pDevicesManager->getDeviceName(devID),			/* name */
			pDevicesManager->getDeviceCapabilities(devID)	/* capabilities */
		);
		device.initMacrosBanks( pDevicesManager->getDeviceMacroKeysNames(devID) );

		_devices[devID] = device;
	}
}

const bool Client::deleteDevice(const std::string & devID)
{
	GK_LOG_FUNC

	try {
		_devices.at(devID);
		_devices.erase(devID);
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

const bool Client::setDeviceBacklightColor(
	const std::string & devID,
	const uint8_t r,
	const uint8_t g,
	const uint8_t b)
{
	GK_LOG_FUNC

#if DEBUGGING_ON
	LOG(DEBUG3) << "setting client backlight color for device " << devID;
#endif
	try {
		DeviceProperties & device = _devices.at(devID);
		device.setRGBBytes(r, g, b);
		return true;
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}

	return false;
}

void Client::setDeviceActiveUser(
	const std::string & devID,
	DevicesManager* pDevicesManager)
{
	GK_LOG_FUNC

	try {
		const DeviceProperties & device = _devices.at(devID);

		uint8_t r, g, b = 0; device.getRGBBytes(r, g, b);

#if DEBUGGING_ON
		LOG(DEBUG1) << "setting active configuration for device " << devID;
#endif
		pDevicesManager->setDeviceActiveConfiguration(
			devID, device.getMacrosBanks(),
			r, g, b, device.getLCDPluginsMask1()
		);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
}

void Client::syncDeviceMacrosBanks(
	const std::string & devID,
	const banksMap_type & macrosBanks)
{
	GK_LOG_FUNC

#if DEBUGGING_ON
	LOG(DEBUG3) << "synchonizing macros banks for device " << devID;
#endif
	try {
		DeviceProperties & device = _devices.at(devID);
		device.setMacrosBanks(macrosBanks);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
}

const macro_type & Client::getDeviceMacro(
	const std::string & devID,
	const std::string & keyName,
	const uint8_t bankID)
{
	GK_LOG_FUNC

	try {
		DeviceProperties & device = _devices.at(devID);
		return device.getMacro(bankID, keyName);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}

	return MacrosBanks::emptyMacro;
}

const bool Client::setDeviceMacrosBank(
	const std::string & devID,
	const uint8_t bankID,
	const mBank_type & bank)
{
	GK_LOG_FUNC

	bool ret = true;

	try {
		DeviceProperties & device = _devices.at(devID);
		device.resetMacrosBank(bankID);
		for(const auto & keyMacroPair : bank) {
			try {
				device.setMacro(bankID, keyMacroPair.first, keyMacroPair.second);
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
	catch (const GLogiKExcept & e) {
		ret = false;
		GKSysLog(LOG_WARNING, WARNING, e.what());
	}

	return ret;
}

const bool Client::resetDeviceMacrosBank(
	const std::string & devID,
	const uint8_t bankID)
{
	GK_LOG_FUNC

	bool ret = false;

	try {
		DeviceProperties & device = _devices.at(devID);
		device.resetMacrosBank(bankID);
		ret = true;
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
	catch (const GLogiKExcept & e) {
		GKSysLog(LOG_WARNING, WARNING, e.what());
	}

	return ret;
}

const bool Client::setDeviceLCDPluginsMask(
	const std::string & devID,
	const uint8_t maskID,
	const uint64_t mask)
{
	GK_LOG_FUNC

	bool ret = false;

	try {
		DeviceProperties & device = _devices.at(devID);
		device.setLCDPluginsMask(maskID, mask);
		ret = true;
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
	catch (const GLogiKExcept & e) {
		GKSysLog(LOG_WARNING, WARNING, e.what());
	}

	return ret;
}

void Client::initializeDevices(DevicesManager* pDevicesManager)
{
	for( const auto & devID : pDevicesManager->getStartedDevices() ) {
		this->initializeDevice(pDevicesManager, devID);
	}

	for( const auto & devID : pDevicesManager->getStoppedDevices() ) {
		this->initializeDevice(pDevicesManager, devID);
	}
}

} // namespace GLogiK

