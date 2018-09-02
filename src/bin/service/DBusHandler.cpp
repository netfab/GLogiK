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

#include <sstream>
#include <new>
#include <functional>
#include <stdexcept>
#include <thread>
#include <chrono>

#include "lib/shared/glogik.hpp"

#include "global.hpp"
#include "DBusHandler.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

DBusHandler::DBusHandler(
	pid_t pid,
	SessionManager& session,
	NSGKUtils::FileSystem* pGKfs)
	:	_pDBus(nullptr),
		_systemBus(NSGKDBus::BusConnection::GKDBUS_SYSTEM),
		_skipRetry(false),
		_registerStatus(false),
		_clientID("undefined"),
		_sessionFramework(SessionFramework::FW_UNKNOWN)
{
	_devices.setGKfs(pGKfs);

	try {
		try {
			try {
				_pDBus = new NSGKDBus::GKDBus(GLOGIK_DESKTOP_SERVICE_DBUS_ROOT_NODE);
			}
			catch (const std::bad_alloc& e) { /* handle new() failure */
				throw GLogiKBadAlloc("GKDBus bad allocation");
			}

			_pDBus->connectToSystemBus(GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME);

			this->setCurrentSessionObjectPath(pid);

			unsigned int retries = 0;
			while( ! _registerStatus ) {
				if( ( session.isSessionAlive() )
						and (retries < UNREACHABLE_DAEMON_MAX_RETRIES)
						and ( ! _skipRetry ) )
				{
					if(retries > 0) {
						LOG(INFO) << "register retry " << retries << " ...";
					}

					try {
						this->registerWithDaemon();
					}
					catch( const GLogiKExcept & e ) {
						if( ! _skipRetry ) {
							unsigned int timer = 5;
							LOG(WARNING) << e.what() << ", retrying in " << timer << " seconds ...";
							std::this_thread::sleep_for(std::chrono::seconds(timer));
						}
					}
				}
				else {
					LOG(ERROR) << "can't register, giving up";
					throw GLogiKExcept("unable to register with daemon");
					break;
				}
				retries++;
			}

			_sessionState = this->getCurrentSessionState();
			this->reportChangedState();

			this->initializeGKDBusSignals();

			/* set GKDBus pointer */
			_devices.setDBus(_pDBus);
			_devices.setClientID(_clientID);

			this->initializeDevices();

		}
		catch ( const GLogiKExcept & e ) {
			this->unregisterWithDaemon();
			delete _pDBus; _pDBus = nullptr;
			throw;
		}
	}
	catch ( const GLogiKFatalError & e ) {
		/* don't try to unregister in case of fatal error,
		 * another GLogiKFatalError exception could be thrown */
		delete _pDBus; _pDBus = nullptr;
		throw;
	}
}

