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

#include <new>
#include <algorithm>
#include <string>

#include "GKDBus.h"

#include "lib/utils/utils.h"

namespace GLogiK
{

GKDBus::GKDBus(const std::string & rootnode) : buffer_("", std::ios_base::app),
	session_conn_(nullptr),
	system_conn_(nullptr)
{
#if DEBUGGING_ON
	LOG(DEBUG1) << "dbus object initialization";
#endif
	dbus_error_init(&(this->error_));
	this->defineRootNode(rootnode);
}

GKDBus::~GKDBus()
{
#if DEBUGGING_ON
	LOG(DEBUG1) << "dbus object destruction";
#endif
	if(this->session_conn_) {
#if DEBUGGING_ON
		LOG(DEBUG) << "closing DBus Session connection";
#endif
		int ret = dbus_bus_release_name(this->session_conn_, this->session_name_.c_str(), nullptr);
		this->checkReleasedName(ret);
		dbus_connection_unref(this->session_conn_);
	}

	if(this->system_conn_) {
#if DEBUGGING_ON
		LOG(DEBUG) << "closing DBus System connection";
#endif

		int ret = dbus_bus_release_name(this->system_conn_, this->system_name_.c_str(), nullptr);
		this->checkReleasedName(ret);
		dbus_connection_unref(this->system_conn_);
	}
}

void GKDBus::connectToSystemBus(const char* connection_name) {
	this->system_conn_ = dbus_bus_get(DBUS_BUS_SYSTEM, &this->error_);
	this->checkDBusError("DBus System connection failure");
#if DEBUGGING_ON
	LOG(DEBUG1) << "DBus System connection opened";
#endif

	int ret = dbus_bus_request_name(this->system_conn_, connection_name,
		DBUS_NAME_FLAG_REPLACE_EXISTING|DBUS_NAME_FLAG_ALLOW_REPLACEMENT, &this->error_);
	this->checkDBusError("DBus System request name failure");

	if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		throw GLogiKExcept("DBus System request name failure : not owner");
	}

#if DEBUGGING_ON
	LOG(DEBUG1) << "DBus System requested connection name : " << connection_name;
#endif
	this->system_name_ = connection_name;
}

const bool GKDBus::checkForNextMessage(BusConnection current) {
	DBusConnection* connection = this->getConnection(current);
	dbus_connection_read_write(connection, 0);
	this->message_ = dbus_connection_pop_message(connection);
	return (this->message_ != nullptr);
}

/* -- */
/*
 * check if :
 *      one of the registered methods
 *   on one of the registered interfaces
 *   on one of the registered object path
 * was called. if yes, then run the corresponding callback function
 * and send DBus reply after appending the return value
 */

void GKDBus::checkForMethodCall(BusConnection current) {
	if(this->message_ == nullptr) {
		LOG(WARNING) << "message is null";
		throw GLogiKExcept("message is null");
	}

	const char* obj = dbus_message_get_path(this->message_);
	std::string asked_object_path("");
	if(obj != nullptr)
		asked_object_path = obj;

	std::string object_path;

	for(const auto & object_pair : this->DBusEvents_) {
		/* object path must match */
		object_path = this->getNode(object_pair.first);
		if(object_path != asked_object_path)
			continue;

		for(const auto & interface_pair : object_pair.second) {
#if DEBUGGING_ON
			LOG(DEBUG2) << "checking " << interface_pair.first << " interface";
#endif

			for(const auto & DBusEvent : interface_pair.second) { /* vector of pointers */
				/* we want only methods */
				if( DBusEvent->eventType != GKDBusEventType::GKDBUS_EVENT_METHOD )
					continue;

				const char* method = DBusEvent->eventName.c_str();
#if DEBUGGING_ON
				LOG(DEBUG3) << "checking for " << method << " call";
#endif
				if( dbus_message_is_method_call(this->message_, interface_pair.first.c_str(), method) ) {
#if DEBUGGING_ON
					LOG(DEBUG1) << "DBus " << method << " method called !";
#endif
					DBusConnection* connection = this->getConnection(current);
					DBusEvent->runCallback(connection, this->message_);
				}
			}
		}
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

void GKDBus::checkReleasedName(int ret) {
	switch(ret) {
		case DBUS_RELEASE_NAME_REPLY_RELEASED:
			LOG(DEBUG1) << "name released";
			break;
		case DBUS_RELEASE_NAME_REPLY_NOT_OWNER:
			LOG(DEBUG1) << "not owner, cannot release";
			break;
		case DBUS_RELEASE_NAME_REPLY_NON_EXISTENT:
			LOG(DEBUG1) << "nobody owned the name";
			break;
		default:
			LOG(WARNING) << "unknown return value";
			break;
	}
}

void GKDBus::checkDBusError(const char* error_message) {
	if( dbus_error_is_set(&this->error_) ) {
		this->buffer_.str(error_message);
		this->buffer_ << " : " << this->error_.message;
		dbus_error_free(&this->error_);
		throw GLogiKExcept(this->buffer_.str());
	}
}

DBusConnection* GKDBus::getConnection(BusConnection wanted_connection) {
	switch(wanted_connection) {
		case BusConnection::GKDBUS_SESSION :
			if(this->session_conn_ == nullptr)
				throw GLogiKExcept("DBus Session connection not opened");
			return this->session_conn_;
			break;
		case BusConnection::GKDBUS_SYSTEM :
			if(this->system_conn_ == nullptr)
				throw GLogiKExcept("DBus System connection not opened");
			return this->system_conn_;
			break;
		default:
			throw GLogiKExcept("asked connection not handled");
			break;
	}
}

} // namespace GLogiK

