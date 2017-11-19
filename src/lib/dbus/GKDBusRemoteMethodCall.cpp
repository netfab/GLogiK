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

#include <config.h>

#include "lib/utils/utils.h"

#define GKDBUS_COMPILATION 1
#include "GKDBusRemoteMethodCall.h"
#undef GKDBUS_COMPILATION

namespace GLogiK
{

GKDBusRemoteMethodCall::GKDBusRemoteMethodCall(DBusConnection* conn, const char* dest,
	const char* object, const char* interface, const char* method, DBusPendingCall** pending, const bool logoff)
		: GKDBusMessage(conn), pending_(pending), log_off_(logoff)
{
	if( ! dbus_validate_bus_name(dest, nullptr) )
		throw GLogiKExcept("invalid destination name");
	if( ! dbus_validate_path(object, nullptr) )
		throw GLogiKExcept("invalid object path");
	if( ! dbus_validate_interface(interface, nullptr) )
		throw GLogiKExcept("invalid interface");
	if( ! dbus_validate_member(method, nullptr) )
		throw GLogiKExcept("invalid method name");

	this->message_ = dbus_message_new_method_call(dest, object, interface, method);
	if(this->message_ == nullptr)
		throw GLogiKExcept("can't allocate memory for Remote Object Method Call DBus message");
	
	/* initialize potential arguments iterator */
	dbus_message_iter_init_append(this->message_, &this->args_it_);

#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ ) {
		LOG(DEBUG2) << "Remote Object Method Call DBus message initialized";
		LOG(DEBUG3) << "dest      : " << dest;
		LOG(DEBUG3) << "object    : " << object;
		LOG(DEBUG3) << "interface : " << interface;
		LOG(DEBUG3) << "method    : " << method;
	}
#endif
}

GKDBusRemoteMethodCall::~GKDBusRemoteMethodCall() {
	if(this->hosed_message_) {
#if DEBUG_GKDBUS_SUBOBJECTS
		LOG(WARNING) << "DBus hosed message, giving up";
#endif
		dbus_message_unref(this->message_);
		return;
	}

	if( ! dbus_connection_send_with_reply(this->connection_, this->message_, this->pending_, DBUS_TIMEOUT_USE_DEFAULT)) {
		dbus_message_unref(this->message_);
		LOG(ERROR) << "DBus remote method call with pending reply sending failure";
		return;
	}

	dbus_connection_flush(this->connection_);
	dbus_message_unref(this->message_);

#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ ) {
		LOG(DEBUG2) << "DBus remote method call with pending reply sent";
	}
#endif
}

void GKDBusRemoteMethodCall::appendToRemoteMethodCall(const std::string & value) {
	const char* p = value.c_str();
	if( ! dbus_message_iter_append_basic(&this->args_it_, DBUS_TYPE_STRING, &p) ) {
		this->hosed_message_ = true;
		throw GKDBusOOMWrongBuild("RemoteMethodCall string append failure, not enough memory");
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ ) {
		LOG(DEBUG2) << "DBus remote method call string value appended";
	}
#endif
}

/* TODO switch for other int types */
void GKDBusRemoteMethodCall::appendToRemoteMethodCall(const uint32_t value) {
	dbus_uint32_t v = value;
	if( ! dbus_message_iter_append_basic(&this->args_it_, DBUS_TYPE_UINT32, &v) ) {
		this->hosed_message_ = true;
		throw GKDBusOOMWrongBuild("RemoteMethodCall integer append failure, not enough memory");
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ ) {
		LOG(DEBUG2) << "DBus remote method call integer value appended";
	}
#endif
}

} // namespace GLogiK

