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

#include "lib/utils/utils.hpp"

#include "sleepInhibition.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

SleepInhibition::SleepInhibition(void)
	:	_pDBus(nullptr),
		_pDevicesManager(nullptr),
		_sessionFramework(SessionFramework::FW_UNKNOWN),
		_delayLockPID(-1)
{
}

SleepInhibition::~SleepInhibition(void)
{
}

void SleepInhibition::startSleepInhibition(
	NSGKDBus::GKDBus* const pDBus,
	DevicesManager* const pDevicesManager)
{
	GK_LOG_FUNC

	_pDBus = pDBus;
	_pDevicesManager = pDevicesManager;

	this->inhibitSleepState();

	switch(_sessionFramework) {
		/* logind */
		case SessionFramework::FW_LOGIND:
			_pDBus->NSGKDBus::Callback<SIGb2v>::receiveSignal(
				_systemBus,
				LOGIND_DBUS_BUS_CONNECTION_NAME,
				LOGIND_MANAGER_DBUS_OBJECT_PATH,
				LOGIND_MANAGER_DBUS_INTERFACE,
				"PrepareForSleep",
				{ {"b", "", "in", "mode"} },
				std::bind(&SleepInhibition::handleSleepEvent, this, std::placeholders::_1)
			);
			break;
		default:
			LOG(warning) << "unknown session tracker";
			break;
	}
}

void SleepInhibition::stopSleepInhibition(void) noexcept
{
	GK_LOG_FUNC

	switch(_sessionFramework) {
		/* logind */
		case SessionFramework::FW_LOGIND:
			_pDBus->removeSignalsInterface(_systemBus,
				LOGIND_DBUS_BUS_CONNECTION_NAME,
				LOGIND_MANAGER_DBUS_OBJECT_PATH,
				LOGIND_MANAGER_DBUS_INTERFACE);
			break;
		default:
			LOG(warning) << "unknown session tracker";
			break;
	}

	this->releaseDelayLock();
}

void SleepInhibition::inhibitSleepState(void)
{
	GK_LOG_FUNC

	const std::string remoteMethod("Inhibit");
	try {
		_pDBus->initializeRemoteMethodCall(
			_systemBus,
			LOGIND_DBUS_BUS_CONNECTION_NAME,
			LOGIND_MANAGER_DBUS_OBJECT_PATH,
			LOGIND_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);

		_pDBus->appendStringToRemoteMethodCall("sleep"); /* What (lock type) */
		_pDBus->appendStringToRemoteMethodCall("GLogiK Daemon"); /* Who */
		_pDBus->appendStringToRemoteMethodCall("Release USB Devices"); /* Why */
		_pDBus->appendStringToRemoteMethodCall("delay"); /* mode */

		_pDBus->sendRemoteMethodCall();

		try {
			_pDBus->waitForRemoteMethodCallReply();

			_delayLockPID = _pDBus->getNextInt32Argument();

			_sessionFramework = SessionFramework::FW_LOGIND;
			GKLog2(trace, "got pid for delay lock from logind: ", static_cast<int>(_delayLockPID))
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_pDBus->abandonRemoteMethodCall();
		LogRemoteCallFailure
	}

	switch(_sessionFramework) {
		/* logind */
		case SessionFramework::FW_LOGIND:
			LOG(info) << "successfully contacted logind";
			break;
		default:
			LOG(warning) << "unknown session tracker";
			break;
	}
}

void SleepInhibition::releaseDelayLock(void)
{
	GK_LOG_FUNC

	GKLog(trace, "release delay lock")
	if( _delayLockPID != -1 ) {
		const int ret = close(_delayLockPID);
		const int close_errno = errno;

		if(ret == -1) {
			LOG(error)	<< "delay lock close : " << getErrnoString(close_errno);
		}

		_delayLockPID = -1;
		GKLog(trace, "released")
	}
}

void SleepInhibition::handleSleepEvent(const bool mode)
{
	GK_LOG_FUNC

	if(mode) {
		GKLog(trace, "going to sleep, stopping devices")
		_pDevicesManager->stopInitializedDevices();
		this->releaseDelayLock();
	}
	else {
		GKLog(trace, "resuming from sleep, starting devices")
		this->inhibitSleepState();
		/* don't start devices too early after resuming */
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		_pDevicesManager->startSleepingDevices();
	}
}

} // namespace GLogiK
