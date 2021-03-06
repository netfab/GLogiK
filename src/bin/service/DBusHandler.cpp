/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2020  Fabrice Delliaux <netbox253@gmail.com>
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
						LOG(INFO) << "register retry " << retries << " ...";
					}

					this->registerWithDaemon();

					if( ! _registerStatus ) {
						unsigned int timer = 5;
						LOG(WARNING) << "retrying in " << timer << " seconds ...";
						std::this_thread::sleep_for(std::chrono::seconds(timer));
					}
				}
				else {
					LOG(ERROR) << "can't register, giving up";
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

DBusHandler::~DBusHandler() {
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
	devices_files_map_t devicesMap = _devices.getDevicesMap();

	pGKfs->readNotifyEvents( devicesMap );

	for( const auto & device : devicesMap ) {
#if DEBUGGING_ON
		LOG(DEBUG) << device.first << " reload configuration file: " << device.second;
#endif
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
	if( _registerStatus ) {
		_devices.clearDevices();
		this->unregisterWithDaemon();
		/* send signal to GUI */
		this->sendDevicesUpdatedSignal();
	}
	else {
#if DEBUGGING_ON
		LOG(DEBUG2) << "client " << _currentSession << " already unregistered with deamon";
#endif
	}
}

void DBusHandler::cleanDBusRequests(void) {
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
			LOG(WARNING) << "unknown session tracker";
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
void DBusHandler::registerWithDaemon(void) {
	if( _registerStatus ) {
		LOG(WARNING) << "don't need to register, since we are already registered";
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

					LOG(INFO) << "successfully registered with daemon - " << _clientID;
				}
				catch (const GLogiKExcept & e) {
					LOG(WARNING) << e.what() << " - will unregister";
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
					LOG(ERROR) << e.what();
				}
				LOG(ERROR) << "failed to register with daemon : false - " << reason;
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

void DBusHandler::unregisterWithDaemon(void) {
	if( ! _registerStatus ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "can't unregister, since we are not currently registered";
#endif
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
				LOG(INFO) << "successfully unregistered with daemon";
			}
			else {
				LOG(ERROR) << "failed to unregister with daemon : false";
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

void DBusHandler::setCurrentSessionObjectPath(pid_t pid) {
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
#if DEBUGGING_ON
				LOG(DEBUG1) << "current session : " << _currentSession;
#endif
				_sessionFramework = SessionFramework::FW_LOGIND;

				/* update session state when PropertyChanged signal receipted */
				const std::string object = _pDBus->getObjectFromObjectPath(_currentSession);

				_pDBus->NSGKDBus::EventGKDBusCallback<VoidToVoid>::exposeSignal(
					_systemBus,
					"org.freedesktop.login1",
					object.c_str(),
					"org.freedesktop.DBus.Properties",
					"PropertiesChanged",
					{},
					std::bind(&DBusHandler::updateSessionState, this)
				);

				LOG(INFO) << "successfully contacted logind";
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
		LOG(ERROR) << e.what();
	}

	/* fatal error */
	throw GLogiKExcept("unable to contact a session manager");
}

void DBusHandler::updateSessionState(void)
{
	_sessionState = this->getCurrentSessionState();
#if DEBUGGING_ON
	LOG(DEBUG1) << "switching session state to : " << _sessionState;
#endif
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
const std::string DBusHandler::getCurrentSessionState(void) {
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

void DBusHandler::reportChangedState(void) {
	if( ! _registerStatus ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "currently not registered, skipping report state";
#endif
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
			if( ret ) {
#if DEBUGGING_ON
				LOG(DEBUG2) << "successfully reported changed state : " << _sessionState;
#endif
			}
			else {
				LOG(ERROR) << "failed to report changed state : false";
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

void DBusHandler::initializeDevices(void) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "initializing devices";
#endif

	if( ! _registerStatus ) {
#if DEBUGGING_ON
		LOG(DEBUG3) << "currently not registered, giving up";
#endif
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
			if( ret ) {
#if DEBUGGING_ON
				LOG(DEBUG2) << "successfully toggled ready propertie";
#endif
			}
			else {
				LOG(WARNING) << "toggling ready propertie failed : false";
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
	std::string status("restart request");
	try {
		/* asking the launcher for a restart */
		_pDBus->initializeBroadcastSignal(
			_sessionBus,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
			"RestartRequest"
		);
		_pDBus->sendBroadcastSignal();

		status += " sent";
		LOG(WARNING) << status;
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_pDBus->abandonBroadcastSignal();
		status += " failure";
		LOG(ERROR) << status << " - " << e.what();
	}

	throw GLogiKExcept(status);
}

void DBusHandler::sendDevicesUpdatedSignal(void)
{
	std::string status("devices updated signal");
	try {
		/* send DevicesUpdated signal to GUI applications */
		_pDBus->initializeBroadcastSignal(
			_sessionBus,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
			"DevicesUpdated"
		);
		_pDBus->sendBroadcastSignal();

		status += " sent";
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_pDBus->abandonBroadcastSignal();
		status += " failure";
		LOG(ERROR) << status << " - " << e.what();
	}

#if DEBUGGING_ON
	LOG(DEBUG3) << status << " on session bus";
#endif
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

void DBusHandler::initializeGKDBusSignals(void) {
	/*
	 * want to be warned by the daemon about those signals
	 * each declaration can throw a GLogiKExcept exception
	 * if something is wrong (handled in constructor)
	 */

	/* -- -- -- -- -- -- -- -- -- -- */
	/*  DevicesManager D-Bus object  */
	/* -- -- -- -- -- -- -- -- -- -- */
	_pDBus->NSGKDBus::EventGKDBusCallback<StringsArrayToVoid>::exposeSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		"DevicesStarted",
		{	{"as", "", "in", "array of started devices ID strings"} },
		std::bind(&DBusHandler::devicesStarted, this, std::placeholders::_1)
	);

	_pDBus->NSGKDBus::EventGKDBusCallback<StringsArrayToVoid>::exposeSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		"DevicesStopped",
		{	{"as", "", "in", "array of stopped devices ID strings"} },
		std::bind(&DBusHandler::devicesStopped, this, std::placeholders::_1)
	);

	_pDBus->NSGKDBus::EventGKDBusCallback<StringsArrayToVoid>::exposeSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		"DevicesUnplugged",
		{	{"as", "", "in", "array of unplugged devices ID strings"} },
		std::bind(&DBusHandler::devicesUnplugged, this, std::placeholders::_1)
	);

	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsOneByteToBool>::exposeSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		"MacroRecorded",
		{	{"s", "device_id", "in", "device ID"},
			{"s", "macro_key_name", "in", "macro key name"},
			{"y", "macro_bankID", "in", "macro bankID"} },
		std::bind(&DBusHandler::macroRecorded, this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3
		)
	);

	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsOneByteToBool>::exposeSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		"MacroCleared",
		{	{"s", "device_id", "in", "device ID"},
			{"s", "macro_key_name", "in", "macro key name"},
			{"y", "macro_bankID", "in", "macro bankID"} },
		std::bind(&DBusHandler::macroCleared, this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3
		)
	);

	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsToVoid>::exposeSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
		"deviceMediaEvent",
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
	_pDBus->NSGKDBus::EventGKDBusCallback<VoidToVoid>::exposeSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
		"DaemonIsStopping",
		{},
		std::bind(&DBusHandler::daemonIsStopping, this)
	);

	_pDBus->NSGKDBus::EventGKDBusCallback<VoidToVoid>::exposeSignal(
		_systemBus,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
		"DaemonIsStarting",
		{},
		std::bind(&DBusHandler::daemonIsStarting, this)
	);

	_pDBus->NSGKDBus::EventGKDBusCallback<VoidToVoid>::exposeSignal(
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
	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsToVoid>::exposeSignal(
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

	_pDBus->NSGKDBus::EventGKDBusCallback<StringToStringsArray>::exposeMethod(
		_sessionBus,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		"GetDevicesList",
		{	{"s", r_ed, "in", r_ed},
			{"as", "array_of_strings", "out", "array of devices ID and configuration files"} },
		std::bind(&DBusHandler::getDevicesList, this, r_ed) );

	_pDBus->NSGKDBus::EventGKDBusCallback<StringToStringsArray>::exposeMethod(
		_sessionBus,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		"GetInformations",
		{	{"s", r_ed, "in", r_ed},
			{"as", "array_of_strings", "out", "array of informations strings"} },
		std::bind(&DBusHandler::getInformations, this, r_ed) );

	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsToLCDPluginsPropertiesArray>::exposeMethod(
		_sessionBus,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		"GetDeviceLCDPluginsProperties",
		{	{"s", "device_id", "in", "device ID"},
			{"s", r_ed, "in", r_ed},
			{"a(tss)", "get_lcd_plugins_properties_array", "out", "LCDPluginsProperties array"} },
		std::bind(&DBusHandler::getDeviceLCDPluginsProperties, this, std::placeholders::_1, r_ed) );
}

void DBusHandler::daemonIsStopping(void) {
	std::ostringstream buffer("received signal: ", std::ios_base::app);
	buffer << __func__;
	LOG(INFO) << buffer.str();
	this->clearAndUnregister();
}

void DBusHandler::daemonIsStarting(void) {
	std::ostringstream buffer("received signal: ", std::ios_base::app);
	buffer << __func__;
	if( _registerStatus ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << buffer.str()
			<< " - but we are already registered with daemon : "
			<< _currentSession;
#endif
	}
	else {
		LOG(INFO)	<< buffer.str()
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
			LOG(WARNING) << e.what();
		}

		if( _registerStatus ) {
			this->updateSessionState();
			_devices.setClientID(_clientID);

			this->initializeDevices();
		}
	}
}

void DBusHandler::devicesStarted(const std::vector<std::string> & devicesID) {
#if DEBUGGING_ON
	LOG(DEBUG3) << "received " << __func__ << " signal"
				<< " - with " << devicesID.size() << " devices";
#endif
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
#if DEBUGGING_ON
					LOG(DEBUG3) << "device status from daemon: " << devID << " started";
#endif
					_devices.startDevice(devID);

					devicesUpdated = true;
				}
				else {
					LOG(WARNING) << "received devicesStarted signal for device " << devID;
					LOG(WARNING) << "but daemon is saying that device status is : " << deviceStatus;
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

void DBusHandler::devicesStopped(const std::vector<std::string> & devicesID) {
#if DEBUGGING_ON
	LOG(DEBUG3) << "received " << __func__ << " signal"
				<< " - with " << devicesID.size() << " devices";
#endif
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
#if DEBUGGING_ON
					LOG(DEBUG3) << "device status from daemon: " << devID << " stopped";
#endif
					_devices.stopDevice(devID);

					devicesUpdated = true;
				}
				else {
					LOG(WARNING) << "received devicesStopped signal for device " << devID;
					LOG(WARNING) << "but daemon is saying that device status is : " << deviceStatus;
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

void DBusHandler::devicesUnplugged(const std::vector<std::string> & devicesID) {
#if DEBUGGING_ON
	LOG(DEBUG3) << "received " << __func__ << " signal"
				<< " - with " << devicesID.size() << " devices";
#endif
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
#if DEBUGGING_ON
					LOG(DEBUG3) << "device status from daemon: " << devID << " unplugged";
#endif
					_devices.unplugDevice(devID);

					devicesUpdated = true;
				}
				else {
					LOG(WARNING) << "received devicesUnplugged signal for device " << devID;
					LOG(WARNING) << "but daemon is saying that device status is : " << deviceStatus;
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

const bool DBusHandler::macroRecorded(
	const std::string & devID,
	const std::string & keyName,
	const uint8_t bankID)
{
#if DEBUGGING_ON
	LOG(DEBUG3) << "received " << __func__ << " signal"
				<< " - device: " << devID
				<< " bankID: " << toUInt(bankID)
				<< " key: " << keyName;
#endif

	if( ! _registerStatus ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "currently not registered, skipping";
#endif
		return false;
	}

	if( _sessionState != "active" ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "currently not active, skipping";
#endif
		return false;
	}

	return _devices.setDeviceMacro(devID, keyName, bankID);
}

const bool DBusHandler::macroCleared(
	const std::string & devID,
	const std::string & keyName,
	const uint8_t bankID)
{
#if DEBUGGING_ON
	LOG(DEBUG3) << "received " << __func__ << " signal"
				<< " - device: " << devID
				<< " bankID: " << toUInt(bankID)
				<< " key: " << keyName;
#endif

	if( ! _registerStatus ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "currently not registered, skipping";
#endif
		return false;
	}

	if( _sessionState != "active" ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "currently not active, skipping";
#endif
		return false;
	}

	return _devices.clearDeviceMacro(devID, keyName, bankID);
}

void DBusHandler::deviceMediaEvent(
	const std::string & devID,
	const std::string & mediaKeyEvent)
{
#if DEBUGGING_ON
	LOG(DEBUG3) << "received " << __func__ << " signal"
				<< " - device: " << devID
				<< " event: " << mediaKeyEvent;
#endif

	if( ! _registerStatus ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "currently not registered, skipping";
#endif
		return;
	}

	if( _sessionState != "active" ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "currently not active, skipping";
#endif
		return;
	}

	_devices.doDeviceFakeKeyEvent(devID, mediaKeyEvent);
}

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
#if DEBUGGING_ON
	LOG(DEBUG3) << "received " << remoteMethod << " signal for device " << devID;
#endif

	if(	(remoteMethod !=  "StopDevice") and
		(remoteMethod !=  "StartDevice") and
		(remoteMethod !=  "RestartDevice") ) {
		LOG(WARNING) << "ignoring wrong remoteMethod : " << remoteMethod;
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
			if( ret ) {
#if DEBUGGING_ON
				LOG(DEBUG2) << remoteMethod << " successful for device " << devID;
#endif
			}
			else {
				LOG(INFO) << remoteMethod << " failure for device " << devID << " : false";
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

