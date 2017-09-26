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

#include "GKDBusRemoteMethodCall.h"

namespace GLogiK
{

GKDBusRemoteMethodCall::GKDBusRemoteMethodCall(DBusConnection* conn, const char* dest,
	const char* object, const char* interface, const char* method, DBusPendingCall** pending, const bool logoff)
		: connection_(conn), message_(nullptr), pending_(pending), log_off_(logoff)
{
	/* sanity checks */
	if(conn == nullptr)
		throw GLogiKExcept("current connection is NULL");

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
	if( ! dbus_connection_send_with_reply(this->connection_, this->message_, this->pending_, DBUS_TIMEOUT_USE_DEFAULT)) {
		dbus_message_unref(this->message_);
		LOG(ERROR) << "DBus remote method call with pending reply sending failure";
		return;
	}

	dbus_connection_flush(this->connection_);
	dbus_message_unref(this->message_);

#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ )
		LOG(DEBUG2) << "DBus remote method call with pending reply sent";
#endif
}

void GKDBusRemoteMethodCall::appendToRemoteMethodCall(const std::string & value) {
	const char* p = value.c_str();
	if( ! dbus_message_iter_append_basic(&this->args_it_, DBUS_TYPE_STRING, &p) )
		throw GLogiKExcept("DBus remote method call append string value failure, not enough memory");

#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ )
		LOG(DEBUG2) << "DBus remote method call string value appended";
#endif
}

} // namespace GLogiK

