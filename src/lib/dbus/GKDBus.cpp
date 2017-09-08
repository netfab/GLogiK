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

#include <algorithm>
#include <string>

#include "GKDBus.h"
#include "include/log.h"

#include "lib/utils/exception.h"

namespace GLogiK
{

GKDBus::GKDBus() : buffer_("", std::ios_base::app), message_(nullptr), reply_(nullptr),
	current_conn_(nullptr), session_conn_(nullptr), system_conn_(nullptr)
{
	LOG(DEBUG1) << "dbus object initialization";
	dbus_error_init(&(this->error_));
}

GKDBus::~GKDBus()
{
	LOG(DEBUG1) << "dbus object destruction";
	if(this->session_conn_) {
		LOG(DEBUG) << "closing DBus Session connection";
		dbus_connection_close(this->session_conn_);
	}
	if(this->system_conn_) {
		LOG(DEBUG) << "closing DBus System connection";
		dbus_connection_close(this->system_conn_);
	}
}

/*
 * DBus connections
 */

void GKDBus::connectToSessionBus(const char* connection_name) {
	this->session_conn_ = dbus_bus_get(DBUS_BUS_SESSION, &this->error_);
	this->checkDBusError("DBus Session connection failure");
	LOG(DEBUG1) << "DBus Session connection opened";

	// TODO check name flags
	int ret = dbus_bus_request_name(this->session_conn_, connection_name,
		DBUS_NAME_FLAG_REPLACE_EXISTING, &this->error_);
	this->checkDBusError("DBus Session request name failure");
	if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		throw GLogiKExcept("DBus Session request name failure : not owner");
	}

	LOG(DEBUG1) << "DBus Session requested connection name : " << connection_name;
}

/* -- */

/*
 * check message methods
 */

const bool GKDBus::checkForNextMessage(BusConnection current) {
	this->setCurrentConnection(current);
	dbus_connection_read_write(this->current_conn_, 0);
	this->message_ = dbus_connection_pop_message(this->current_conn_);
	return (this->message_ != nullptr);
}

const bool GKDBus::checkMessageForSignalOnInterface(const char* interface, const char* signal_name) {
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

const bool GKDBus::checkMessageForMethodCallOnInterface(const char* interface, const char* method) {
	if(this->message_ == nullptr)
		return false;
	const bool ret = dbus_message_is_method_call(this->message_, interface, method);
	if(ret)
		this->fillInArguments();
	return ret;
}

/* -- */

/*
 *	Signals setup on connection
 */

void GKDBus::addSignalMatch(BusConnection current, const char* interface) {
	this->setCurrentConnection(current);
	std::string rule = "type='signal',interface='";
	rule += interface;
	rule += "'";
	dbus_bus_add_match(this->current_conn_, rule.c_str(), &this->error_);
	dbus_connection_flush(this->current_conn_);
	this->checkDBusError("DBus Session match error");
	LOG(DEBUG1) << "DBus Session match rule sent : " << rule;
}

/* -- */

/*
 *	DBus Method call reply
 */

void GKDBus::initializeMethodCallReply(BusConnection current) {
	if(this->reply_) /* sanity check */
		throw GLogiKExcept("DBus reply object already allocated");
	this->setCurrentConnection(current);
	this->reply_ = new GKDBusMsgReply(this->current_conn_, this->message_);
}

void GKDBus::appendToMethodCallReply(const bool value) {
	if(this->reply_ == nullptr) /* sanity check */
		throw GLogiKExcept("DBus reply object not initialized");
	this->reply_->appendToReply(value);
}

void GKDBus::sendMethodCallReply(void) {
	if(this->reply_) { /* sanity check */
		delete this->reply_;
		this->reply_ = nullptr;
	}
	else {
		LOG(WARNING) << __func__ << " failure because reply object not contructed";
	}
}

/* -- */

std::string GKDBus::getNextStringArgument(void) {
	if( this->string_arguments_.empty() )
		throw EmptyContainer("no string argument");
	std::string ret = this->string_arguments_.back();
	this->string_arguments_.pop_back();
	return ret;
}

void GKDBus::checkMethodsCalls(BusConnection current) {
	for(const auto & interface : this->events_string_to_bool_) {
		LOG(DEBUG2) << "checking " << interface.first << " interface";

		for(const auto & DBusEvent : interface.second) {
			const char* method = DBusEvent.method.c_str();
			LOG(DEBUG3) << "checking for " << method << " call";
			if( this->checkMessageForMethodCallOnInterface(interface.first.c_str(), method) ) {
				bool ret = false;
				LOG(DEBUG1) << "DBus " << method << " called !";

				try {
					const std::string devID = this->getNextStringArgument();
					ret = DBusEvent.callback(devID);
				}
				catch ( const EmptyContainer & e ) {
					LOG(DEBUG3) << e.what();
				}

				try {
					this->initializeMethodCallReply(current);
					this->appendToMethodCallReply(ret);
				}
				catch (const std::bad_alloc& e) { /* handle new() failure */
					LOG(ERROR) << "DBus reply allocation failure : " << e.what();
				}
				catch ( const GLogiKExcept & e ) {
					LOG(ERROR) << "DBus reply failure : " << e.what();
				}

				/* delete reply object if allocated */
				this->sendMethodCallReply();
				return; /* one reply at a time */
			}
		}
	}
}

/*
 * private
 */

void GKDBus::checkDBusError(const char* error_message) {
	if( dbus_error_is_set(&this->error_) ) {
		this->buffer_.str(error_message);
		this->buffer_ << " : " << this->error_.message;
		dbus_error_free(&this->error_);
		throw GLogiKExcept(this->buffer_.str());
	}
}

void GKDBus::fillInArguments(void) {
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

void GKDBus::setCurrentConnection(BusConnection current) {
	switch(current) {
		case BusConnection::GKDBUS_SESSION :
			if(this->session_conn_ == nullptr)
				throw GLogiKExcept("DBus Session connection not opened");
			this->current_conn_ = this->session_conn_;
			break;
		case BusConnection::GKDBUS_SYSTEM :
			if(this->system_conn_ == nullptr)
				throw GLogiKExcept("DBus System connection not opened");
			this->current_conn_ = this->system_conn_;
			break;
		default:
			throw GLogiKExcept("asked connection not handled");
			break;
	}
}

/* -- */

} // namespace GLogiK

