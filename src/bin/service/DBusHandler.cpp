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

#include <sstream>
#include <new>
#include <functional>
#include <stdexcept>
#include <thread>
#include <chrono>

#include "lib/shared/glogik.hpp"

#include "DBusHandler.hpp"

#include "include/MBank.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

restartRequest::restartRequest( const std::string& msg ) : message(msg) {}
restartRequest::~restartRequest( void ) throw() {}

const char* restartRequest::what( void ) const throw()
{
    return message.c_str();
}

DBusHandler::DBusHandler(
	pid_t pid,
	SessionManager& session,
	NSGKUtils::FileSystem* pGKfs,
	NSGKDBus::GKDBus* pDBus)
	:	_clientID("undefined"),
		_daemonVersion("unknown"),
		_currentSession(""),
		_sessionState(""),
		_pDBus(pDBus),
		_sessionFramework(SessionFramework::FW_UNKNOWN),
		_sessionBus(NSGKDBus::BusConnection::GKDBUS_SESSION),
		_systemBus(NSGKDBus::BusConnection::GKDBUS_SYSTEM),
		_registerStatus(false),
		_wantToExit(false)
{
	GK_LOG_FUNC

	_devices.setGKfs(pGKfs);

	try {
		this->setCurrentSessionObjectPath(pid);

		try {
			unsigned int retries = 0;
			while( ! _registerStatus ) {
				if( ( session.isSessionAlive() )
						and (retries < UNREACHABLE_DAEMON_MAX_RETRIES) )
				{
					if(retries > 0) {
						LOG(info) << "register retry " << retries << " ...";
					}

					this->registerWithDaemon();

					if( ! _registerStatus ) {
						unsigned int timer = 5;
						LOG(warning) << "retrying in " << timer << " seconds ...";
						std::this_thread::sleep_for(std::chrono::seconds(timer));
					}
				}
				else {
					LOG(error) << "can't register, giving up";
					throw GLogiKExcept("unable to register with daemon");
				}
				retries++;
			}
		}
		catch ( const restartRequest & e ) {
			this->sendRestartRequest(); /* throws */
		}

		this->updateSessionState();

		this->initializeGKDBusSignals();
		this->initializeGKDBusMethods();

		/* set GKDBus pointer */
		_devices.setDBus(_pDBus);
		_devices.setClientID(_clientID);

		this->initializeDevices();
	}
	catch ( const GLogiKExcept & e ) {
		this->unregisterWithDaemon();
		throw;
	}
}

DBusHandler::~DBusHandler()
{
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

	devices_files_map_t devicesMap = _devices.getDevicesMap();

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
	return ( ! _wantToExit );
}

void DBusHandler::clearAndUnregister(void)
{
	GK_LOG_FUNC

	if( _registerStatus ) {
		_devices.clearDevices();
		this->unregisterWithDaemon();
		/* send signal to GUI */
		this->sendDevicesUpdatedSignal();
	}
	else {
		GKLog2(trace, "client not registered with deamon : ", _currentSession)
	}
}

