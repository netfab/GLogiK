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

#include <sstream>
#include <new>
#include <functional>
#include <stdexcept>
#include <csignal>

#include "lib/shared/glogik.hpp"

#include "DBusHandler.hpp"

#include "include/MBank.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

bool DBusHandler::WantToExit = false;

DBusHandler::DBusHandler(
	pid_t pid,
	NSGKUtils::FileSystem* pGKfs,
	GKDepsMap_type* dependencies)
	:	_clientID("undefined"),
		_daemonVersion("unknown"),
		_CURRENT_SESSION_DBUS_OBJECT_PATH(""),
		_sessionState(""),
		_pDepsMap(dependencies),
		_sessionFramework(SessionFramework::FW_UNKNOWN),
		_registerStatus(false)
{
	GK_LOG_FUNC

	_devices.setGKfs(pGKfs);

	this->setCurrentSessionObjectPath(pid);

	this->registerWithDaemon();

	try {
		this->updateSessionState();

		this->getDaemonDependenciesMap(dependencies);

		this->initializeGKDBusSignals();
		this->initializeGKDBusMethods();

		_devices.setClientID(_clientID);

		this->initializeDevices();

		process::setSignalHandler(SIGUSR1, DBusHandler::handleSignal);
		process::setSignalHandler(SIGUSR2, DBusHandler::handleSignal);
	}
	catch ( const GLogiKExcept & e ) {
		/* don't show desktop notifications */
		this->prepareToStop(false);

		/* clean each already declared DBus signal/method */
		this->cleanGKDBusEvents();

		throw;
	}
}

DBusHandler::~DBusHandler()
{
	GK_LOG_FUNC

	/* don't show desktop notifications */
	this->prepareToStop(false);

	this->cleanGKDBusEvents();
}

/*
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 *
 * === public === public === public === public === public ===
 *
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 */

void DBusHandler::checkNotifyEvents(NSGKUtils::FileSystem* pGKfs)
{
	GK_LOG_FUNC

	DevicesFilesMap_type devicesMap = _devices.getDevicesFilesMap();

	/* can thow */
	pGKfs->readNotifyEvents( devicesMap );

	for( const auto & device : devicesMap ) {
		GKLog4(trace,
			device.first, " filesystem notification event",
			"reloading file : ", device.second
		)
		_devices.reloadDeviceConfigurationFile(device.first);

		/*
		 * force state update, to load active user's parameters
		 * for all plugged devices
		 */
		if( _sessionState == "active" ) {
			this->reportChangedState();
		}
	}
}

/* return false if we want to exit on next main loop run */
const bool DBusHandler::getExitStatus(void) const
{
	return ( ! DBusHandler::WantToExit );
}

void DBusHandler::prepareToStop(const bool notifications)
{
	GK_LOG_FUNC

	GKLog(trace, "clearing devices and unregistering with daemon")

	if( _registerStatus ) {
		/* We must be registered against the daemon to clear (unref) devices.
		 * Anyway, devices are always initialized *after* a successful registration. */
		_devices.clearDevices(notifications);

		this->unregisterWithDaemon();

		/* send signal to GUI */
		this->sendDevicesUpdatedSignal();
	}
	else {
		GKLog2(trace, "client not registered with deamon : ", _CURRENT_SESSION_DBUS_OBJECT_PATH)
	}
}

void DBusHandler::cleanGKDBusEvents(void) noexcept
{
	GK_LOG_FUNC

	GKLog(trace, "cleaning GKDBus configuration")

	/* remove SessionMessageHandler D-Bus interface and object */
	DBus.removeMethodsInterface(_sessionBus,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE);

	/* remove GUISessionMessageHandler D-Bus interface and object */
	DBus.removeSignalsInterface(_sessionBus,
		GLOGIK_DESKTOP_QT5_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DESKTOP_QT5_SESSION_DBUS_OBJECT_PATH,
		GLOGIK_DESKTOP_QT5_SESSION_DBUS_INTERFACE);

	/* remove DevicesManager D-Bus interface and object */
	DBus.removeSignalsInterface(_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE);

	/* remove ClientsManager D-Bus interface and object */
	DBus.removeSignalsInterface(_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE);

	/* remove PropertyChanged signal event */
	switch(_sessionFramework) {
		/* logind */
		case SessionFramework::FW_LOGIND:
			DBus.removeSignalsInterface(_systemBus,
				LOGIND_DBUS_BUS_CONNECTION_NAME,
				_CURRENT_SESSION_DBUS_OBJECT_PATH.c_str(),
				FREEDESKTOP_DBUS_PROPERTIES_STANDARD_INTERFACE);
			break;
		default:
			LOG(warning) << "unknown session tracker";
			break;
	}
}

