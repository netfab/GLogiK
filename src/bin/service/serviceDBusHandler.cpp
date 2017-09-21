/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2017  Fabrice Delliaux <netbox253@gmail.com>
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

#include "lib/utils/utils.h"
#include "include/log.h"

#include "serviceDBusHandler.h"

namespace GLogiK
{

ServiceDBusHandler::ServiceDBusHandler() : warn_count_(0), DBus(nullptr) {
	try {
		this->DBus = new GKDBus();
		this->DBus->connectToSystemBus(GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME);

		this->setCurrentSessionObjectPath();
		this->session_state_ = this->getCurrentSessionState();
	}
	catch ( const GLogiKExcept & e ) {
		if(this->DBus != nullptr)
			delete this->DBus;
		throw;
	}
}

ServiceDBusHandler::~ServiceDBusHandler() {
	if(this->DBus != nullptr)
		delete this->DBus;
}

void ServiceDBusHandler::setCurrentSessionObjectPath(void) {
	// TODO logind support

	/* getting consolekit current session */
	this->DBus->initializeRemoteMethodCall(BusConnection::GKDBUS_SYSTEM, "org.freedesktop.ConsoleKit",
		"/org/freedesktop/ConsoleKit/Manager", "org.freedesktop.ConsoleKit.Manager", "GetCurrentSession");
	this->DBus->sendRemoteMethodCall();

	try {
		this->DBus->waitForRemoteMethodCallReply();
		this->current_session_ = this->DBus->getNextStringArgument();
		LOG(DEBUG2) << "current session : " << this->current_session_;
	}
	catch ( const GLogiKExcept & e ) {
		std::string err("unable to get current session path from session manager");
		LOG(ERROR) << err;
		GK_ERR << err << "\n";
		throw;
	}
}

const std::string ServiceDBusHandler::getCurrentSessionState(void) {
	// TODO logind support

	this->DBus->initializeRemoteMethodCall(BusConnection::GKDBUS_SYSTEM, "org.freedesktop.ConsoleKit",
		this->current_session_.c_str(), "org.freedesktop.ConsoleKit.Session", "GetSessionState");
	this->DBus->sendRemoteMethodCall();

	try {
		this->DBus->waitForRemoteMethodCallReply();
		const std::string ret_string = this->DBus->getNextStringArgument();
#if DEBUGGING_ON
		LOG(DEBUG5) << "current session state : " << ret_string;
#endif
		return ret_string;
	}
	catch ( const GLogiKExcept & e ) {
		if(this->warn_count_ >= MAXIMUM_WARNINGS_BEFORE_FATAL_ERROR)
			throw GLogiKExcept("maximum warning count reached, this is fatal");
		std::string warn("can't get session state from session manager, assuming unchanged : ");
		warn += e.what();
		LOG(WARNING) << warn;
		GK_WARN << warn << "\n";
		this->warn_count_++;
	}

	return this->session_state_;
}

void ServiceDBusHandler::updateSessionState(void) {
	const std::string new_state = this->getCurrentSessionState();
	if(this->session_state_ == new_state) {
#if DEBUGGING_ON
		LOG(DEBUG5) << "session state did not changed";
#endif
		return;
	}

#if DEBUGGING_ON
	LOG(DEBUG3) << "switching session from " << this->session_state_ << " to " << new_state;
#endif

	if( this->session_state_ == "active" ) {
		if(new_state == "online") {
		}
		else if(new_state == "closing") {
		}
	}
	else if( this->session_state_ == "online" ) {
		if(new_state == "active") {
		}
		else if(new_state == "closing") {
		}
	}
	else if( this->session_state_ == "closing" ) {
		if(new_state == "active") {
		}
		else if(new_state == "online") {
		}
	}
	else {
		std::string e = "unhandled session state : ";
		e += this->session_state_;
		throw GLogiKExcept(e);
	}

	this->session_state_ = new_state;
}

void ServiceDBusHandler::checkDBusMessages(void) {
	//if( DBus->checkForNextMessage(BusConnection::GKDBUS_SYSTEM) ) {
	//}
}

} // namespace GLogiK