DBusHandler::~DBusHandler() {
	if( _registerStatus ) {
		try {
			_devices.clearDevices();
			this->unregisterWithDaemon();
		}
		catch ( const GLogiKFatalError & e ) {
			// giving up
		}
	}
	else {
#if DEBUGGING_ON
		LOG(DEBUG2) << "client " << _currentSession << " already unregistered with deamon";
#endif
	}

	delete _pDBus; _pDBus = nullptr;
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

void DBusHandler::checkDBusMessages(void) {
	_pDBus->checkForNextMessage(_systemBus);
}

void DBusHandler::updateSessionState(void) {
	/* if debug output is ON, force-disable it, else debug file
	 * will be spammed by the following DBus request debug output */
	const bool disabledDebugOutput = true;
	const std::string newState = this->getCurrentSessionState(disabledDebugOutput);
	if(_sessionState == newState) {
#if 0 && DEBUGGING_ON
		LOG(DEBUG5) << "session state did not changed";
#endif
		return;
	}

#if DEBUGGING_ON
	LOG(DEBUG1) << "current session state : " << _sessionState;
#endif

	std::string unhandled = "unhandled session state : "; unhandled += newState;

	if( _sessionState == "active" ) {
		if(newState == "online") {
		}
		else if(newState == "closing") {
		}
		else {
			FATALERROR << unhandled;
			return;
		}
	}
	else if( _sessionState == "online" ) {
		if(newState == "active") {
		}
		else if(newState == "closing") {
		}
		else {
			FATALERROR << unhandled;
			return;
		}
	}
	else if( _sessionState == "closing" ) {
		if(newState == "active") {
		}
		else if(newState == "online") {
		}
		else {
			FATALERROR << unhandled;
			return;
		}
	}
	else {
		FATALERROR << unhandled;
		return;
	}

#if DEBUGGING_ON
	LOG(DEBUG1) << "switching session state to : " << newState;
#endif

	_sessionState = newState;
	this->reportChangedState();
}

const devices_files_map_t DBusHandler::getDevicesMap(void) {
	return _devices.getDevicesMap();
}

void DBusHandler::checkDeviceConfigurationFile(const std::string & devID) {
	_devices.checkDeviceConfigurationFile(devID);

	/*
	 * force state update, to load active user's parameters
	 * for all plugged devices
	 */
	if( _sessionState == "active" ) {
		this->reportChangedState();
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
 * try to tell the daemon we are alive
 * throws on failure
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
				_clientID = _pDBus->getNextStringArgument();
				_registerStatus = true;
				LOG(INFO) << "successfully registered with daemon - " << _clientID;
			}
			else {
				LOG(ERROR)	<< "failed to register with daemon : false";
				const std::string reason(_pDBus->getNextStringArgument());
				if(reason == "already registered")
					_skipRetry = true;
				throw GLogiKExcept(reason);
			}
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
			throw GLogiKExcept("RegisterClient failure");
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_pDBus->abandonRemoteMethodCall();
		LogRemoteCallFailure
		throw GLogiKExcept("RegisterClient call failure");
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
		const std::string remoteMethod("GetCurrentSession");

		try {
			/* getting consolekit current session */
			_pDBus->initializeRemoteMethodCall(
				_systemBus,
				"org.freedesktop.ConsoleKit",
				"/org/freedesktop/ConsoleKit/Manager",
				"org.freedesktop.ConsoleKit.Manager",
				remoteMethod.c_str()
			);
			_pDBus->sendRemoteMethodCall();

			try {
				_pDBus->waitForRemoteMethodCallReply();
				_currentSession = _pDBus->getNextStringArgument();
#if DEBUGGING_ON
				LOG(DEBUG1) << "current session : " << _currentSession;
#endif
				_sessionFramework = SessionFramework::FW_CONSOLEKIT;
				return;
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
				throw GLogiKExcept("failure to get session ID from consolekit");
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			_pDBus->abandonRemoteMethodCall();
			LogRemoteCallFailure
		}
	}
	catch ( const GLogiKExcept & e ) {
		LOG(ERROR) << e.what();
		LOG(INFO) << "trying to contact logind";

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
				return;
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

	/* fatal error */
	throw GLogiKExcept("unable to contact a session manager");
}

/*
 * Ask to session tracker the current session state, and returns it.
 * If the state fails to be updated for whatever reason, throws.
 * Possible returns values are :
 *  - active
 *  - online
 *  - closing
 */
const std::string DBusHandler::getCurrentSessionState(const bool disabledDebugOutput) {
	std::string remoteMethod;
	switch(_sessionFramework) {
		/* consolekit */
		case SessionFramework::FW_CONSOLEKIT:
			remoteMethod = "GetSessionState";
			try {
				_pDBus->initializeRemoteMethodCall(
					_systemBus,
					"org.freedesktop.ConsoleKit",
					_currentSession.c_str(),
					"org.freedesktop.ConsoleKit.Session",
					remoteMethod.c_str(),
					disabledDebugOutput
				);
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
		case SessionFramework::FW_LOGIND:
			/* logind */
			remoteMethod = "Get";
			try {
				_pDBus->initializeRemoteMethodCall(
					_systemBus,
					"org.freedesktop.login1",
					_currentSession.c_str(),
					"org.freedesktop.DBus.Properties",
					remoteMethod.c_str(),
					disabledDebugOutput
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
				LOG(DEBUG2) << "successfully toggled ready propertie";
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

	_pDBus->NSGKDBus::EventGKDBusCallback<StringsArrayToVoid>::exposeSignal(
		_systemBus,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"DevicesStarted",
		{	{"as", "", "in", "array of started devices ID strings"} },
		std::bind(&DBusHandler::devicesStarted, this, std::placeholders::_1)
	);

	_pDBus->NSGKDBus::EventGKDBusCallback<StringsArrayToVoid>::exposeSignal(
		_systemBus,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"DevicesStopped",
		{	{"as", "", "in", "array of stopped devices ID strings"} },
		std::bind(&DBusHandler::devicesStopped, this, std::placeholders::_1)
	);

	_pDBus->NSGKDBus::EventGKDBusCallback<StringsArrayToVoid>::exposeSignal(
		_systemBus,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"DevicesUnplugged",
		{	{"as", "", "in", "array of unplugged devices ID strings"} },
		std::bind(&DBusHandler::devicesUnplugged, this, std::placeholders::_1)
	);

	_pDBus->NSGKDBus::EventGKDBusCallback<VoidToVoid>::exposeSignal(
		_systemBus,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"DaemonIsStopping",
		{},
		std::bind(&DBusHandler::daemonIsStopping, this)
	);

	_pDBus->NSGKDBus::EventGKDBusCallback<VoidToVoid>::exposeSignal(
		_systemBus,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"DaemonIsStarting",
		{},
		std::bind(&DBusHandler::daemonIsStarting, this)
	);

	_pDBus->NSGKDBus::EventGKDBusCallback<VoidToVoid>::exposeSignal(
		_systemBus,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"ReportYourself",
		{},
		std::bind(&DBusHandler::reportChangedState, this)
	);

	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsOneByteToBool>::exposeSignal(
		_systemBus,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"MacroRecorded",
		{	{"s", "device_id", "in", "device ID"},
			{"s", "macro_key_name", "in", "macro key name"},
			{"y", "macro_bankID", "in", "macro bankID"} },
		std::bind(	&DBusHandler::macroRecorded, this,
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3
		)
	);

	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsOneByteToBool>::exposeSignal(
		_systemBus,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"MacroCleared",
		{	{"s", "device_id", "in", "device ID"},
			{"s", "macro_key_name", "in", "macro key name"},
			{"y", "macro_bankID", "in", "macro bankID"} },
		std::bind(	&DBusHandler::macroCleared, this,
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3
		)
	);

	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsToVoid>::exposeSignal(
		_systemBus,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"deviceMediaEvent",
		{	{"s", "device_id", "in", "device ID"},
			{"s", "media_key_event", "in", "media key event"}
		},
		std::bind(	&DBusHandler::deviceMediaEvent, this,
					std::placeholders::_1, std::placeholders::_2
		)
	);
}

void DBusHandler::daemonIsStopping(void) {
	std::ostringstream buffer("received signal: ", std::ios_base::app);
	buffer << __func__;
	if( _registerStatus ) {
		LOG(INFO)	<< buffer.str()
					<< " - saving state and unregistering with daemon";
		_devices.clearDevices();
		this->unregisterWithDaemon();
	}
	else {
#if DEBUGGING_ON
		LOG(DEBUG2) << buffer.str()
					<< " - but we are NOT registered with daemon : "
					<< _currentSession;
#endif
	}
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
					<< " - trying to contact the daemon";
		this->registerWithDaemon();
		_sessionState = this->getCurrentSessionState();
		this->reportChangedState();
		_devices.setClientID(_clientID);

		this->initializeDevices();
	}
}

void DBusHandler::devicesStarted(const std::vector<std::string> & devicesID) {
#if DEBUGGING_ON
	LOG(DEBUG3) << "received " << __func__ << " signal"
				<< " - with " << devicesID.size() << " devices";
#endif
	const std::string remoteMethod("GetDeviceStatus");
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
}

void DBusHandler::devicesUnplugged(const std::vector<std::string> & devicesID) {
#if DEBUGGING_ON
	LOG(DEBUG3) << "received " << __func__ << " signal"
				<< " - with " << devicesID.size() << " devices";
#endif
	const std::string remoteMethod("GetDeviceStatus");
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
}

const bool DBusHandler::macroRecorded(
	const std::string & devID,
	const std::string & keyName,
	const uint8_t bankID)
{
#if DEBUGGING_ON
	LOG(DEBUG3) << "received " << __func__ << " signal"
				<< " - device: " << devID
				<< " bankID: " << to_uint(bankID)
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
				<< " bankID: " << to_uint(bankID)
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

	_devices.runDeviceMediaEvent(devID, mediaKeyEvent);
}

} // namespace GLogiK

