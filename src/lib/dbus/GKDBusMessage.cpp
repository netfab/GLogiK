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

#include "GKDBusMessage.h"

namespace GLogiK
{

GKDBusMessage::GKDBusMessage(DBusConnection* conn, const bool logoff)
	: connection_(conn), message_(nullptr), hosed_message_(false), log_off_(logoff)
{
	/* sanity check */
	if(conn == nullptr)
		throw GLogiKExcept("current connection is NULL");
}

GKDBusMessage::~GKDBusMessage() {
}

void GKDBusMessage::appendToMessage(const bool value) {
	dbus_bool_t v = value;
	if( ! dbus_message_iter_append_basic(&this->args_it_, DBUS_TYPE_BOOLEAN, &v) ) {
		this->hosed_message_ = true;
#if DEBUG_GKDBUS_SUBOBJECTS
			LOG(ERROR) << "boolean append_basic failure, not enough memory";
#endif
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}
#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ ) {
		LOG(DEBUG2) << "DBus boolean value appended";
	}
#endif
}

void GKDBusMessage::appendToMessage(const std::string & value) {
	const char* p = value.c_str();
	if( ! dbus_message_iter_append_basic(&this->args_it_, DBUS_TYPE_STRING, &p) ) {
		this->hosed_message_ = true;
#if DEBUG_GKDBUS_SUBOBJECTS
			LOG(ERROR) << "string append_basic failure, not enough memory";
#endif
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}
#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ ) {
		LOG(DEBUG2) << "DBus string value appended";
	}
#endif
}

void GKDBusMessage::appendToMessage(const std::vector<std::string> & list) {
	DBusMessageIter container_it;

	if( ! dbus_message_iter_open_container(&this->args_it_, DBUS_TYPE_ARRAY, "s", &container_it) ) {
		this->hosed_message_ = true;
#if DEBUG_GKDBUS_SUBOBJECTS
		LOG(ERROR) << "string array open_container failure, not enough memory";
#endif
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}

	for (const std::string & s : list) {
		const char* p = s.c_str();
		if( ! dbus_message_iter_append_basic(&container_it, DBUS_TYPE_STRING, &p) ) {
#if DEBUG_GKDBUS_SUBOBJECTS
			LOG(ERROR) << "string array append_basic failure, not enough memory";
#endif
			this->hosed_message_ = true;
			dbus_message_iter_abandon_container(&this->args_it_, &container_it);
			throw GKDBusOOMWrongBuild(this->append_failure_);
		}
	}

	if( ! dbus_message_iter_close_container(&this->args_it_, &container_it) ) {
#if DEBUG_GKDBUS_SUBOBJECTS
		LOG(ERROR) << "string array close_container failure, not enough memory";
#endif
		this->hosed_message_ = true;
		dbus_message_iter_abandon_container(&this->args_it_, &container_it);
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ ) {
		LOG(DEBUG2) << "DBus string array appended";
	}
#endif
}

/* TODO switch for other int types */
void GKDBusMessage::appendToMessage(const uint32_t value) {
	dbus_uint32_t v = value;
	if( ! dbus_message_iter_append_basic(&this->args_it_, DBUS_TYPE_UINT32, &v) ) {
		this->hosed_message_ = true;
		throw GKDBusOOMWrongBuild("RemoteMethodCall integer append failure, not enough memory");
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ ) {
		LOG(DEBUG2) << "DBus int32 value appended";
	}
#endif
}

void GKDBusMessage::appendVariantToMessage(const std::string & value) {
	DBusMessageIter container_it;

	if( ! dbus_message_iter_open_container(&this->args_it_, DBUS_TYPE_VARIANT, "s", &container_it) ) {
		this->hosed_message_ = true;
#if DEBUG_GKDBUS_SUBOBJECTS
		LOG(ERROR) << "string variant open_container failure, not enough memory";
#endif
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}

	const char* p = value.c_str();

	if( ! dbus_message_iter_append_basic(&container_it, DBUS_TYPE_STRING, &p) ) {
#if DEBUG_GKDBUS_SUBOBJECTS
		LOG(ERROR) << "string variant append_basic failure, not enough memory";
#endif
		this->hosed_message_ = true;
		dbus_message_iter_abandon_container(&this->args_it_, &container_it);
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}

	if( ! dbus_message_iter_close_container(&this->args_it_, &container_it) ) {
#if DEBUG_GKDBUS_SUBOBJECTS
		LOG(ERROR) << "string variant close_container failure, not enough memory";
#endif
		this->hosed_message_ = true;
		dbus_message_iter_abandon_container(&this->args_it_, &container_it);
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ ) {
		LOG(DEBUG2) << "DBus string variant appended";
	}
#endif
}

} // namespace GLogiK

