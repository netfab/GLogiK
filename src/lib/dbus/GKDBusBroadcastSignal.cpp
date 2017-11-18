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
#include "GKDBusBroadcastSignal.h"
#undef GKDBUS_COMPILATION

namespace GLogiK
{

GKDBusBroadcastSignal::GKDBusBroadcastSignal(DBusConnection* conn, const char* object,
	const char* interface, const char* signal) : GKDBusMessage(conn)
{
	if( ! dbus_validate_path(object, nullptr) )
		throw GLogiKExcept("invalid object path");
	if( ! dbus_validate_interface(interface, nullptr) )
		throw GLogiKExcept("invalid interface");
	if( ! dbus_validate_member(signal, nullptr) )
		throw GLogiKExcept("invalid signal name");

	this->message_ = dbus_message_new_signal(object, interface, signal);
	if(this->message_ == nullptr)
		throw GLogiKExcept("can't allocate memory for Signal DBus message");

	/* initialize potential arguments iterator */
	dbus_message_iter_init_append(this->message_, &this->args_it_);
#if DEBUG_GKDBUS_SUBOBJECTS
	LOG(DEBUG2) << "DBus signal initialized";
#endif
}

GKDBusBroadcastSignal::~GKDBusBroadcastSignal() {
	if(this->hosed_message_) {
#if DEBUG_GKDBUS_SUBOBJECTS
		LOG(WARNING) << "DBus hosed message, giving up";
#endif
		dbus_message_unref(this->message_);
		return;
	}

	// TODO dbus_uint32_t serial;
	if( ! dbus_connection_send(this->connection_, this->message_, nullptr) ) {
		dbus_message_unref(this->message_);
		LOG(ERROR) << "DBus signal sending failure";
		return;
	}

	dbus_connection_flush(this->connection_);
	dbus_message_unref(this->message_);
#if DEBUG_GKDBUS_SUBOBJECTS
	LOG(DEBUG2) << "DBus signal sent";
#endif
}

void GKDBusBroadcastSignal::appendToBroadcastSignal(const std::string & value) {
	const char* p = value.c_str();
	if( ! dbus_message_iter_append_basic(&this->args_it_, DBUS_TYPE_STRING, &p) ) {
		this->hosed_message_ = true;
		throw GKDBusOOMWrongBuild("BroadcastSignal string append failure, not enough memory");
	}
#if DEBUG_GKDBUS_SUBOBJECTS
	LOG(DEBUG2) << "DBus signal string value appended";
#endif
}

} // namespace GLogiK

