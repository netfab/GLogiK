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

#define GKDBUS_COMPILATION 1
#include "GKDBusMessageReply.h"
#undef GKDBUS_COMPILATION

namespace GLogiK
{

GKDBusMessageReply::GKDBusMessageReply(DBusConnection* conn, DBusMessage* message) : GKDBusMessage(conn)
{
	/* sanity check */
	if(message == nullptr)
		throw GLogiKExcept("DBus message is NULL");

	/* initialize reply from message */
	this->message_ = dbus_message_new_method_return(message);
	if(this->message_ == nullptr)
		throw GLogiKExcept("can't allocate memory for DBus reply message");

	/* initialize potential arguments iterator */
	dbus_message_iter_init_append(this->message_, &this->args_it_);
#if DEBUG_GKDBUS_SUBOBJECTS
	LOG(DEBUG2) << "DBus reply initialized";
#endif
}

GKDBusMessageReply::~GKDBusMessageReply() {
	if(this->hosed_message_) {
#if DEBUG_GKDBUS_SUBOBJECTS
		LOG(WARNING) << "DBus hosed reply, giving up";
#endif
		dbus_message_unref(this->message_);
		return;
	}

	// TODO dbus_uint32_t serial;
	if( ! dbus_connection_send(this->connection_, this->message_, nullptr) ) {
		dbus_message_unref(this->message_);
#if DEBUG_GKDBUS_SUBOBJECTS
		LOG(ERROR) << "DBus reply sending failure";
#endif
		return;
	}

	dbus_connection_flush(this->connection_);
	dbus_message_unref(this->message_);
#if DEBUG_GKDBUS_SUBOBJECTS
	LOG(DEBUG2) << "DBus reply sent";
#endif
}

void GKDBusMessageReply::appendToMessageReply(const dbus_bool_t value) {
	if( ! dbus_message_iter_append_basic(&this->args_it_, DBUS_TYPE_BOOLEAN, &value) ) {
		this->hosed_message_ = true;
#if DEBUG_GKDBUS_SUBOBJECTS
			LOG(ERROR) << "boolean append_basic failure, not enough memory";
#endif
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}
#if DEBUG_GKDBUS_SUBOBJECTS
	LOG(DEBUG2) << "DBus reply boolean value appended";
#endif
}

void GKDBusMessageReply::appendToMessageReply(const std::string & value) {
	const char* p = value.c_str();
	if( ! dbus_message_iter_append_basic(&this->args_it_, DBUS_TYPE_STRING, &p) ) {
		this->hosed_message_ = true;
#if DEBUG_GKDBUS_SUBOBJECTS
			LOG(ERROR) << "string append_basic failure, not enough memory";
#endif
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}
#if DEBUG_GKDBUS_SUBOBJECTS
	LOG(DEBUG2) << "DBus reply string value appended";
#endif
}

void GKDBusMessageReply::appendToMessageReply(const std::vector<std::string> & list) {
	DBusMessageIter container_it;

	if( ! dbus_message_iter_open_container(&this->args_it_, DBUS_TYPE_ARRAY, "s", &container_it) ) {
		this->hosed_message_ = true;
#if DEBUG_GKDBUS_SUBOBJECTS
		LOG(ERROR) << "open_container failure, not enough memory";
#endif
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}

	for (const std::string & s : list) {
		const char* p = s.c_str();
		if( ! dbus_message_iter_append_basic(&container_it, DBUS_TYPE_STRING, &p) ) {
#if DEBUG_GKDBUS_SUBOBJECTS
			LOG(ERROR) << "array string append_basic failure, not enough memory";
#endif
			this->hosed_message_ = true;
			dbus_message_iter_abandon_container(&this->args_it_, &container_it);
			throw GKDBusOOMWrongBuild(this->append_failure_);
			break;
		}
	}

	if( ! dbus_message_iter_close_container(&this->args_it_, &container_it) ) {
#if DEBUG_GKDBUS_SUBOBJECTS
		LOG(ERROR) << "close_container failure, not enough memory";
#endif
		this->hosed_message_ = true;
		dbus_message_iter_abandon_container(&this->args_it_, &container_it);
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	LOG(DEBUG2) << "DBus reply strings array appended";
#endif
}

} // namespace GLogiK

