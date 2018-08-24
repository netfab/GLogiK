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

#include <new>
#include <functional>
#include <stdexcept>
#include <thread>
#include <chrono>

#include "lib/shared/glogik.h"

#include "global.h"
#include "DBusHandler.h"

namespace GLogiK
{

using namespace NSGKUtils;

DBusHandler::DBusHandler(
	pid_t pid,
	SessionManager& session,
	NSGKUtils::FileSystem* pGKfs
	)
	:	pDBus_(nullptr),
		system_bus_(NSGKDBus::BusConnection::GKDBUS_SYSTEM),
		_skipRetry(false),
		_registerStatus(false),
		_clientID("undefined"),
		_sessionFramework(SessionFramework::FW_UNKNOWN),
		buffer_("", std::ios_base::app)
{
	this->devices_.setGKfs(pGKfs);

	try {
		try {
			try {
				this->pDBus_ = new NSGKDBus::GKDBus(GLOGIK_DESKTOP_SERVICE_DBUS_ROOT_NODE);
			}
			catch (const std::bad_alloc& e) { /* handle new() failure */
				throw GLogiKBadAlloc("GKDBus bad allocation");
			}

			this->pDBus_->connectToSystemBus(GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME);

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
			this->devices_.setDBus(this->pDBus_);
			this->devices_.setClientID(_clientID);

			this->initializeDevices();

		}
		catch ( const GLogiKExcept & e ) {
			this->unregisterWithDaemon();
			delete this->pDBus_;
			this->pDBus_ = nullptr;
			throw;
		}
	}
	catch ( const GLogiKFatalError & e ) {
		/* don't try to unregister in case of fatal error,
		 * another GLogiKFatalError exception could be thrown */
		delete this->pDBus_;
		this->pDBus_ = nullptr;
		throw;
	}
}

DBusHandler::~DBusHandler() {
	if( _registerStatus ) {
		try {
			this->devices_.clearDevices();
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

	delete this->pDBus_;
	this->pDBus_ = nullptr;
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
	this->pDBus_->checkForNextMessage(this->system_bus_);
}

void DBusHandler::updateSessionState(void) {
	/* if debug output is ON, force-disable it, else debug file
	 * will be spammed by the following DBus request debug output */
	const bool disabledDebugOutput = true;
	const std::string new_state = this->getCurrentSessionState(disabledDebugOutput);
	if(_sessionState == new_state) {
#if 0 && DEBUGGING_ON
		LOG(DEBUG5) << "session state did not changed";
#endif
		return;
	}

#if DEBUGGING_ON
	LOG(DEBUG1) << "current session state : " << _sessionState;
#endif

	std::string unhandled = "unhandled session state : "; unhandled += new_state;

	if( _sessionState == "active" ) {
		if(new_state == "online") {
		}
		else if(new_state == "closing") {
		}
		else {
			FATALERROR << unhandled;
			return;
		}
	}
	else if( _sessionState == "online" ) {
		if(new_state == "active") {
		}
		else if(new_state == "closing") {
		}
		else {
			FATALERROR << unhandled;
			return;
		}
	}
	else if( _sessionState == "closing" ) {
		if(new_state == "active") {
		}
		else if(new_state == "online") {
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
	LOG(DEBUG1) << "switching session state to : " << new_state;
#endif

	_sessionState = new_state;
	this->reportChangedState();
}

const devices_files_map_t DBusHandler::getDevicesMap(void) {
	return this->devices_.getDevicesMap();
}

void DBusHandler::checkDeviceConfigurationFile(const std::string & devID) {
	this->devices_.checkDeviceConfigurationFile(devID);

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
		this->pDBus_->initializeRemoteMethodCall(
			this->system_bus_,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		this->pDBus_->appendStringToRemoteMethodCall(_currentSession);
		this->pDBus_->sendRemoteMethodCall();

		/* -- */

		try {
			this->pDBus_->waitForRemoteMethodCallReply();

			const bool ret = this->pDBus_->getNextBooleanArgument();
			if( ret ) {
				_clientID = this->pDBus_->getNextStringArgument();
				_registerStatus = true;
				LOG(INFO) << "successfully registered with daemon - " << _clientID;
			}
			else {
				LOG(ERROR)	<< "failed to register with daemon : false";
				const std::string reason(this->pDBus_->getNextStringArgument());
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
		this->pDBus_->abandonRemoteMethodCall();
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
		this->pDBus_->initializeRemoteMethodCall(
			this->system_bus_,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		this->pDBus_->appendStringToRemoteMethodCall(_clientID);
		this->pDBus_->sendRemoteMethodCall();

		try {
			this->pDBus_->waitForRemoteMethodCallReply();

			const bool ret = this->pDBus_->getNextBooleanArgument();
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
		this->pDBus_->abandonRemoteMethodCall();
		LogRemoteCallFailure
	}
}

void DBusHandler::setCurrentSessionObjectPath(pid_t pid) {
	try {
		const std::string remoteMethod("GetCurrentSession");

		try {
			/* getting consolekit current session */
			this->pDBus_->initializeRemoteMethodCall(
				this->system_bus_,
				"org.freedesktop.ConsoleKit",
				"/org/freedesktop/ConsoleKit/Manager",
				"org.freedesktop.ConsoleKit.Manager",
				remoteMethod.c_str()
			);
			this->pDBus_->sendRemoteMethodCall();

			try {
				this->pDBus_->waitForRemoteMethodCallReply();
				_currentSession = this->pDBus_->getNextStringArgument();
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
			this->pDBus_->abandonRemoteMethodCall();
			LogRemoteCallFailure
		}
	}
	catch ( const GLogiKExcept & e ) {
		LOG(ERROR) << e.what();
		LOG(INFO) << "trying to contact logind";

		const std::string remoteMethod("GetSessionByPID");

		try {
			/* getting logind current session */
			this->pDBus_->initializeRemoteMethodCall(
				this->system_bus_,
				"org.freedesktop.login1",
				"/org/freedesktop/login1",
				"org.freedesktop.login1.Manager",
				"GetSessionByPID"
			);
			this->pDBus_->appendUInt32ToRemoteMethodCall(pid);
			this->pDBus_->sendRemoteMethodCall();

			try {
				this->pDBus_->waitForRemoteMethodCallReply();
				_currentSession = this->pDBus_->getNextStringArgument();
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
			this->pDBus_->abandonRemoteMethodCall();
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
				this->pDBus_->initializeRemoteMethodCall(
					this->system_bus_,
					"org.freedesktop.ConsoleKit",
					_currentSession.c_str(),
					"org.freedesktop.ConsoleKit.Session",
					remoteMethod.c_str(),
					disabledDebugOutput
				);
				this->pDBus_->sendRemoteMethodCall();

				try {
					this->pDBus_->waitForRemoteMethodCallReply();
					return this->pDBus_->getNextStringArgument();
				}
				catch (const GLogiKExcept & e) {
					LogRemoteCallGetReplyFailure
				}
			}
			catch (const GKDBusMessageWrongBuild & e) {
				this->pDBus_->abandonRemoteMethodCall();
				LogRemoteCallFailure
			}
			break;
		case SessionFramework::FW_LOGIND:
			/* logind */
			remoteMethod = "Get";
			try {
				this->pDBus_->initializeRemoteMethodCall(
					this->system_bus_,
					"org.freedesktop.login1",
					_currentSession.c_str(),
					"org.freedesktop.DBus.Properties",
					remoteMethod.c_str(),
					disabledDebugOutput
				);
				this->pDBus_->appendStringToRemoteMethodCall("org.freedesktop.login1.Session");
				this->pDBus_->appendStringToRemoteMethodCall("State");
				this->pDBus_->sendRemoteMethodCall();

				try {
					this->pDBus_->waitForRemoteMethodCallReply();
					return this->pDBus_->getNextStringArgument();
				}
				catch (const GLogiKExcept & e) {
					LogRemoteCallGetReplyFailure
				}
			}
			catch (const GKDBusMessageWrongBuild & e) {
				this->pDBus_->abandonRemoteMethodCall();
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
		this->pDBus_->initializeRemoteMethodCall(
			this->system_bus_,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		this->pDBus_->appendStringToRemoteMethodCall(_clientID);
		this->pDBus_->appendStringToRemoteMethodCall(_sessionState);
		this->pDBus_->sendRemoteMethodCall();

		try {
			this->pDBus_->waitForRemoteMethodCallReply();
			const bool ret( this->pDBus_->getNextBooleanArgument() );
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
		this->pDBus_->abandonRemoteMethodCall();
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
		this->pDBus_->initializeRemoteMethodCall(
			this->system_bus_,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		this->pDBus_->appendStringToRemoteMethodCall(_clientID);
		this->pDBus_->sendRemoteMethodCall();

		try {
			this->pDBus_->waitForRemoteMethodCallReply();

			devicesID = this->pDBus_->getStringsArray();
			this->devicesStarted(devicesID);
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		this->pDBus_->abandonRemoteMethodCall();
		LogRemoteCallFailure
	}

	devicesID.clear();

	remoteMethod = "ToggleClientReadyPropertie";

	/* saying the daemon that we are ready */
	try {
		this->pDBus_->initializeRemoteMethodCall(
			this->system_bus_,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		this->pDBus_->appendStringToRemoteMethodCall(_clientID);
		this->pDBus_->sendRemoteMethodCall();

		try {
			this->pDBus_->waitForRemoteMethodCallReply();

			const bool ret = this->pDBus_->getNextBooleanArgument();
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
		this->pDBus_->abandonRemoteMethodCall();
		LogRemoteCallFailure
	}

	remoteMethod = "GetStoppedDevices";

	/* stopped devices */
	try {
		this->pDBus_->initializeRemoteMethodCall(
			this->system_bus_,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		this->pDBus_->appendStringToRemoteMethodCall(_clientID);
		this->pDBus_->sendRemoteMethodCall();

		try {
			this->pDBus_->waitForRemoteMethodCallReply();

			devicesID = this->pDBus_->getStringsArray();
			this->devicesStopped(devicesID);
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		this->pDBus_->abandonRemoteMethodCall();
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

	this->pDBus_->NSGKDBus::EventGKDBusCallback<StringsArrayToVoid>::exposeSignal(
		this->system_bus_,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"DevicesStarted",
		{}, // FIXME
		std::bind(&DBusHandler::devicesStarted, this, std::placeholders::_1)
	);

	this->pDBus_->NSGKDBus::EventGKDBusCallback<StringsArrayToVoid>::exposeSignal(
		this->system_bus_,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"DevicesStopped",
		{}, // FIXME
		std::bind(&DBusHandler::devicesStopped, this, std::placeholders::_1)
	);

	this->pDBus_->NSGKDBus::EventGKDBusCallback<StringsArrayToVoid>::exposeSignal(
		this->system_bus_,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"DevicesUnplugged",
		{}, // FIXME
		std::bind(&DBusHandler::devicesUnplugged, this, std::placeholders::_1)
	);

	this->pDBus_->NSGKDBus::EventGKDBusCallback<VoidToVoid>::exposeSignal(
		this->system_bus_,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"DaemonIsStopping",
		{}, // FIXME
		std::bind(&DBusHandler::daemonIsStopping, this)
	);

	this->pDBus_->NSGKDBus::EventGKDBusCallback<VoidToVoid>::exposeSignal(
		this->system_bus_,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"DaemonIsStarting",
		{}, // FIXME
		std::bind(&DBusHandler::daemonIsStarting, this)
	);

	this->pDBus_->NSGKDBus::EventGKDBusCallback<VoidToVoid>::exposeSignal(
		this->system_bus_,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"ReportYourself",
		{}, // FIXME
		std::bind(&DBusHandler::reportChangedState, this)
	);

	this->pDBus_->NSGKDBus::EventGKDBusCallback<TwoStringsOneByteToBool>::exposeSignal(
		this->system_bus_,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"MacroRecorded",
		{}, // FIXME
		std::bind(	&DBusHandler::macroRecorded, this,
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3
		)
	);

	this->pDBus_->NSGKDBus::EventGKDBusCallback<TwoStringsOneByteToBool>::exposeSignal(
		this->system_bus_,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"MacroCleared",
		{}, // FIXME
		std::bind(	&DBusHandler::macroCleared, this,
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3
		)
	);

	this->pDBus_->NSGKDBus::EventGKDBusCallback<TwoStringsToVoid>::exposeSignal(
		this->system_bus_,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"deviceMediaEvent",
		{}, // FIXME
		std::bind(	&DBusHandler::deviceMediaEvent, this,
					std::placeholders::_1, std::placeholders::_2
		)
	);
}

void DBusHandler::daemonIsStopping(void) {
	this->buffer_.str("received ");
	this->buffer_ << __func__ << " signal";
	if( _registerStatus ) {
		LOG(INFO)	<< this->buffer_.str()
					<< " - saving state and unregistering with daemon";
		this->devices_.clearDevices();
		this->unregisterWithDaemon();
	}
	else {
#if DEBUGGING_ON
		LOG(DEBUG2) << this->buffer_.str()
					<< " - but we are NOT registered with daemon : "
					<< _currentSession;
#endif
	}
}

void DBusHandler::daemonIsStarting(void) {
	this->buffer_.str("received ");
	this->buffer_ << __func__ << " signal";
	if( _registerStatus ) {
#if DEBUGGING_ON
		LOG(WARNING) << this->buffer_.str()
			<< " - but we are already registered with daemon"
			<< _currentSession;
#endif
	}
	else {
		LOG(INFO) << this->buffer_.str()
				<< " - trying to contact the daemon";
		this->registerWithDaemon();
		_sessionState = this->getCurrentSessionState();
		this->reportChangedState();
		this->devices_.setClientID(_clientID);

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
			this->pDBus_->initializeRemoteMethodCall(
				this->system_bus_,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			this->pDBus_->appendStringToRemoteMethodCall(_clientID);
			this->pDBus_->appendStringToRemoteMethodCall(devID);
			this->pDBus_->sendRemoteMethodCall();

			try {
				this->pDBus_->waitForRemoteMethodCallReply();

				const std::string device_status( this->pDBus_->getNextStringArgument() );
				if(device_status == "started") {
#if DEBUGGING_ON
					LOG(DEBUG3) << "device status from daemon: [" << devID << "] started";
#endif
					this->devices_.startDevice(devID);
				}
				else {
					LOG(WARNING) << "received devicesStarted signal for device " << devID;
					LOG(WARNING) << "but daemon is saying that device status is : " << device_status;
				}
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			this->pDBus_->abandonRemoteMethodCall();
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
			this->pDBus_->initializeRemoteMethodCall(
				this->system_bus_,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			this->pDBus_->appendStringToRemoteMethodCall(_clientID);
			this->pDBus_->appendStringToRemoteMethodCall(devID);
			this->pDBus_->sendRemoteMethodCall();

			try {
				this->pDBus_->waitForRemoteMethodCallReply();

				const std::string device_status( this->pDBus_->getNextStringArgument() );
				if(device_status == "stopped") {
#if DEBUGGING_ON
					LOG(DEBUG3) << "device status from daemon: [" << devID << "] stopped";
#endif
					this->devices_.stopDevice(devID);
				}
				else {
					LOG(WARNING) << "received devicesStopped signal for device " << devID;
					LOG(WARNING) << "but daemon is saying that device status is : " << device_status;
				}
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			this->pDBus_->abandonRemoteMethodCall();
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
			this->pDBus_->initializeRemoteMethodCall(
				this->system_bus_,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				remoteMethod.c_str()
			);
			this->pDBus_->appendStringToRemoteMethodCall(_clientID);
			this->pDBus_->appendStringToRemoteMethodCall(devID);
			this->pDBus_->sendRemoteMethodCall();

			try {
				this->pDBus_->waitForRemoteMethodCallReply();

				const std::string device_status( this->pDBus_->getNextStringArgument() );
				if(device_status == "unplugged") {
#if DEBUGGING_ON
					LOG(DEBUG3) << "device status from daemon: [" << devID << "] unplugged";
#endif
					this->devices_.unplugDevice(devID);
				}
				else {
					LOG(WARNING) << "received devicesUnplugged signal for device " << devID;
					LOG(WARNING) << "but daemon is saying that device status is : " << device_status;
				}
			}
			catch (const GLogiKExcept & e) {
				LogRemoteCallGetReplyFailure
			}
		}
		catch (const GKDBusMessageWrongBuild & e) {
			this->pDBus_->abandonRemoteMethodCall();
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

	return this->devices_.setDeviceMacro(devID, keyName, bankID);
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

	return this->devices_.clearDeviceMacro(devID, keyName, bankID);
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

	this->devices_.runDeviceMediaEvent(devID, mediaKeyEvent);
}

} // namespace GLogiK

