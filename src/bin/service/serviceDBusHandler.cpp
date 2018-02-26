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

#include <functional>
#include <stdexcept>
#include <thread>
#include <chrono>

#include "lib/utils/utils.h"
#include "lib/shared/glogik.h"

#include "warningCheck.h"
#include "serviceDBusHandler.h"

namespace GLogiK
{

using namespace NSGKUtils;

ServiceDBusHandler::ServiceDBusHandler(pid_t pid, SessionManager& session)
	:	pDBus_(nullptr),
		system_bus_(NSGKDBus::BusConnection::GKDBUS_SYSTEM),
		register_retry_(true),
		are_we_registered_(false),
		client_id_("undefined"),
		session_framework_(SessionTracker::F_UNKNOWN),
		buffer_("", std::ios_base::app)
{
	try {
		this->pDBus_ = new NSGKDBus::GKDBus(GLOGIK_DESKTOP_SERVICE_DBUS_ROOT_NODE);
		this->pDBus_->connectToSystemBus(GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME);

		this->setCurrentSessionObjectPath(pid);

		unsigned int retries = 0;
		while( ! this->are_we_registered_ ) {
			if( ( session.isSessionAlive() ) and (retries++ < UNREACHABLE_DAEMON_MAX_RETRIES) ) {
				if(retries > 0) {
					LOG(INFO) << "register retry " << retries << " ...";
				}

				try {
					this->registerWithDaemon();
				}
				catch( const GKDBusRemoteCallNoReply & e ) {
#if DEBUGGING_ON
					LOG(DEBUG) << e.what();
#endif
					unsigned int timer = 5;
					LOG(WARNING) << "daemon unreachable, can't register, retrying in " << timer << " seconds ...";
					std::this_thread::sleep_for(std::chrono::seconds(timer));
				}
			}
			else {
				LOG(ERROR) << "daemon unreachable, can't register, giving up";
				throw GLogiKExcept("daemon unreachable");
				break;
			}
		}

		this->session_state_ = this->getCurrentSessionState();
		this->reportChangedState();

		/* want to be warned by the daemon about those signals */

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

ServiceDBusHandler::~ServiceDBusHandler() {
	if( this->are_we_registered_ ) {
		this->devices_.saveDevicesProperties();
		this->unregisterWithDaemon();
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
 * try to tell the daemon we are alive
 * throws on failure
 */
void ServiceDBusHandler::registerWithDaemon(void) {
	if( this->are_we_registered_ ) {
		LOG(WARNING) << "don't need to register, since we are already registered";
		return;
	}

	this->pDBus_->initializeRemoteMethodCall(
		this->system_bus_,
		GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
		GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
		"RegisterClient"
	);
	this->pDBus_->appendStringToRemoteMethodCall(this->current_session_);
	this->pDBus_->sendRemoteMethodCall();

	this->pDBus_->waitForRemoteMethodCallReply();

	const bool ret = this->pDBus_->getNextBooleanArgument();
	if( ret ) {
		this->client_id_ = this->pDBus_->getNextStringArgument();
		this->are_we_registered_ = true;
		LOG(INFO) << "successfully registered with daemon - " << this->client_id_;
	}
	else {
		const char * failure = "failed to register with daemon : false";
		const std::string reason(this->pDBus_->getNextStringArgument());
		if( this->register_retry_ ) {
			/* retrying */
			this->register_retry_ = false;
			LOG(WARNING) << failure << " - " << reason << ", retrying ...";
			std::this_thread::sleep_for(std::chrono::seconds(2));
			this->registerWithDaemon();
		}
		else {
			LOG(ERROR) << failure << " - " << reason;
			throw GLogiKExcept(failure);
		}
	}
}

void ServiceDBusHandler::unregisterWithDaemon(void) {
	if( ! this->are_we_registered_ ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "can't unregister, since we are not currently registered";
#endif
		return;
	}

	try {
		/* telling the daemon we're killing ourself */
		this->pDBus_->initializeRemoteMethodCall(
			this->system_bus_,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
			"UnregisterClient"
		);
		this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
		this->pDBus_->sendRemoteMethodCall();

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
	catch ( const GLogiKExcept & e ) {
		std::string err("failure to unregister with daemon : ");
		err += e.what();
		LOG(ERROR) << err;
	}
}

void ServiceDBusHandler::setCurrentSessionObjectPath(pid_t pid) {
	try {
		/* getting consolekit current session */
		this->pDBus_->initializeRemoteMethodCall(
			this->system_bus_,
			"org.freedesktop.ConsoleKit",
			"/org/freedesktop/ConsoleKit/Manager",
			"org.freedesktop.ConsoleKit.Manager",
			"GetCurrentSession"
		);
		this->pDBus_->sendRemoteMethodCall();

		this->pDBus_->waitForRemoteMethodCallReply();
		this->current_session_ = this->pDBus_->getNextStringArgument();
#if DEBUGGING_ON
		LOG(DEBUG1) << "current session : " << this->current_session_;
#endif
		this->session_framework_ = SessionTracker::F_CONSOLEKIT;
	}
	catch ( const GLogiKExcept & e ) {
		try {
#if DEBUGGING_ON
			LOG(DEBUG) << "error : " << e.what();
#endif
			LOG(INFO) << "consolekit contact failure, trying logind";

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

			this->pDBus_->waitForRemoteMethodCallReply();
			this->current_session_ = this->pDBus_->getNextStringArgument();
#if DEBUGGING_ON
			LOG(DEBUG1) << "current session : " << this->current_session_;
#endif
			this->session_framework_ = SessionTracker::F_LOGIND;
		}
		catch ( const GLogiKExcept & e ) {
			LOG(ERROR) << e.what();
			LOG(ERROR) << "unable to get current session path from session manager";
			throw;
		}
	}
}

/*
 * is the session in active, online or closing state ?
 */
const std::string ServiceDBusHandler::getCurrentSessionState(const bool logoff) {
	std::string ret;
	switch(this->session_framework_) {
		case SessionTracker::F_CONSOLEKIT:
			try {
				this->pDBus_->initializeRemoteMethodCall(
					this->system_bus_,
					"org.freedesktop.ConsoleKit",
					this->current_session_.c_str(),
					"org.freedesktop.ConsoleKit.Session",
					"GetSessionState",
					logoff
				);
				this->pDBus_->sendRemoteMethodCall();

				this->pDBus_->waitForRemoteMethodCallReply();
				ret = this->pDBus_->getNextStringArgument();
#if DEBUGGING_ON
				LOG(DEBUG5) << "current session state : " << ret;
#endif
				return ret;
			}
			catch ( const GLogiKExcept & e ) {
				std::string warn(__func__);
				warn += " failure : ";
				warn += e.what();
				WarningCheck::warnOrThrows(warn);
			}
			break;
		case SessionTracker::F_LOGIND:
				this->pDBus_->initializeRemoteMethodCall(
					this->system_bus_,
					"org.freedesktop.login1",
					this->current_session_.c_str(),
					"org.freedesktop.DBus.Properties",
					"Get",
					logoff
				);
				this->pDBus_->appendStringToRemoteMethodCall("org.freedesktop.login1.Session");
				this->pDBus_->appendStringToRemoteMethodCall("State");
				this->pDBus_->sendRemoteMethodCall();

				this->pDBus_->waitForRemoteMethodCallReply();
				ret = this->pDBus_->getNextStringArgument();
#if DEBUGGING_ON
				LOG(DEBUG5) << "current session state : " << ret;
#endif
				return ret;
			break;
		default:
			throw GLogiKExcept("unhandled session tracker");
			break;
	}

	return this->session_state_;
}

void ServiceDBusHandler::reportChangedState(void) {
	if( ! this->are_we_registered_ ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "currently not registered, skipping report state";
#endif
		return;
	}

	try {
		this->pDBus_->initializeRemoteMethodCall(
			this->system_bus_,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
			"UpdateClientState"
		);
		this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
		this->pDBus_->appendStringToRemoteMethodCall(this->session_state_);
		this->pDBus_->sendRemoteMethodCall();

		this->pDBus_->waitForRemoteMethodCallReply();
		const bool ret = this->pDBus_->getNextBooleanArgument();
		if( ret ) {
#if DEBUGGING_ON
			LOG(DEBUG2) << "successfully reported changed state : " << this->session_state_;
#endif
		}
		else {
			LOG(ERROR) << "failed to report changed state : false";
		}
	}
	catch ( const GLogiKExcept & e ) {
		std::string warn(__func__);
		warn += " failure : ";
		warn += e.what();
		WarningCheck::warnOrThrows(warn);
	}
}

void ServiceDBusHandler::warnUnhandledSessionState(const std::string & state) {
	std::string warn = "unhandled session state : ";
	warn += state;
	WarningCheck::warnOrThrows(warn);
}

void ServiceDBusHandler::updateSessionState(void) {
	/* if debug output is ON, force-disable it, else debug file
	 * will be spammed by the following DBus request debug output */
	const bool logoff = true;
	const std::string new_state = this->getCurrentSessionState(logoff);
	if(this->session_state_ == new_state) {
#if DEBUGGING_ON
		LOG(DEBUG5) << "session state did not changed";
#endif
		return;
	}

#if DEBUGGING_ON
	LOG(DEBUG1) << "current session state : " << this->session_state_;
#endif

	if( this->session_state_ == "active" ) {
		if(new_state == "online") {
		}
		else if(new_state == "closing") {
		}
		else {
			this->warnUnhandledSessionState(new_state);
			return;
		}
	}
	else if( this->session_state_ == "online" ) {
		if(new_state == "active") {
		}
		else if(new_state == "closing") {
		}
		else {
			this->warnUnhandledSessionState(new_state);
			return;
		}
	}
	else if( this->session_state_ == "closing" ) {
		if(new_state == "active") {
		}
		else if(new_state == "online") {
		}
		else {
			this->warnUnhandledSessionState(new_state);
			return;
		}
	}
	else {
		this->warnUnhandledSessionState(this->session_state_);
		return;
	}

#if DEBUGGING_ON
	LOG(DEBUG1) << "switching session state to : " << new_state;
#endif

	this->session_state_ = new_state;
	this->reportChangedState();
}


void ServiceDBusHandler::daemonIsStopping(void) {
	if( this->are_we_registered_ ) {
		LOG(INFO) << "received daemonIsStopping signal - saving state and unregistering with daemon";
		this->devices_.saveDevicesProperties();
		this->unregisterWithDaemon();
	}
	else {
#if DEBUGGING_ON
		LOG(DEBUG2) << "client " << this->current_session_ << " already unregistered with deamon";
#endif
	}

	this->devices_.clearLoadedDevices();
	// TODO sleep and retry to register
}

void ServiceDBusHandler::devicesStarted(const std::vector<std::string> & devicesIDArray) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "it seems that some devices were started";
#endif
	for(const auto& devID : devicesIDArray) {
		try {
			this->pDBus_->initializeRemoteMethodCall(
				this->system_bus_,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				"GetDeviceStatus"
			);
			this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
			this->pDBus_->appendStringToRemoteMethodCall(devID);
			this->pDBus_->sendRemoteMethodCall();

			this->pDBus_->waitForRemoteMethodCallReply();

			const std::string device_status( this->pDBus_->getNextStringArgument() );
			if(device_status == "started") {
#if DEBUGGING_ON
				LOG(DEBUG3) << "daemon said that device " << devID << " is really started, what about us ?";
#endif
				this->devices_.startDevice(devID, this->session_state_);
			}
			else {
				LOG(WARNING) << "received devicesStarted signal for device " << devID;
				LOG(WARNING) << "but daemon is saying that device status is : " << device_status;
			}
		}
		catch (const GLogiKExcept & e) {
			std::string warn(__func__);
			warn += " failure : ";
			warn += e.what();
			WarningCheck::warnOrThrows(warn);
		}
	}
}

void ServiceDBusHandler::devicesStopped(const std::vector<std::string> & devicesIDArray) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "it seems that some devices were stopped";
#endif
	for(const auto& devID : devicesIDArray) {
		try {
			this->pDBus_->initializeRemoteMethodCall(
				this->system_bus_,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				"GetDeviceStatus"
			);
			this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
			this->pDBus_->appendStringToRemoteMethodCall(devID);
			this->pDBus_->sendRemoteMethodCall();

			this->pDBus_->waitForRemoteMethodCallReply();

			const std::string device_status( this->pDBus_->getNextStringArgument() );
			if(device_status == "stopped") {
#if DEBUGGING_ON
				LOG(DEBUG3) << "daemon said that device " << devID << " is really stopped, what about us ?";
#endif
				this->devices_.stopDevice(devID);
			}
			else {
				LOG(WARNING) << "received devicesStopped signal for device " << devID;
				LOG(WARNING) << "but daemon is saying that device status is : " << device_status;
			}
		}
		catch (const GLogiKExcept & e) {
			std::string warn(__func__);
			warn += " failure : ";
			warn += e.what();
			WarningCheck::warnOrThrows(warn);
		}
	}
}

void ServiceDBusHandler::devicesUnplugged(const std::vector<std::string> & devicesIDArray) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "it seems that some devices were unplugged";
#endif
	for(const auto& devID : devicesIDArray) {
#if DEBUGGING_ON
		LOG(DEBUG3) << "device : " << devID;
#endif
		try {
			this->pDBus_->initializeRemoteMethodCall(
				this->system_bus_,
				GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				"GetDeviceStatus"
			);
			this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
			this->pDBus_->appendStringToRemoteMethodCall(devID);
			this->pDBus_->sendRemoteMethodCall();

			this->pDBus_->waitForRemoteMethodCallReply();

			const std::string device_status( this->pDBus_->getNextStringArgument() );
			if(device_status == "unplugged") {
#if DEBUGGING_ON
				LOG(DEBUG3) << "daemon said that device " << devID << " is really unplugged, what about us ?";
#endif
				this->devices_.unplugDevice(devID);
			}
			else {
				LOG(WARNING) << "received devicesUnplugged signal for device " << devID;
				LOG(WARNING) << "but daemon is saying that device status is : " << device_status;
			}
		}
		catch (const GLogiKExcept & e) {
			std::string warn(__func__);
			warn += " failure : ";
			warn += e.what();
			WarningCheck::warnOrThrows(warn);
		}
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

	/* started devices */
	try {
		this->pDBus_->initializeRemoteMethodCall(
			this->system_bus_,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
			"GetStartedDevices"
		);
		this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
		this->pDBus_->sendRemoteMethodCall();

		this->pDBus_->waitForRemoteMethodCallReply();

		devicesID = this->pDBus_->getStringsArray();
#if DEBUGGING_ON
		LOG(DEBUG3) << "daemon says " << devicesID.size() << " devices started";
#endif
		this->devicesStarted(devicesID);
	}
	catch (const GLogiKExcept & e) {
		std::string warn(__func__);
		warn += " failure : ";
		warn += e.what();
		WarningCheck::warnOrThrows(warn);
	}

	devicesID.clear();

	/* saying the daemon that we are ready */
	try {
		this->pDBus_->initializeRemoteMethodCall(
			this->system_bus_,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
			"ToggleClientReadyPropertie"
		);
		this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
		this->pDBus_->sendRemoteMethodCall();

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
		std::string warn(__func__);
		warn += " failure : ";
		warn += e.what();
		WarningCheck::warnOrThrows(warn);
	}

	/* stopped devices */
	try {
		this->pDBus_->initializeRemoteMethodCall(
			this->system_bus_,
			GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
			"GetStoppedDevices"
		);
		this->pDBus_->appendStringToRemoteMethodCall(this->client_id_);
		this->pDBus_->sendRemoteMethodCall();

		this->pDBus_->waitForRemoteMethodCallReply();

		devicesID = this->pDBus_->getStringsArray();
#if DEBUGGING_ON
		LOG(DEBUG3) << "daemon says " << devicesID.size() << " devices stopped";
#endif

		this->devicesStopped(devicesID);
	}
	catch (const GLogiKExcept & e) {
		std::string warn(__func__);
		warn += " failure : ";
		warn += e.what();
		WarningCheck::warnOrThrows(warn);
	}
}

const bool ServiceDBusHandler::macroRecorded(
	const std::string & devID,
	const std::string & keyName,
	const uint8_t profile)
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "got MacroRecorded signal for device " << devID
				<< " profile " << to_uint(profile) << " key " << keyName;
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

void ServiceDBusHandler::checkDBusMessages(void) {
	this->pDBus_->checkForNextMessage(this->system_bus_);
}

} // namespace GLogiK