void DBusHandler::cleanDBusRequests(void)
{
	GK_LOG_FUNC

	this->clearAndUnregister();

	/* remove SessionMessageHandler D-Bus interface and object */
	_pDBus->removeMethodsInterface(_sessionBus,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE);

	/* remove GUISessionMessageHandler D-Bus interface and object */
	_pDBus->removeSignalsInterface(_sessionBus,
		GLOGIK_DESKTOP_QT5_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DESKTOP_QT5_SESSION_DBUS_OBJECT,
		GLOGIK_DESKTOP_QT5_SESSION_DBUS_INTERFACE);

	/* remove DevicesManager D-Bus interface and object */
	_pDBus->removeSignalsInterface(_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE);

	/* remove ClientsManager D-Bus interface and object */
	_pDBus->removeSignalsInterface(_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE);

	const std::string object = _pDBus->getObjectFromObjectPath(_currentSession);

	switch(_sessionFramework) {
		/* logind */
		case SessionFramework::FW_LOGIND:
			_pDBus->removeSignalsInterface(_systemBus,
				"org.freedesktop.login1",
				object.c_str(),
				"org.freedesktop.DBus.Properties");
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

/*
 * register against the daemon
 * throws restartRequest exception if register was sucessful but something
 * goes wrong after (like daemon version mismatch)
 */
void DBusHandler::registerWithDaemon(void)
{
	GK_LOG_FUNC

	if( _registerStatus ) {
		LOG(warning) << "don't need to register, since we are already registered";
		return;
	}

	const std::string remoteMethod("RegisterClient");

	try {
		_pDBus->initializeRemoteMethodCall(
			_systemBus,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		_pDBus->appendStringToRemoteMethodCall(_currentSession);
		_pDBus->sendRemoteMethodCall();

		/* -- */

		try {
			_pDBus->waitForRemoteMethodCallReply();

			const bool ret = _pDBus->getNextBooleanArgument();
			if( ret ) {
				_registerStatus = true;
				try {
					_clientID = _pDBus->getNextStringArgument();
					_daemonVersion = _pDBus->getNextStringArgument();

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
					throw restartRequest();
				}
			}
			else {
				std::string reason("unknown");
				try {
					reason = _pDBus->getNextStringArgument();
				}
				catch (const GLogiKExcept & e) {
					LOG(error) << e.what();
				}
				LOG(error) << "failed to register with daemon : false - " << reason;
			}
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_pDBus->abandonRemoteMethodCall();
		LogRemoteCallFailure
	}
}

void DBusHandler::unregisterWithDaemon(void)
{
	GK_LOG_FUNC

	if( ! _registerStatus ) {
		GKLog(trace, "cannot unregister, since we are not currently registered")
		return;
	}

	const std::string remoteMethod("UnregisterClient");

	try {
		/* telling the daemon we're killing ourself */
		_pDBus->initializeRemoteMethodCall(
			_systemBus,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		_pDBus->appendStringToRemoteMethodCall(_clientID);
		_pDBus->sendRemoteMethodCall();

		try {
			_pDBus->waitForRemoteMethodCallReply();

			const bool ret = _pDBus->getNextBooleanArgument();
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
		_pDBus->abandonRemoteMethodCall();
		LogRemoteCallFailure
	}
}

void DBusHandler::setCurrentSessionObjectPath(pid_t pid)
{
	GK_LOG_FUNC

	try {
		const std::string remoteMethod("GetSessionByPID");

		try {
			/* getting logind current session */
			_pDBus->initializeRemoteMethodCall(
				_systemBus,
				"org.freedesktop.login1",
				"/org/freedesktop/login1",
				"org.freedesktop.login1.Manager",
				"GetSessionByPID"
			);
			_pDBus->appendUInt32ToRemoteMethodCall(pid);
			_pDBus->sendRemoteMethodCall();

			try {
				_pDBus->waitForRemoteMethodCallReply();
				_currentSession = _pDBus->getNextStringArgument();

				GKLog2(trace, "GetSessionByPID : ", _currentSession)

				_sessionFramework = SessionFramework::FW_LOGIND;

				/* update session state when PropertyChanged signal receipted */
				const std::string object = _pDBus->getObjectFromObjectPath(_currentSession);

				_pDBus->NSGKDBus::Callback<SIGv2v>::exposeSignal(
					_systemBus,
					"org.freedesktop.login1",
					object.c_str(),
					"org.freedesktop.DBus.Properties",
					"PropertiesChanged",
					{},
					std::bind(&DBusHandler::updateSessionState, this)
				);

				LOG(info) << "successfully contacted logind";
				return; /* everything ok */
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
				throw GLogiKExcept("failure to get session ID from logind");
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			_pDBus->abandonRemoteMethodCall();
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
				_pDBus->initializeRemoteMethodCall(
					_systemBus,
					"org.freedesktop.login1",
					_currentSession.c_str(),
					"org.freedesktop.DBus.Properties",
					remoteMethod.c_str()
				);
				_pDBus->appendStringToRemoteMethodCall("org.freedesktop.login1.Session");
				_pDBus->appendStringToRemoteMethodCall("State");
				_pDBus->sendRemoteMethodCall();

				try {
					_pDBus->waitForRemoteMethodCallReply();
					return _pDBus->getNextStringArgument();
				}
				catch (const GLogiKExcept & e) {
					LogRemoteCallGetReplyFailure
				}
			}
			catch (const GKDBusMessageWrongBuild & e) {
				_pDBus->abandonRemoteMethodCall();
				LogRemoteCallFailure
			}
			break;
		default:
			throw GLogiKExcept("unhandled session tracker");
			break;
	}

	throw GLogiKExcept("unable to get session state");
}

void DBusHandler::reportChangedState(void)
{
	GK_LOG_FUNC

	if( ! _registerStatus ) {
		GKLog(trace, "currently not registered, skipping report state")
		return;
	}

	std::string remoteMethod("UpdateClientState");

	try {
		_pDBus->initializeRemoteMethodCall(
			_systemBus,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		_pDBus->appendStringToRemoteMethodCall(_clientID);
		_pDBus->appendStringToRemoteMethodCall(_sessionState);
		_pDBus->sendRemoteMethodCall();

		try {
			_pDBus->waitForRemoteMethodCallReply();
			const bool ret( _pDBus->getNextBooleanArgument() );
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
		_pDBus->abandonRemoteMethodCall();
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

	std::string remoteMethod("GetStartedDevices");

	/* started devices */
	try {
		_pDBus->initializeRemoteMethodCall(
			_systemBus,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		_pDBus->appendStringToRemoteMethodCall(_clientID);
		_pDBus->sendRemoteMethodCall();

		try {
			_pDBus->waitForRemoteMethodCallReply();

			devicesID = _pDBus->getStringsArray();
			this->devicesStarted(devicesID);
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_pDBus->abandonRemoteMethodCall();
		LogRemoteCallFailure
	}

	devicesID.clear();

	remoteMethod = "ToggleClientReadyPropertie";

	/* saying the daemon that we are ready */
	try {
		_pDBus->initializeRemoteMethodCall(
			_systemBus,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		_pDBus->appendStringToRemoteMethodCall(_clientID);
		_pDBus->sendRemoteMethodCall();

		try {
			_pDBus->waitForRemoteMethodCallReply();

			const bool ret = _pDBus->getNextBooleanArgument();
			if( ! ret ) {
				LOG(warning) << "failed to toggle ready propertie : false";
			}
			else {
				GKLog(trace, "successfully toggled ready propertie")
			}
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_pDBus->abandonRemoteMethodCall();
		LogRemoteCallFailure
	}

	remoteMethod = "GetStoppedDevices";

	/* stopped devices */
	try {
		_pDBus->initializeRemoteMethodCall(
			_systemBus,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		_pDBus->appendStringToRemoteMethodCall(_clientID);
		_pDBus->sendRemoteMethodCall();

		try {
			_pDBus->waitForRemoteMethodCallReply();

			devicesID = _pDBus->getStringsArray();
			this->devicesStopped(devicesID);
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_pDBus->abandonRemoteMethodCall();
		LogRemoteCallFailure
	}
}

void DBusHandler::sendRestartRequest(void)
{
	GK_LOG_FUNC

	try {
		/* asking the launcher for a restart */
		_pDBus->initializeBroadcastSignal(
			_sessionBus,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
			"RestartRequest"
		);
		_pDBus->sendBroadcastSignal();

		LOG(info) << "sent signal : RestartRequest";
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_pDBus->abandonBroadcastSignal();
		LOG(error) << "failed to send signal : RestartRequest : " << e.what();
	}

	throw GLogiKExcept("restart request done");
}

void DBusHandler::sendDevicesUpdatedSignal(void)
{
	GK_LOG_FUNC

	try {
		/* send DevicesUpdated signal to GUI applications */
		_pDBus->initializeBroadcastSignal(
			_sessionBus,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
			"DevicesUpdated"
		);
		_pDBus->sendBroadcastSignal();

		LOG(info) << "sent signal : DevicesUpdated";
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_pDBus->abandonBroadcastSignal();
		LOG(error) << "failed to send signal : DevicesUpdated : " << e.what();
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
	/*
	 * want to be warned by the daemon about those signals
	 * each declaration can throw a GLogiKExcept exception
	 * if something is wrong (handled in constructor)
	 */

	/* -- -- -- -- -- -- -- -- -- -- */
	/*  DevicesManager D-Bus object  */
	/* -- -- -- -- -- -- -- -- -- -- */
	_pDBus->NSGKDBus::Callback<SIGas2v>::exposeSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		"DevicesStarted",
		{	{"as", "", "in", "array of started devices ID strings"} },
		std::bind(&DBusHandler::devicesStarted, this, std::placeholders::_1)
	);

	_pDBus->NSGKDBus::Callback<SIGas2v>::exposeSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		"DevicesStopped",
		{	{"as", "", "in", "array of stopped devices ID strings"} },
		std::bind(&DBusHandler::devicesStopped, this, std::placeholders::_1)
	);

	_pDBus->NSGKDBus::Callback<SIGas2v>::exposeSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		"DevicesUnplugged",
		{	{"as", "", "in", "array of unplugged devices ID strings"} },
		std::bind(&DBusHandler::devicesUnplugged, this, std::placeholders::_1)
	);

	_pDBus->NSGKDBus::Callback<SIGsm2v>::exposeSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		"DeviceMBankSwitch",
		{	{"s", "device_id", "in", "device ID"},
			{"y", "macro_bankID", "in", "macro bankID"}
		},
		std::bind(&DBusHandler::deviceMBankSwitch, this,
			std::placeholders::_1, std::placeholders::_2
		)
	);

	_pDBus->NSGKDBus::Callback<SIGsGM2v>::exposeSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		"DeviceMacroRecorded",
		{	{"s", "device_id", "in", "device ID"},
			{"y", "macro_keyID", "in", "macro key ID"},
			{"a(yyq)", "macro_array", "in", "macro array"}
		},
		std::bind(&DBusHandler::deviceMacroRecorded, this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3
		)
	);

	_pDBus->NSGKDBus::Callback<SIGsG2v>::exposeSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		"DeviceMacroCleared",
		{	{"s", "device_id", "in", "device ID"},
			{"y", "macro_keyID", "in", "macro key ID"}
		},
		std::bind(&DBusHandler::deviceMacroCleared, this,
			std::placeholders::_1, std::placeholders::_2
		)
	);

	_pDBus->NSGKDBus::Callback<SIGsG2v>::exposeSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		"DeviceGKeyEvent",
		{	{"s", "device_id", "in", "device ID"},
			{"y", "macro_keyID", "in", "macro key ID"}
		},
		std::bind(&DBusHandler::deviceGKeyEvent, this,
			std::placeholders::_1, std::placeholders::_2
		)
	);

	_pDBus->NSGKDBus::Callback<SIGss2v>::exposeSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		"DeviceMediaEvent",
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
	_pDBus->NSGKDBus::Callback<SIGv2v>::exposeSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
		"DaemonIsStopping",
		{},
		std::bind(&DBusHandler::daemonIsStopping, this)
	);

	_pDBus->NSGKDBus::Callback<SIGv2v>::exposeSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
		"DaemonIsStarting",
		{},
		std::bind(&DBusHandler::daemonIsStarting, this)
	);

	_pDBus->NSGKDBus::Callback<SIGv2v>::exposeSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
		"ReportYourself",
		{},
		std::bind(&DBusHandler::reportChangedState, this)
	);

	/* -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
	/*   GUISessionMessageHandler GUI requests D-Bus object  */
	/* -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
	_pDBus->NSGKDBus::Callback<SIGss2v>::exposeSignal(
		_sessionBus,
		GLOGIK_DESKTOP_QT5_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DESKTOP_QT5_SESSION_DBUS_OBJECT,
		GLOGIK_DESKTOP_QT5_SESSION_DBUS_INTERFACE,
		"DeviceStatusChangeRequest",
		{	{"s", "device_id", "in", "device ID"},
			{"s", "wanted_status", "in", "wanted status"}	},
		std::bind(&DBusHandler::deviceStatusChangeRequest, this,
		std::placeholders::_1, std::placeholders::_2)
	);
}

void DBusHandler::initializeGKDBusMethods(void)
{
	const std::string r_ed("reserved");

	_pDBus->NSGKDBus::Callback<SIGs2as>::exposeMethod(
		_sessionBus,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		"GetDevicesList",
		{	{"s", r_ed, "in", r_ed},
			{"as", "array_of_strings", "out", "array of devices ID and configuration files"} },
		std::bind(&DBusHandler::getDevicesList, this, r_ed) );

	_pDBus->NSGKDBus::Callback<SIGs2as>::exposeMethod(
		_sessionBus,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		"GetInformations",
		{	{"s", r_ed, "in", r_ed},
			{"as", "array_of_strings", "out", "array of informations strings"} },
		std::bind(&DBusHandler::getInformations, this, r_ed) );

	_pDBus->NSGKDBus::Callback<SIGss2aP>::exposeMethod(
		_sessionBus,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		"GetDeviceLCDPluginsProperties",
		{	{"s", "device_id", "in", "device ID"},
			{"s", r_ed, "in", r_ed},
			{"a(tss)", "get_lcd_plugins_properties_array", "out", "LCDPluginsProperties array"} },
		std::bind(&DBusHandler::getDeviceLCDPluginsProperties, this, std::placeholders::_1, r_ed) );
}

void DBusHandler::daemonIsStopping(void)
{
	GK_LOG_FUNC

	LOG(info) << "received signal : " << __func__;

	this->clearAndUnregister();
}

void DBusHandler::daemonIsStarting(void)
{
	GK_LOG_FUNC

	if( _registerStatus ) {
		GKLog4(trace,
			"received signal : ", __func__,
			"but we are already registered with daemon : ", _currentSession
		)
	}
	else {
		LOG(info)	<< "received signal : " << __func__
					<< " - contacting the daemon";

		try {
			try {
				this->registerWithDaemon();
			}
			catch ( const restartRequest & e ) {
				_wantToExit = true;
				this->sendRestartRequest(); /* throws */
			}
		}
		catch (const GLogiKExcept & e) {
			LOG(info) << e.what();
		}

		if( _registerStatus ) {
			this->updateSessionState();
			_devices.setClientID(_clientID);

			this->initializeDevices();
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

	const std::string remoteMethod("GetDeviceStatus");

	bool devicesUpdated(false);

	for(const auto& devID : devicesID) {
		try {
			_pDBus->initializeRemoteMethodCall(
				_systemBus,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			_pDBus->appendStringToRemoteMethodCall(_clientID);
			_pDBus->appendStringToRemoteMethodCall(devID);
			_pDBus->sendRemoteMethodCall();

			try {
				_pDBus->waitForRemoteMethodCallReply();

				const std::string deviceStatus( _pDBus->getNextStringArgument() );
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
			_pDBus->abandonRemoteMethodCall();
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

	const std::string remoteMethod("GetDeviceStatus");

	bool devicesUpdated(false);

	for(const auto& devID : devicesID) {
		try {
			_pDBus->initializeRemoteMethodCall(
				_systemBus,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			_pDBus->appendStringToRemoteMethodCall(_clientID);
			_pDBus->appendStringToRemoteMethodCall(devID);
			_pDBus->sendRemoteMethodCall();

			try {
				_pDBus->waitForRemoteMethodCallReply();

				const std::string deviceStatus( _pDBus->getNextStringArgument() );
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
			_pDBus->abandonRemoteMethodCall();
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

	const std::string remoteMethod("GetDeviceStatus");

	bool devicesUpdated(false);

	for(const auto& devID : devicesID) {
		try {
			_pDBus->initializeRemoteMethodCall(
				_systemBus,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			_pDBus->appendStringToRemoteMethodCall(_clientID);
			_pDBus->appendStringToRemoteMethodCall(devID);
			_pDBus->sendRemoteMethodCall();

			try {
				_pDBus->waitForRemoteMethodCallReply();

				const std::string deviceStatus( _pDBus->getNextStringArgument() );
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
			_pDBus->abandonRemoteMethodCall();
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

	GKLog4(trace,
		devID, " received signal : DeviceMBankSwitch",
		"bankID : ", bankID
	)

	if( ! _registerStatus ) {
		GKLog(trace, "currently not registered, skipping")
		return;
	}

	if( _sessionState != "active" ) {
		GKLog(trace, "currently not active, skipping")
		return;
	}

	LOG(info) << "received DeviceMBankSwitch signal : " << bankID;

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

	GKLog4(trace,
		devID, " received signal : DeviceMacroRecorded",
		"key : ", getGKeyName(keyID)
	)

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

	GKLog4(trace,
		devID, " received signal : DeviceMacroCleared",
		"key : ", getGKeyName(keyID)
	)

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

	GKLog4(trace,
		devID, " received signal : DeviceMediaEvent",
		"event : ", mediaKeyEvent
	)

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

	GKLog4(trace,
		devID, " received signal : DeviceGKeyEvent",
		"key : ", getGKeyName(keyID)
	)

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

		_GKeysEvent.runMacro(banksMap, bankID, keyID);
	}
	catch (const GLogiKExcept & e) {
		LOG(error) << devID << " run macro failure - " << keyID;
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

const LCDPluginsPropertiesArray_type & DBusHandler::getDeviceLCDPluginsProperties(
	const std::string & devID,
	const std::string & reserved)
{
	return _devices.getDeviceLCDPluginsProperties(devID);
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

	if(	(remoteMethod !=  "StopDevice") and
		(remoteMethod !=  "StartDevice") and
		(remoteMethod !=  "RestartDevice") ) {
		LOG(warning) << devID << " ignoring wrong remote method : " << remoteMethod;
		return;
	}

	try {
		_pDBus->initializeRemoteMethodCall(
			_systemBus,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		_pDBus->appendStringToRemoteMethodCall(_clientID);
		_pDBus->appendStringToRemoteMethodCall(devID);
		_pDBus->sendRemoteMethodCall();

		try {
			_pDBus->waitForRemoteMethodCallReply();

			const bool ret = _pDBus->getNextBooleanArgument();
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
		_pDBus->abandonRemoteMethodCall();
		LogRemoteCallFailure
	}
}

} // namespace GLogiK