/*
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 *
 * === private === private === private === private === private ===
 *
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 */

void DBusHandler::handleSignal(int signum)
{
	GK_LOG_FUNC

	switch( signum ) {
		case SIGUSR1:
			LOG(info) << process::getSignalHandlingDesc(signum, " --> sending restart request and exiting");

			process::resetSignalHandler(SIGUSR1);

			DBusHandler::WantToExit = true;
			DBusHandler::sendServiceStartRequest();
			break;
		default:
			LOG(warning) << process::getSignalHandlingDesc(signum, " --> unhandled");
			break;
	}
}

/*
 * register against the daemon, throws on failure
 */
void DBusHandler::registerWithDaemon(void)
{
	GK_LOG_FUNC

	if( _registerStatus ) {
		LOG(warning) << "don't need to register, since we are already registered";
		return;
	}

	const std::string remoteMethod(GK_DBUS_DAEMON_METHOD_REGISTER_CLIENT);

	try {
		DBus.initializeRemoteMethodCall(
			_systemBus,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		DBus.appendStringToRemoteMethodCall(_CURRENT_SESSION_DBUS_OBJECT_PATH);
		DBus.sendRemoteMethodCall();

		/* -- */

		try {
			DBus.waitForRemoteMethodCallReply(); /* (1) */

			const bool ret = DBus.getNextBooleanArgument(); /* (2) */
			/* nextString - *clientID* or *failure reason* */
			const std::string nextString = DBus.getNextStringArgument(); /* (3) */

			if( ret ) {
				_clientID = nextString;
				_registerStatus = true;

				try {
					_daemonVersion = DBus.getNextStringArgument();

					if( _daemonVersion != VERSION ) {
						std::string mismatch("daemon version mismatch : ");
						mismatch += _daemonVersion;
						throw GLogiKExcept(mismatch);
					}

					LOG(info) << "successfully registered with daemon - " << _clientID;
				}
				catch (const GLogiKExcept & e) {
					LOG(warning) << e.what() << " - will unregister";
					this->unregisterWithDaemon();
				}
			}
			else {
				LOG(error) << "failed to register with daemon : false - " << nextString;
			}
		}
		catch (const GLogiKExcept & e) {
			/* potential GKDBus exceptions on these calls: (1) (2) (3) */
			LogRemoteCallGetReplyFailure
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		DBus.abandonRemoteMethodCall();
		LogRemoteCallFailure
	}

	if( ! _registerStatus ) {
		LOG(error) << "can't register, giving up";
		throw GLogiKExcept("unable to register with daemon");
	}
}

void DBusHandler::unregisterWithDaemon(void)
{
	GK_LOG_FUNC

	if( ! _registerStatus ) {
		GKLog(trace, "cannot unregister, since we are not currently registered")
		return;
	}

	const std::string remoteMethod(GK_DBUS_DAEMON_METHOD_UNREGISTER_CLIENT);

	try {
		/* telling the daemon we're killing ourself */
		DBus.initializeRemoteMethodCall(
			_systemBus,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		DBus.appendStringToRemoteMethodCall(_clientID);
		DBus.sendRemoteMethodCall();

		try {
			DBus.waitForRemoteMethodCallReply();

			const bool ret = DBus.getNextBooleanArgument();
			if( ret ) {
				_registerStatus = false;
				_clientID = "undefined";
				_daemonVersion = "unknown";
				LOG(info) << "successfully unregistered with daemon";
			}
			else {
				LOG(error) << "failed to unregister with daemon : false";
			}
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		DBus.abandonRemoteMethodCall();
		LogRemoteCallFailure
	}
}

void DBusHandler::getDaemonDependenciesMap(GKDepsMap_type* const dependencies)
{
	GK_LOG_FUNC

	if( ! _registerStatus ) {
		LOG(warning) << "currently not registered";
		return;
	}

	const std::string remoteMethod(GK_DBUS_DAEMON_METHOD_GET_DAEMON_DEPENDENCIES_MAP);

	try {
		DBus.initializeRemoteMethodCall(
			_systemBus,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		DBus.appendStringToRemoteMethodCall(_clientID);
		DBus.sendRemoteMethodCall();

		try {
			DBus.waitForRemoteMethodCallReply();

			GKDepsMap_type daemonDeps = DBus.getNextGKDepsMapArgument();
			dependencies->insert(daemonDeps.begin(), daemonDeps.end());
			// /* debug */ printVersionDeps("daemon / service dependencies", (*dependencies));
			return;
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		DBus.abandonRemoteMethodCall();
		LogRemoteCallFailure
	}

	/* fatal error */
	throw GLogiKExcept("unable to get daemon dependencies map");
}

void DBusHandler::setCurrentSessionObjectPath(pid_t pid)
{
	GK_LOG_FUNC

	try {
		const std::string remoteMethod("GetSessionByPID");

		try {
			/* getting logind current session */
			DBus.initializeRemoteMethodCall(
				_systemBus,
				LOGIND_DBUS_BUS_CONNECTION_NAME,
				LOGIND_MANAGER_DBUS_OBJECT_PATH,
				LOGIND_MANAGER_DBUS_INTERFACE,
				"GetSessionByPID"
			);
			DBus.appendUInt32ToRemoteMethodCall(pid);
			DBus.sendRemoteMethodCall();

			try {
				DBus.waitForRemoteMethodCallReply();
				_CURRENT_SESSION_DBUS_OBJECT_PATH = DBus.getNextStringArgument();

				GKLog2(trace, "GetSessionByPID : ", _CURRENT_SESSION_DBUS_OBJECT_PATH)

				_sessionFramework = SessionFramework::FW_LOGIND;

				LOG(info) << "successfully contacted logind";
				return; /* everything ok */
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
				throw GLogiKExcept("failure to get session ID from logind");
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			DBus.abandonRemoteMethodCall();
			LogRemoteCallFailure
		}
	}
	catch ( const GLogiKExcept & e ) {
		LOG(error) << e.what();
	}

	/* fatal error */
	throw GLogiKExcept("unable to contact a session manager");
}

void DBusHandler::updateSessionState(void)
{
	GK_LOG_FUNC

	_sessionState = this->getCurrentSessionState();

	GKLog2(trace, "switching session state to : ", _sessionState)

	this->reportChangedState();
}

/*
 * Ask to session tracker the current session state, and returns it.
 * If the state fails to be updated for whatever reason, throws.
 * Possible returns values are :
 *  - active
 *  - online
 *  - closing
 */
const std::string DBusHandler::getCurrentSessionState(void)
{
	GK_LOG_FUNC

	std::string remoteMethod;
	switch(_sessionFramework) {
		case SessionFramework::FW_LOGIND:
			/* logind */
			remoteMethod = "Get";
			try {
				DBus.initializeRemoteMethodCall(
					_systemBus,
					LOGIND_DBUS_BUS_CONNECTION_NAME,
					_CURRENT_SESSION_DBUS_OBJECT_PATH.c_str(),
					FREEDESKTOP_DBUS_PROPERTIES_STANDARD_INTERFACE,
					remoteMethod.c_str()
				);
				DBus.appendStringToRemoteMethodCall(LOGIND_SESSION_DBUS_INTERFACE);
				DBus.appendStringToRemoteMethodCall("State");
				DBus.sendRemoteMethodCall();

				try {
					DBus.waitForRemoteMethodCallReply();
					return DBus.getNextStringArgument();
				}
				catch (const GLogiKExcept & e) {
					LogRemoteCallGetReplyFailure
				}
			}
			catch (const GKDBusMessageWrongBuild & e) {
				DBus.abandonRemoteMethodCall();
				LogRemoteCallFailure
			}
			break;
		default:
			throw GLogiKExcept("unhandled session tracker");
			break;
	}

	throw GLogiKExcept("unable to get session state");
}

void DBusHandler::reportChangedState(void) noexcept
{
	GK_LOG_FUNC

	if( ! _registerStatus ) {
		GKLog(trace, "currently not registered, skipping report state")
		return;
	}

	const std::string remoteMethod(GK_DBUS_DAEMON_METHOD_UPDATE_CLIENT_STATE);

	try {
		DBus.initializeRemoteMethodCall(
			_systemBus,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		DBus.appendStringToRemoteMethodCall(_clientID);
		DBus.appendStringToRemoteMethodCall(_sessionState);
		DBus.sendRemoteMethodCall();

		try {
			DBus.waitForRemoteMethodCallReply();
			const bool ret( DBus.getNextBooleanArgument() );
			if( ! ret ) {
				LOG(error) << "failed to report changed state : false";
			}
			else {
				GKLog2(trace, "successfully reported changed state : ", _sessionState)
			}
			return;
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		DBus.abandonRemoteMethodCall();
		LogRemoteCallFailure
	}
}

void DBusHandler::initializeDevices(void)
{
	GK_LOG_FUNC

	GKLog(trace, "initializing devices")

	if( ! _registerStatus ) {
		GKLog(trace, "currently not registered, giving up")
		return;
	}

	std::string device;
	std::vector<std::string> devicesID;

	std::string remoteMethod(GK_DBUS_DAEMON_METHOD_GET_STARTED_DEVICES);

	/* started devices */
	try {
		DBus.initializeRemoteMethodCall(
			_systemBus,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		DBus.appendStringToRemoteMethodCall(_clientID);
		DBus.sendRemoteMethodCall();

		try {
			DBus.waitForRemoteMethodCallReply();

			devicesID = DBus.getNextStringArray();
			this->devicesStarted(devicesID);
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		DBus.abandonRemoteMethodCall();
		LogRemoteCallFailure
	}

	devicesID.clear();

	remoteMethod = GK_DBUS_DAEMON_METHOD_SET_CLIENT_READY;

	/* saying the daemon that we are ready */
	try {
		DBus.initializeRemoteMethodCall(
			_systemBus,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		DBus.appendStringToRemoteMethodCall(_clientID);
		DBus.sendRemoteMethodCall();

		try {
			DBus.waitForRemoteMethodCallReply();

			const bool ret = DBus.getNextBooleanArgument();
			if( ! ret ) {
				LOG(warning) << "failed to enable ready state : false";
			}
			else {
				GKLog(trace, "successfully enabled ready state")
			}
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		DBus.abandonRemoteMethodCall();
		LogRemoteCallFailure
	}

	remoteMethod = GK_DBUS_DAEMON_METHOD_GET_STOPPED_DEVICES;

	/* stopped devices */
	try {
		DBus.initializeRemoteMethodCall(
			_systemBus,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		DBus.appendStringToRemoteMethodCall(_clientID);
		DBus.sendRemoteMethodCall();

		try {
			DBus.waitForRemoteMethodCallReply();

			devicesID = DBus.getNextStringArray();
			this->devicesStopped(devicesID);
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		DBus.abandonRemoteMethodCall();
		LogRemoteCallFailure
	}
}

void DBusHandler::sendServiceStartRequest(void)
{
	GK_LOG_FUNC

	try {
		/* asking the launcher to spawn the service after sleeping 300 ms
		 *
		 * this process is about to exit, and the launcher must wait enough time
		 * before starting a new service process, to make sure that the
		 * GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME is not already used
		 */
		DBus.initializeBroadcastSignal(
			_sessionBus,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
			GK_DBUS_LAUNCHER_SIGNAL_SERVICE_START_REQUEST
		);
		DBus.appendUInt16ToBroadcastSignal(300);
		DBus.sendBroadcastSignal();

		LOG(info) << "sent signal: " << GK_DBUS_LAUNCHER_SIGNAL_SERVICE_START_REQUEST;
	}
	catch (const GKDBusMessageWrongBuild & e) {
		DBus.abandonBroadcastSignal();
		LOG(error)	<< "failed to send signal: "
					<< GK_DBUS_LAUNCHER_SIGNAL_SERVICE_START_REQUEST
					<< " - " << e.what();
	}
}

void DBusHandler::sendDevicesUpdatedSignal(void)
{
	GK_LOG_FUNC

	try {
		/* send DevicesUpdated signal to GUI applications */
		DBus.initializeBroadcastSignal(
			_sessionBus,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
			GK_DBUS_GUI_SIGNAL_DEVICES_UPDATED
		);
		DBus.sendBroadcastSignal();

		LOG(info) << "sent signal: " << GK_DBUS_GUI_SIGNAL_DEVICES_UPDATED;
	}
	catch (const GKDBusMessageWrongBuild & e) {
		DBus.abandonBroadcastSignal();
		LOG(error)	<< "failed to send signal: "
					<< GK_DBUS_GUI_SIGNAL_DEVICES_UPDATED
					<< " - " << e.what();
	}
}

/*
 * --- --- --- --- ---
 * --- --- --- --- ---
 *
 * signals - signals
 *
 * --- --- --- --- ---
 * --- --- --- --- ---
 *
 */

void DBusHandler::initializeGKDBusSignals(void)
{
	/* update session state when PropertyChanged signal receipted */
	switch(_sessionFramework) {
		/* logind */
		case SessionFramework::FW_LOGIND:
			DBus.NSGKDBus::Callback<SIGv2v>::receiveSignal(
				_systemBus,
				LOGIND_DBUS_BUS_CONNECTION_NAME,
				_CURRENT_SESSION_DBUS_OBJECT_PATH.c_str(),
				FREEDESKTOP_DBUS_PROPERTIES_STANDARD_INTERFACE,
				"PropertiesChanged",
				{},
				std::bind(&DBusHandler::updateSessionState, this)
			);
			break;
		default:
			LOG(warning) << "unknown session tracker";
			break;
	}

	/*
	 * want to be warned by the daemon about those signals
	 * each declaration can throw a GLogiKExcept exception
	 * if something is wrong (handled in constructor)
	 */

	/* -- -- -- -- -- -- -- -- -- -- */
	/*  DevicesManager D-Bus object  */
	/* -- -- -- -- -- -- -- -- -- -- */
	DBus.NSGKDBus::Callback<SIGas2v>::receiveSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		GK_DBUS_SERVICE_SIGNAL_DEVICES_STARTED,
		{	{"as", "", "in", "array of started devices ID strings"} },
		std::bind(&DBusHandler::devicesStarted, this, std::placeholders::_1)
	);

	DBus.NSGKDBus::Callback<SIGas2v>::receiveSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		GK_DBUS_SERVICE_SIGNAL_DEVICES_STOPPED,
		{	{"as", "", "in", "array of stopped devices ID strings"} },
		std::bind(&DBusHandler::devicesStopped, this, std::placeholders::_1)
	);

	DBus.NSGKDBus::Callback<SIGas2v>::receiveSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		GK_DBUS_SERVICE_SIGNAL_DEVICES_UNPLUGGED,
		{	{"as", "", "in", "array of unplugged devices ID strings"} },
		std::bind(&DBusHandler::devicesUnplugged, this, std::placeholders::_1)
	);

	DBus.NSGKDBus::Callback<SIGsm2v>::receiveSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		GK_DBUS_SERVICE_SIGNAL_DEVICE_MBANK_SWITCH,
		{	{"s", "device_id", "in", "device ID"},
			{"y", "macro_bankID", "in", "macro bankID"}
		},
		std::bind(&DBusHandler::deviceMBankSwitch, this,
			std::placeholders::_1, std::placeholders::_2
		)
	);

	DBus.NSGKDBus::Callback<SIGsGM2v>::receiveSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		GK_DBUS_SERVICE_SIGNAL_DEVICE_MACRO_RECORDED,
		{	{"s", "device_id", "in", "device ID"},
			{"y", "macro_keyID", "in", "macro key ID"},
			{"a(yyq)", "macro_array", "in", "macro array"}
		},
		std::bind(&DBusHandler::deviceMacroRecorded, this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3
		)
	);

	DBus.NSGKDBus::Callback<SIGsG2v>::receiveSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		GK_DBUS_SERVICE_SIGNAL_DEVICE_MACRO_CLEARED,
		{	{"s", "device_id", "in", "device ID"},
			{"y", "macro_keyID", "in", "macro key ID"}
		},
		std::bind(&DBusHandler::deviceMacroCleared, this,
			std::placeholders::_1, std::placeholders::_2
		)
	);

	DBus.NSGKDBus::Callback<SIGsG2v>::receiveSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		GK_DBUS_SERVICE_SIGNAL_DEVICE_GKEY_EVENT,
		{	{"s", "device_id", "in", "device ID"},
			{"y", "macro_keyID", "in", "macro key ID"}
		},
		std::bind(&DBusHandler::deviceGKeyEvent, this,
			std::placeholders::_1, std::placeholders::_2
		)
	);

	DBus.NSGKDBus::Callback<SIGss2v>::receiveSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		GK_DBUS_SERVICE_SIGNAL_DEVICE_MEDIA_EVENT,
		{	{"s", "device_id", "in", "device ID"},
			{"s", "media_key_event", "in", "media key event"}
		},
		std::bind(&DBusHandler::deviceMediaEvent, this,
			std::placeholders::_1, std::placeholders::_2
		)
	);

	/* -- -- -- -- -- -- -- -- -- -- */
	/*  ClientsManager D-Bus object  */
	/* -- -- -- -- -- -- -- -- -- -- */
	DBus.NSGKDBus::Callback<SIGv2v>::receiveSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
		GK_DBUS_SERVICE_SIGNAL_DEAMON_IS_STOPPING,
		{},
		std::bind(&DBusHandler::daemonIsStopping, this)
	);

	DBus.NSGKDBus::Callback<SIGv2v>::receiveSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
		GK_DBUS_SERVICE_SIGNAL_DEAMON_IS_STARTING,
		{},
		std::bind(&DBusHandler::daemonIsStarting, this)
	);

	DBus.NSGKDBus::Callback<SIGv2v>::receiveSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
		GK_DBUS_SERVICE_SIGNAL_REPORT_YOURSELF,
		{},
		std::bind(&DBusHandler::reportChangedState, this)
	);

	/* -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
	/*   GUISessionMessageHandler GUI requests D-Bus object  */
	/* -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
	DBus.NSGKDBus::Callback<SIGss2v>::receiveSignal(
		_sessionBus,
		GLOGIK_DESKTOP_QT5_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DESKTOP_QT5_SESSION_DBUS_OBJECT_PATH,
		GLOGIK_DESKTOP_QT5_SESSION_DBUS_INTERFACE,
		GK_DBUS_SERVICE_SIGNAL_DEVICE_STATUS_CHANGE_REQUEST,
		{	{"s", "device_id", "in", "device ID"},
			{"s", "wanted_status", "in", "wanted status"}	},
		std::bind(&DBusHandler::deviceStatusChangeRequest, this,
		std::placeholders::_1, std::placeholders::_2)
	);

	DBus.declareIntrospectableSignal(
		_sessionBus,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		GK_DBUS_LAUNCHER_SIGNAL_SERVICE_START_REQUEST,
		{ {"q", "sleep_ms", "out", "sleeping time in milliseconds before spawning service"} }
	);

	DBus.declareIntrospectableSignal(
		_sessionBus,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		GK_DBUS_GUI_SIGNAL_DEVICES_UPDATED,
		{}
	);

	DBus.declareIntrospectableSignal(
		_sessionBus,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		GK_DBUS_GUI_SIGNAL_DEVICE_CONFIGURATION_SAVED,
		{ {"s", "device_id", "out", "device ID"} }
	);
}

void DBusHandler::initializeGKDBusMethods(void)
{
	const std::string r_ed("reserved");

	/* -- -- -- -- -- -- -- -- -- -- -- -- */
	/*  SessionMessageHandler D-Bus object */
	/* -- -- -- -- -- -- -- -- -- -- -- -- */

	DBus.NSGKDBus::Callback<SIGs2as>::exposeMethod(
		_sessionBus,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		GK_DBUS_SERVICE_METHOD_GET_DEVICES_LIST,
		{	{"s", r_ed, "in", r_ed},
			{"as", "array_of_strings", "out", "array of devices ID and configuration files"} },
		std::bind(&DBusHandler::getDevicesList, this, r_ed) );

	DBus.NSGKDBus::Callback<SIGs2as>::exposeMethod(
		_sessionBus,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		GK_DBUS_SERVICE_METHOD_GET_INFORMATIONS,
		{	{"s", r_ed, "in", r_ed},
			{"as", "array_of_strings", "out", "array of informations strings"} },
		std::bind(&DBusHandler::getInformations, this, r_ed) );

	DBus.NSGKDBus::Callback<SIGss2aP>::exposeMethod(
		_sessionBus,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		GK_DBUS_DAEMON_METHOD_GET_DEVICE_LCD_PLUGINS_PROPERTIES, /* same method name as in daemon */
		{	{"s", "device_id", "in", "device ID"},
			{"s", r_ed, "in", r_ed},
			{"a(tss)", "get_lcd_plugins_properties_array", "out", "LCDPluginsProperties array"} },
		std::bind(&DBusHandler::getDeviceLCDPluginsProperties, this, std::placeholders::_1, r_ed) );

	DBus.NSGKDBus::Callback<SIGs2D>::exposeMethod(
		_sessionBus,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		GK_DBUS_SERVICE_METHOD_GET_EXECUTABLES_DEPENDENCIES_MAP,
		{	{"s", r_ed, "in", r_ed},
			{"a(yta(sss))", "dependencies_map", "out", "array of executable dependencies"} },
		std::bind(&DBusHandler::getExecutablesDependenciesMap, this, r_ed) );
}

void DBusHandler::daemonIsStopping(void)
{
	GK_LOG_FUNC

	LOG(info) << "received signal : " << __func__;

	this->prepareToStop();
}

void DBusHandler::daemonIsStarting(void)
{
	GK_LOG_FUNC

	if( _registerStatus ) {
		GKLog4(trace,
			"received signal : ", __func__,
			"but we are already registered with daemon : ", _CURRENT_SESSION_DBUS_OBJECT_PATH
		)
	}
	else {
		LOG(info)	<< "received signal : " << __func__
					<< " - contacting the daemon";

		try {
			this->registerWithDaemon();
			if( _registerStatus ) {
				this->updateSessionState();
				_devices.setClientID(_clientID);
				this->initializeDevices();
			}
		}
		catch (const GLogiKExcept & e) {
			LOG(error) << e.what();
			DBusHandler::WantToExit = true;
			this->sendServiceStartRequest();
		}
	}
}

void DBusHandler::devicesStarted(const std::vector<std::string> & devicesID)
{
	GK_LOG_FUNC

	GKLog4(trace,
		"received signal : ", __func__,
		"number of devices : ", devicesID.size()
	)

	const std::string remoteMethod(GK_DBUS_DAEMON_METHOD_GET_DEVICE_STATUS);

	bool devicesUpdated(false);

	for(const auto& devID : devicesID) {
		try {
			DBus.initializeRemoteMethodCall(
				_systemBus,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			DBus.appendStringToRemoteMethodCall(_clientID);
			DBus.appendStringToRemoteMethodCall(devID);
			DBus.sendRemoteMethodCall();

			try {
				DBus.waitForRemoteMethodCallReply();

				const std::string deviceStatus( DBus.getNextStringArgument() );
				if(deviceStatus == "started") {
					GKLog2(trace, devID, " status from daemon : started")
					_devices.startDevice(devID);
					devicesUpdated = true;
				}
				else {
					LOG(warning) << "received devicesStarted signal for device " << devID;
					LOG(warning) << "but daemon is saying that device status is : " << deviceStatus;
				}
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			DBus.abandonRemoteMethodCall();
			LogRemoteCallFailure
		}
	}

	if(devicesUpdated) {
		/* send signal to GUI */
		this->sendDevicesUpdatedSignal();
	}

	/*
	 * force state update, to load active user's parameters
	 * for all hotplugged devices
	 */
	if( _sessionState == "active" ) {
		this->reportChangedState();
	}
}

void DBusHandler::devicesStopped(const std::vector<std::string> & devicesID)
{
	GK_LOG_FUNC

	GKLog4(trace,
		"received signal : ", __func__,
		"number of devices : ", devicesID.size()
	)

	const std::string remoteMethod(GK_DBUS_DAEMON_METHOD_GET_DEVICE_STATUS);

	bool devicesUpdated(false);

	for(const auto& devID : devicesID) {
		try {
			DBus.initializeRemoteMethodCall(
				_systemBus,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			DBus.appendStringToRemoteMethodCall(_clientID);
			DBus.appendStringToRemoteMethodCall(devID);
			DBus.sendRemoteMethodCall();

			try {
				DBus.waitForRemoteMethodCallReply();

				const std::string deviceStatus( DBus.getNextStringArgument() );
				if(deviceStatus == "stopped") {
					GKLog2(trace, devID, " status from daemon : stopped")
					_devices.stopDevice(devID);
					devicesUpdated = true;
				}
				else {
					LOG(warning) << "received devicesStopped signal for device " << devID;
					LOG(warning) << "but daemon is saying that device status is : " << deviceStatus;
				}
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			DBus.abandonRemoteMethodCall();
			LogRemoteCallFailure
		}
	}

	if(devicesUpdated) {
		/* send signal to GUI */
		this->sendDevicesUpdatedSignal();
	}
}

void DBusHandler::devicesUnplugged(const std::vector<std::string> & devicesID)
{
	GK_LOG_FUNC

	GKLog4(trace,
		"received signal : ", __func__,
		"number of devices : ", devicesID.size()
	)

	const std::string remoteMethod(GK_DBUS_DAEMON_METHOD_GET_DEVICE_STATUS);

	bool devicesUpdated(false);

	for(const auto& devID : devicesID) {
		try {
			DBus.initializeRemoteMethodCall(
				_systemBus,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			DBus.appendStringToRemoteMethodCall(_clientID);
			DBus.appendStringToRemoteMethodCall(devID);
			DBus.sendRemoteMethodCall();

			try {
				DBus.waitForRemoteMethodCallReply();

				const std::string deviceStatus( DBus.getNextStringArgument() );
				if(deviceStatus == "unplugged") {
					GKLog2(trace, devID, " status from daemon : unplugged")
					_devices.unplugDevice(devID);
					devicesUpdated = true;
				}
				else {
					LOG(warning) << "received devicesUnplugged signal for device " << devID;
					LOG(warning) << "but daemon is saying that device status is : " << deviceStatus;
				}
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			DBus.abandonRemoteMethodCall();
			LogRemoteCallFailure
		}
	}

	if(devicesUpdated) {
		/* send signal to GUI */
		this->sendDevicesUpdatedSignal();
	}
}

void DBusHandler::deviceMBankSwitch(
	const std::string & devID,
	const MKeysID bankID)
{
	GK_LOG_FUNC

	GKLog3(trace, devID, " received signal - bankID: ", bankID)

	if( ! _registerStatus ) {
		GKLog(trace, "currently not registered, skipping")
		return;
	}

	if( _sessionState != "active" ) {
		GKLog(trace, "currently not active, skipping")
		return;
	}

	LOG(info)	<< "received signal: " << GK_DBUS_SERVICE_SIGNAL_DEVICE_MBANK_SWITCH
				<< " - " << bankID;

	try {
		_devices.setDeviceCurrentBankID(devID, bankID);
	}
	catch (const GLogiKExcept & e) {
		LOG(error) << devID << " setting bankID failure - " << bankID;
	}
}

void DBusHandler::deviceMacroRecorded(
	const std::string & devID,
	const GKeysID keyID,
	const macro_type & macro)
{
	GK_LOG_FUNC

	GKLog3(trace, devID, " received signal for key: ", getGKeyName(keyID))

	if( ! _registerStatus ) {
		GKLog(trace, "currently not registered, skipping")
		return;
	}

	if( _sessionState != "active" ) {
		GKLog(trace, "currently not active, skipping")
		return;
	}

	try {
		MKeysID bankID;
		banksMap_type & banksMap = _devices.getDeviceBanks(devID, bankID);

		_GKeysEvent.setMacro(banksMap, macro, bankID, keyID);

		_devices.saveDeviceConfigurationFile(devID);
	}
	catch (const GLogiKExcept & e) {
		LOG(error) << devID << " macro record failure - " << keyID;
	}
}

void DBusHandler::deviceMacroCleared(const std::string & devID, const GKeysID keyID)
{
	GK_LOG_FUNC

	GKLog3(trace, devID, " received signal for key: ", getGKeyName(keyID))

	if( ! _registerStatus ) {
		GKLog(trace, "currently not registered, skipping")
		return;
	}

	if( _sessionState != "active" ) {
		GKLog(trace, "currently not active, skipping")
		return;
	}

	try {
		MKeysID bankID;
		banksMap_type & banksMap = _devices.getDeviceBanks(devID, bankID);

		if( _GKeysEvent.clearMacro(banksMap, bankID, keyID) ) {
			_devices.saveDeviceConfigurationFile(devID);
		}
	}
	catch (const GLogiKExcept & e) {
		LOG(error) << devID << " clear macro failure - " << keyID;
	}
}

void DBusHandler::deviceMediaEvent(
	const std::string & devID,
	const std::string & mediaKeyEvent)
{
	GK_LOG_FUNC

	GKLog3(trace, devID, " received signal for event: ", mediaKeyEvent)

	if( ! _registerStatus ) {
		GKLog(trace, "currently not registered, skipping")
		return;
	}

	if( _sessionState != "active" ) {
		GKLog(trace, "currently not active, skipping")
		return;
	}

	_devices.doDeviceFakeKeyEvent(devID, mediaKeyEvent);
}

void DBusHandler::deviceGKeyEvent(const std::string & devID, const GKeysID keyID)
{
	GK_LOG_FUNC

	GKLog3(trace, devID, " received signal for event: ", getGKeyName(keyID))

	if( ! _registerStatus ) {
		GKLog(trace, "currently not registered, skipping")
		return;
	}

	if( _sessionState != "active" ) {
		GKLog(trace, "currently not active, skipping")
		return;
	}

	try {
		MKeysID bankID;
		banksMap_type & banksMap = _devices.getDeviceBanks(devID, bankID);

		_GKeysEvent.runEvent(banksMap, bankID, keyID);
	}
	catch (const GLogiKExcept & e) {
		LOG(error) << devID << " run event failure - " << keyID;
	}
};

const std::vector<std::string> DBusHandler::getDevicesList(const std::string & reserved) {
	return _devices.getDevicesList();
}

const std::vector<std::string> DBusHandler::getInformations(const std::string & reserved)
{
	std::vector<std::string> ret;
	ret.push_back(_daemonVersion);
	ret.push_back(VERSION);
	if(_registerStatus)
		ret.push_back("registered");
	else
		ret.push_back("unregistered");
	return ret;
}

const LCDPPArray_type & DBusHandler::getDeviceLCDPluginsProperties(
	const std::string & devID,
	const std::string & reserved)
{
	return _devices.getDeviceLCDPluginsProperties(devID);
}

const GKDepsMap_type & DBusHandler::getExecutablesDependenciesMap(const std::string & reserved)
{
	return (*_pDepsMap);
}

void DBusHandler::deviceStatusChangeRequest(
	const std::string & devID,
	const std::string & remoteMethod)
{
	GK_LOG_FUNC

	GKLog4(trace,
		devID, " received status change request",
		"requested status : ", remoteMethod
	)

	/* request coming from GUI on session bus, but
	 * methods names are the same as in the daemon
	 */
	if(	(remoteMethod !=  GK_DBUS_DAEMON_METHOD_STOP_DEVICE) and
		(remoteMethod !=  GK_DBUS_DAEMON_METHOD_START_DEVICE) and
		(remoteMethod !=  GK_DBUS_DAEMON_METHOD_RESTART_DEVICE) ) {
		LOG(warning) << devID << " ignoring wrong remote method : " << remoteMethod;
		return;
	}

	try {
		DBus.initializeRemoteMethodCall(
			_systemBus,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		DBus.appendStringToRemoteMethodCall(_clientID);
		DBus.appendStringToRemoteMethodCall(devID);
		DBus.sendRemoteMethodCall();

		try {
			DBus.waitForRemoteMethodCallReply();

			const bool ret = DBus.getNextBooleanArgument();
			if( ! ret ) {
				LOG(error) << devID << " request failure : false";
			}
			else {
				GKLog2(trace, devID, " request successfully done")
			}
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		DBus.abandonRemoteMethodCall();
		LogRemoteCallFailure
	}
}

} // namespace GLogiK

