/*
 *
 *	This file is part of GLogiK project.
 *	GLogiKd, daemon to handle special features on gaming keyboards
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


#include <string>

#include "glogik_dbus.h"
#include "include/log.h"

#include "lib/utils/exception.h"

namespace GLogiK
{

DBus::DBus() : message(nullptr), sessionConnection(nullptr), systemConnection(nullptr)
{
	LOG(DEBUG1) << "dbus object initialization";
	dbus_error_init(&(this->error));
}

DBus::~DBus()
{
	LOG(DEBUG1) << "dbus object destruction";
	if(this->sessionConnection) {
		LOG(DEBUG) << "closing DBus Session connection";
		dbus_connection_close(this->sessionConnection);
	}
	if(this->systemConnection) {
		LOG(DEBUG) << "closing DBus System connection";
		dbus_connection_close(this->systemConnection);
	}
}

const bool DBus::checkDBusError(const char* error_message) {
	if( dbus_error_is_set(&this->error) ) {
		LOG(ERROR) << error_message << " : " << this->error.message;
		dbus_error_free(&this->error);
		return true;
	}
	return false;
}

void DBus::connectToSessionBus(const char* connection_name) {
	this->sessionConnection = dbus_bus_get(DBUS_BUS_SESSION, &this->error);
	if( this->checkDBusError("DBus Session connection failure") ) {
		throw GLogiKExcept("DBus Session connection failure");
	}
	LOG(DEBUG1) << "DBus Session connection opened";

	// TODO check name flags
	int ret = dbus_bus_request_name(this->sessionConnection, connection_name,
		DBUS_NAME_FLAG_REPLACE_EXISTING, &this->error);
	this->checkDBusError("DBus Session request name failure");
	if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		throw GLogiKExcept("Dbus Session request name failure");
	}

	LOG(DEBUG1) << "DBus Session requested connection name : " << connection_name;
}

void DBus::addSessionSignalMatch(const char* interface) {
	std::string rule = "type='signal',interface='";
	rule += interface;
	rule += "'";
	dbus_bus_add_match(this->sessionConnection, rule.c_str(), &this->error);
	dbus_connection_flush(this->sessionConnection);
	if( this->checkDBusError("DBus Session match error") ) {
		throw GLogiKExcept("Dbus Session match error");
	}
	LOG(DEBUG1) << "DBus Session match rule sent : " << rule;
}

const bool DBus::checkForNextSessionMessage(void) {
	dbus_connection_read_write(this->sessionConnection, 0);
	this->message = dbus_connection_pop_message(this->sessionConnection);
	return (this->message != nullptr);
}

const bool DBus::checkForSessionSignal(const char* interface, const char* signal_name) {
	if(this->message == nullptr)
		return false;
	LOG(DEBUG2) << "checking for signal";
	if( dbus_message_is_signal(this->message, interface, signal_name) ) {
		LOG(INFO) << "got signal";
		dbus_message_unref(this->message);
		return true;
	}
	dbus_message_unref(this->message);
	return false;

}

} // namespace GLogiK

