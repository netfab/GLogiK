/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2022  Fabrice Delliaux <netbox253@gmail.com>
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
		GKLog2(trace, devID, " initializing properties")

		DeviceProperties device;
		device.setProperties(
			pDevicesManager->getDeviceVendor(devID),		/* vendor */
			pDevicesManager->getDeviceProduct(devID),		/* model */
			pDevicesManager->getDeviceName(devID),			/* name */
			pDevicesManager->getDeviceCapabilities(devID)	/* capabilities */
		);
		device.initMacrosBanks(
			pDevicesManager->getDeviceMKeysIDArray(devID),
			pDevicesManager->getDeviceGKeysIDArray(devID)
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
		DeviceProperties & device = _devices.at(devID);
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
	DevicesManager* pDevicesManager)
{
	GK_LOG_FUNC

	try {
		const DeviceProperties & device = _devices.at(devID);

		uint8_t r, g, b = 0; device.getRGBBytes(r, g, b);

		GKLog2(trace, devID, " setting active configuration")

		pDevicesManager->setDeviceActiveConfiguration(
			devID, device.getMacrosBanks(),
			r, g, b, device.getLCDPluginsMask1()
		);
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
	}
}

void Client::syncDeviceMacrosBanks(
	const std::string & devID,
	const banksMap_type & macrosBanks)
{
	GK_LOG_FUNC

	GKLog2(trace, devID, " synchonizing macros banks")

	try {
		DeviceProperties & device = _devices.at(devID);
		device.setMacrosBanks(macrosBanks);
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
	}
}

const macro_type & Client::getDeviceMacro(
	const std::string & devID,
	const MKeysID bankID,
	const GKeysID keyID)
{
	GK_LOG_FUNC

	try {
		DeviceProperties & device = _devices.at(devID);
		return device.getMacro(bankID, keyID);
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
	}

	return MacrosBanks::emptyMacro;
}

const bool Client::setDeviceMacrosBank(
	const std::string & devID,
	const MKeysID bankID,
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
				GKSysLogWarning(e.what());
			}
		}
	}
	catch (const std::out_of_range& oor) {
		ret = false;
		GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
	}
	catch (const GLogiKExcept & e) {
		ret = false;
		GKSysLogWarning(e.what());
	}

	return ret;
}

const bool Client::resetDeviceMacrosBank(
	const std::string & devID,
	const MKeysID bankID)
{
	GK_LOG_FUNC

	bool ret = false;

	try {
		DeviceProperties & device = _devices.at(devID);
		device.resetMacrosBank(bankID);
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
		GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
	}
	catch (const GLogiKExcept & e) {
		GKSysLogWarning(e.what());
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

	GKLog2(trace, "number of initialized devices configurations : ", _devices.size())
}

} // namespace GLogiK

