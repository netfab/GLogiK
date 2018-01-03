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

#include "messages/GKDBusMessage.h"
#include "GKDBus.h"

namespace GLogiK
{

GKDBus::GKDBus(const std::string & rootnode)
	:	GKDBusEvents(rootnode),
		buffer_("", std::ios_base::app),
		session_conn_(nullptr),
		system_conn_(nullptr)
{
#if DEBUGGING_ON
	LOG(DEBUG1) << "dbus object initialization";
#endif
	dbus_error_init(&(this->error_));
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

void GKDBus::checkMessageType(BusConnection current) {
	if(this->message_ == nullptr) {
		LOG(WARNING) << "message is null";
		return;
	}

	try {
		const std::string asked_object_path( to_string( dbus_message_get_path(this->message_) ) );

		DBusConnection* connection = this->getConnection(current);

		for(const auto & object_pair : this->DBusEvents_) {
			/* handle root node introspection special case */
			if( asked_object_path != this->getRootNode() )
				/* object path must match */
				if(asked_object_path != this->getNode(object_pair.first))
					continue;

			for(const auto & interface_pair : object_pair.second) {
				const char* interface = interface_pair.first.c_str();
#if DEBUGGING_ON
				LOG(DEBUG2) << "checking " << interface << " interface";
#endif

				for(const auto & DBusEvent : interface_pair.second) { /* vector of pointers */
					const char* eventName = DBusEvent->eventName.c_str();
#if DEBUGGING_ON
					LOG(DEBUG3) << "checking " << eventName << " event";
#endif
					switch(DBusEvent->eventType) {
						case GKDBusEventType::GKDBUS_EVENT_METHOD: {
							if( dbus_message_is_method_call(this->message_, interface, eventName) ) {
#if DEBUGGING_ON
								LOG(DEBUG1) << "DBus " << eventName << " method called !";
#endif
								DBusEvent->runCallback(connection, this->message_);
								throw GKDBusEventFound();
							}
							break;
						}
						case GKDBusEventType::GKDBUS_EVENT_SIGNAL: {
							if( dbus_message_is_signal(this->message_, interface, eventName) ) {
#if DEBUGGING_ON
								LOG(DEBUG1) << "DBus " << eventName << " signal receipted !";
#endif
								DBusEvent->runCallback(connection, this->message_);
								throw GKDBusEventFound();
							}
							break;
						}
						default:
							throw GLogiKExcept("wrong event type");
							break;
					}
				}
			}
		}
	}
	catch ( const GKDBusEventFound & e ) {
		LOG(DEBUG2) << e.what();
	}
	catch ( const GLogiKExcept & e ) {
		LOG(ERROR) << e.what();
	}

#if DEBUGGING_ON
	LOG(DEBUG3) << "freeing message";
#endif
	dbus_message_unref(this->message_);
	this->message_ = nullptr;
}

void GKDBus::appendExtraStringToMessage(const std::string & value) {
	GKDBusMessage::extra_strings_.push_back(value);
}

/* helper function */
const macro_t GKDBus::getMacro(void) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "rebuilding macro array for DBus values";
#endif
	macro_t macro_array;
	try {
		do {
			KeyEvent e;

			e.event_code = this->getNextByteArgument();

			const uint8_t value = this->getNextByteArgument();
			if( to_uint(value) > to_uint(EventValue::EVENT_KEY_UNKNOWN) )
				throw GLogiKExcept("wrong event value for enum conversion");

			e.event		 = static_cast<EventValue>(value);
			e.interval	 = this->getNextUInt16Argument();

			macro_array.push_back(e);
		}
		while( ! this->byte_arguments_.empty() );
	}
	catch ( const EmptyContainer & e ) {
		LOG(WARNING) << "missing macro argument : " << e.what();
		throw GLogiKExcept("rebuild macro failed");
	}

	if( ! this->uint16_arguments_.empty() ) { /* sanity check */
		LOG(WARNING) << "uint16 container not empty";
	}

#if DEBUGGING_ON
	LOG(DEBUG3) << "events array size : " << macro_array.size();
#endif
	return macro_array;
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

