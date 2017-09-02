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

#include <algorithm>
#include <string>

#include "glogik_dbus.h"
#include "include/log.h"

#include "lib/utils/exception.h"

namespace GLogiK
{

DBus::DBus() : buffer_("", std::ios_base::app), message_(nullptr), reply_(nullptr),
	sessionConnection_(nullptr), systemConnection_(nullptr)
{
	LOG(DEBUG1) << "dbus object initialization";
	dbus_error_init(&(this->error_));
}

DBus::~DBus()
{
	LOG(DEBUG1) << "dbus object destruction";
	if(this->sessionConnection_) {
		LOG(DEBUG) << "closing DBus Session connection";
		dbus_connection_close(this->sessionConnection_);
	}
	if(this->systemConnection_) {
		LOG(DEBUG) << "closing DBus System connection";
		dbus_connection_close(this->systemConnection_);
	}
}

void DBus::checkDBusError(const char* error_message) {
	if( dbus_error_is_set(&this->error_) ) {
		this->buffer_.str(error_message);
		this->buffer_ << " : " << this->error_.message;
		dbus_error_free(&this->error_);
		throw GLogiKExcept(this->buffer_.str());
	}
}

void DBus::connectToSessionBus(const char* connection_name) {
	this->sessionConnection_ = dbus_bus_get(DBUS_BUS_SESSION, &this->error_);
	this->checkDBusError("DBus Session connection failure");
	LOG(DEBUG1) << "DBus Session connection opened";

	// TODO check name flags
	int ret = dbus_bus_request_name(this->sessionConnection_, connection_name,
		DBUS_NAME_FLAG_REPLACE_EXISTING, &this->error_);
	this->checkDBusError("DBus Session request name failure");
	if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		throw GLogiKExcept("DBus Session request name failure : not owner");
	}

	LOG(DEBUG1) << "DBus Session requested connection name : " << connection_name;
}

const bool DBus::checkMessageForMethodCallOnInterface(const char* interface, const char* method) {
	if(this->message_ == nullptr)
		return false;
	return dbus_message_is_method_call(this->message_, interface, method);
}

void DBus::initializeReplyToMethodCall(void) {
	if(this->message_ == nullptr) /* sanity check */
		throw GLogiKExcept("DBus message is NULL");

	/* initialize reply from message */
	this->reply_ = dbus_message_new_method_return(this->message_);
	if(this->reply_ == nullptr)
		throw GLogiKExcept("can't allocate memory for DBus reply message");

	/* initialize potential arguments iterator */
	dbus_message_iter_init_append(this->reply_, &this->rep_args_it_);
	LOG(DEBUG2) << "DBus reply initialized";
}

/* must be optionally called right after DBus::initializeReplyToMethodCall() */
void DBus::appendBooleanValueToReply(const bool value) {
	if( ! dbus_message_iter_append_basic(&this->rep_args_it_, DBUS_TYPE_BOOLEAN, &value) )
		throw GLogiKExcept("DBus reply append boolean value failure, not enough memory");
	LOG(DEBUG2) << "DBus reply boolean value appended";
}

void DBus::sendSessionReply(void) {
	// TODO dbus_uint32_t serial;
	if( ! dbus_connection_send(this->sessionConnection_, this->reply_, nullptr) ) {
		dbus_message_unref(this->reply_);
		throw GLogiKExcept("DBus reply sending failure");
	}

	dbus_connection_flush(this->sessionConnection_);
	dbus_message_unref(this->reply_);
	LOG(DEBUG2) << "DBus reply sent";
}

const bool DBus::checkForNextSessionMessage(void) {
	dbus_connection_read_write(this->sessionConnection_, 0);
	this->message_ = dbus_connection_pop_message(this->sessionConnection_);
	return (this->message_ != nullptr);
}

void DBus::addSessionSignalMatch(const char* interface) {
	std::string rule = "type='signal',interface='";
	rule += interface;
	rule += "'";
	dbus_bus_add_match(this->sessionConnection_, rule.c_str(), &this->error_);
	dbus_connection_flush(this->sessionConnection_);
	this->checkDBusError("DBus Session match error");
	LOG(DEBUG1) << "DBus Session match rule sent : " << rule;
}

std::string DBus::getNextStringArgument(void) {
	if( this->string_arguments_.empty() )
		throw EmptyContainer("no string argument");
	std::string ret = this->string_arguments_.back();
	this->string_arguments_.pop_back();
	return ret;
}

void DBus::fillInArguments(void) {
	int current_type = 0;
	LOG(DEBUG2) << "checking signal arguments";
	this->string_arguments_.clear();

	char* signal_value = nullptr;
	DBusMessageIter arg_it;

	dbus_message_iter_init(this->message_, &arg_it);
	while ((current_type = dbus_message_iter_get_arg_type (&arg_it)) != DBUS_TYPE_INVALID) {
		switch(current_type) {
			case DBUS_TYPE_STRING:
				dbus_message_iter_get_basic(&arg_it, &signal_value);
				this->string_arguments_.push_back(signal_value);
				LOG(DEBUG4) << "string arg value : " << signal_value;
				break;
			default: /* other dbus type */
				break;
		}

		dbus_message_iter_next(&arg_it);
	}

	if( ! this->string_arguments_.empty() )
		std::reverse(this->string_arguments_.begin(), this->string_arguments_.end());
}

const bool DBus::checkMessageForSignal(const char* interface, const char* signal_name) {
	if(this->message_ == nullptr)
		return false;
	LOG(DEBUG) << "checking for signal";
	if( dbus_message_is_signal(this->message_, interface, signal_name) ) {
		LOG(DEBUG1) << "got signal " << signal_name;
		this->fillInArguments();
		dbus_message_unref(this->message_);
		return true;
	}
	dbus_message_unref(this->message_);
	return false;

}

} // namespace GLogiK

