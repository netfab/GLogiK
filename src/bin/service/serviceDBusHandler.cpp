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

#include "serviceDBusHandler.h"

namespace GLogiK
{

using namespace NSGKUtils;

ServiceDBusHandler::ServiceDBusHandler(
	pid_t pid,
	SessionManager& session,
	NSGKUtils::FileSystem* pGKfs
	)
	:	pDBus_(nullptr),
		system_bus_(NSGKDBus::BusConnection::GKDBUS_SYSTEM),
		skip_retry_(false),
		are_we_registered_(false),
		client_id_("undefined"),
		session_framework_(SessionTracker::F_UNKNOWN),
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
			while( ! this->are_we_registered_ ) {
				if( ( session.isSessionAlive() )
						and (retries < UNREACHABLE_DAEMON_MAX_RETRIES)
						and ( ! this->skip_retry_ ) )
				{
					if(retries > 0) {
						LOG(INFO) << "register retry " << retries << " ...";
					}

					try {
						this->registerWithDaemon();
					}
					catch( const GLogiKExcept & e ) {
						if( ! this->skip_retry_ ) {
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

			this->session_state_ = this->getCurrentSessionState();
			this->reportChangedState();

			this->initializeGKDBusSignals();

			/* set GKDBus pointer */
			this->devices_.setDBus(this->pDBus_);
			this->devices_.setClientID(this->client_id_);

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

ServiceDBusHandler::~ServiceDBusHandler() {
	if( this->are_we_registered_ ) {
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
		LOG(DEBUG2) << "client " << this->current_session_ << " already unregistered with deamon";
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

void ServiceDBusHandler::checkDBusMessages(void) {
	this->pDBus_->checkForNextMessage(this->system_bus_);
}

void ServiceDBusHandler::updateSessionState(void) {
	/* if debug output is ON, force-disable it, else debug file
	 * will be spammed by the following DBus request debug output */
	const bool logoff = true;
	const std::string new_state = this->getCurrentSessionState(logoff);
	if(this->session_state_ == new_state) {
#if 0 && DEBUGGING_ON
		LOG(DEBUG5) << "session state did not changed";
#endif
		return;
	}

#if DEBUGGING_ON
	LOG(DEBUG1) << "current session state : " << this->session_state_;
#endif

	std::string unhandled = "unhandled session state : "; unhandled += new_state;

	if( this->session_state_ == "active" ) {
		if(new_state == "online") {
		}
		else if(new_state == "closing") {
		}
		else {
			FATALERROR << unhandled;
			return;
		}
	}
	else if( this->session_state_ == "online" ) {
		if(new_state == "active") {
		}
		else if(new_state == "closing") {
		}
		else {
			FATALERROR << unhandled;
			return;
		}
	}
	else if( this->session_state_ == "closing" ) {
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

	this->session_state_ = new_state;
	this->reportChangedState();
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
void ServiceDBusHandler::registerWithDaemon(void) {
	if( this->are_we_registered_ ) {
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
		this->pDBus_->appendStringToRemoteMethodCall(this->current_session_);
		this->pDBus_->sendRemoteMethodCall();

		/* -- */

		try {
			this->pDBus_->waitForRemoteMethodCallReply();

			const bool ret = this->pDBus_->getNextBooleanArgument();
			if( ret ) {
				this->client_id_ = this->pDBus_->getNextStringArgument();
				this->are_we_registered_ = true;
				LOG(INFO) << "successfully registered with daemon - " << this->client_id_;
			}
			else {
				LOG(ERROR)	<< "failed to register with daemon : false";
				const std::string reason(this->pDBus_->getNextStringArgument());
				if(reason == "already registered")
					this->skip_retry_ = true;
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

void ServiceDBusHandler::unregisterWithDaemon(void) {
	if( ! this->are_we_registered_ ) {
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
		this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
		this->pDBus_->sendRemoteMethodCall();

		try {
			this->pDBus_->waitForRemoteMethodCallReply();

			const bool ret = this->pDBus_->getNextBooleanArgument();
			if( ret ) {
				this->are_we_registered_ = false;
				this->client_id_ = "undefined";
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

void ServiceDBusHandler::setCurrentSessionObjectPath(pid_t pid) {
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
				this->current_session_ = this->pDBus_->getNextStringArgument();
#if DEBUGGING_ON
				LOG(DEBUG1) << "current session : " << this->current_session_;
#endif
				this->session_framework_ = SessionTracker::F_CONSOLEKIT;
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
				this->current_session_ = this->pDBus_->getNextStringArgument();
#if DEBUGGING_ON
				LOG(DEBUG1) << "current session : " << this->current_session_;
#endif
				this->session_framework_ = SessionTracker::F_LOGIND;
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
const std::string ServiceDBusHandler::getCurrentSessionState(const bool logoff) {
	std::string remoteMethod;
	switch(this->session_framework_) {
		/* consolekit */
		case SessionTracker::F_CONSOLEKIT:
			remoteMethod = "GetSessionState";
			try {
				this->pDBus_->initializeRemoteMethodCall(
					this->system_bus_,
					"org.freedesktop.ConsoleKit",
					this->current_session_.c_str(),
					"org.freedesktop.ConsoleKit.Session",
					remoteMethod.c_str(),
					logoff
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
		case SessionTracker::F_LOGIND:
			/* logind */
			remoteMethod = "Get";
			try {
				this->pDBus_->initializeRemoteMethodCall(
					this->system_bus_,
					"org.freedesktop.login1",
					this->current_session_.c_str(),
					"org.freedesktop.DBus.Properties",
					remoteMethod.c_str(),
					logoff
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

void ServiceDBusHandler::reportChangedState(void) {
	if( ! this->are_we_registered_ ) {
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
		this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
		this->pDBus_->appendStringToRemoteMethodCall(this->session_state_);
		this->pDBus_->sendRemoteMethodCall();

		try {
			this->pDBus_->waitForRemoteMethodCallReply();
			const bool ret( this->pDBus_->getNextBooleanArgument() );
			if( ret ) {
#if DEBUGGING_ON
				LOG(DEBUG2) << "successfully reported changed state : " << this->session_state_;
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

void ServiceDBusHandler::initializeDevices(void) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "initializing devices";
#endif

	if( ! this->are_we_registered_ ) {
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
		this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
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
		this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
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
		this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
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

void ServiceDBusHandler::initializeGKDBusSignals(void) {
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
		std::bind(&ServiceDBusHandler::devicesStarted, this, std::placeholders::_1)
	);

	this->pDBus_->NSGKDBus::EventGKDBusCallback<StringsArrayToVoid>::exposeSignal(
		this->system_bus_,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"DevicesStopped",
		{}, // FIXME
		std::bind(&ServiceDBusHandler::devicesStopped, this, std::placeholders::_1)
	);

	this->pDBus_->NSGKDBus::EventGKDBusCallback<StringsArrayToVoid>::exposeSignal(
		this->system_bus_,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"DevicesUnplugged",
		{}, // FIXME
		std::bind(&ServiceDBusHandler::devicesUnplugged, this, std::placeholders::_1)
	);

	this->pDBus_->NSGKDBus::EventGKDBusCallback<VoidToVoid>::exposeSignal(
		this->system_bus_,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"DaemonIsStopping",
		{}, // FIXME
		std::bind(&ServiceDBusHandler::daemonIsStopping, this)
	);

	this->pDBus_->NSGKDBus::EventGKDBusCallback<VoidToVoid>::exposeSignal(
		this->system_bus_,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"DaemonIsStarting",
		{}, // FIXME
		std::bind(&ServiceDBusHandler::daemonIsStarting, this)
	);

	this->pDBus_->NSGKDBus::EventGKDBusCallback<VoidToVoid>::exposeSignal(
		this->system_bus_,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"ReportYourself",
		{}, // FIXME
		std::bind(&ServiceDBusHandler::reportChangedState, this)
	);

	this->pDBus_->NSGKDBus::EventGKDBusCallback<TwoStringsOneByteToBool>::exposeSignal(
		this->system_bus_,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"MacroRecorded",
		{}, // FIXME
		std::bind(	&ServiceDBusHandler::macroRecorded, this,
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3
		)
	);

	this->pDBus_->NSGKDBus::EventGKDBusCallback<TwoStringsOneByteToBool>::exposeSignal(
		this->system_bus_,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
		"MacroCleared",
		{}, // FIXME
		std::bind(	&ServiceDBusHandler::macroCleared, this,
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3
		)
	);
}

void ServiceDBusHandler::daemonIsStopping(void) {
	this->buffer_.str("received ");
	this->buffer_ << __func__ << " signal";
	if( this->are_we_registered_ ) {
		LOG(INFO)	<< this->buffer_.str()
					<< " - saving state and unregistering with daemon";
		this->devices_.clearDevices();
		this->unregisterWithDaemon();
	}
	else {
#if DEBUGGING_ON
		LOG(DEBUG2) << this->buffer_.str()
					<< " - but we are NOT registered with daemon : "
					<< this->current_session_;
#endif
	}
}

void ServiceDBusHandler::daemonIsStarting(void) {
	this->buffer_.str("received ");
	this->buffer_ << __func__ << " signal";
	if( this->are_we_registered_ ) {
#if DEBUGGING_ON
		LOG(WARNING) << this->buffer_.str()
			<< " - but we are already registered with daemon"
			<< this->current_session_;
#endif
	}
	else {
		LOG(INFO) << this->buffer_.str()
				<< " - trying to contact the daemon";
		this->registerWithDaemon();
		this->session_state_ = this->getCurrentSessionState();
		this->reportChangedState();
		this->devices_.setClientID(this->client_id_);

		this->initializeDevices();
	}
}

void ServiceDBusHandler::devicesStarted(const std::vector<std::string> & devicesID) {
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
			this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
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
	if( this->session_state_ == "active" ) {
		this->reportChangedState();
	}
}

void ServiceDBusHandler::devicesStopped(const std::vector<std::string> & devicesID) {
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
			this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
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

void ServiceDBusHandler::devicesUnplugged(const std::vector<std::string> & devicesID) {
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
			this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
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

const bool ServiceDBusHandler::macroRecorded(
	const std::string & devID,
	const std::string & keyName,
	const uint8_t profile)
{
#if DEBUGGING_ON
	LOG(DEBUG3) << "received " << __func__ << " signal"
				<< " - device: " << devID
				<< " profile: " << to_uint(profile)
				<< " key: " << keyName;
#endif

	if( ! this->are_we_registered_ ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "currently not registered, skipping";
#endif
		return false;
	}

	if( this->session_state_ != "active" ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "currently not active, skipping";
#endif
		return false;
	}

	return this->devices_.setDeviceMacro(devID, keyName, profile);
}

const bool ServiceDBusHandler::macroCleared(
	const std::string & devID,
	const std::string & keyName,
	const uint8_t profile)
{
#if DEBUGGING_ON
	LOG(DEBUG3) << "received " << __func__ << " signal"
				<< " - device: " << devID
				<< " profile: " << to_uint(profile)
				<< " key: " << keyName;
#endif

	if( ! this->are_we_registered_ ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "currently not registered, skipping";
#endif
		return false;
	}

	if( this->session_state_ != "active" ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "currently not active, skipping";
#endif
		return false;
	}

	return this->devices_.clearDeviceMacro(devID, keyName, profile);
}

} // namespace GLogiK

