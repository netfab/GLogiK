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
#include "GKDBusBroadcastSignal.h"
#undef GKDBUS_COMPILATION

namespace GLogiK
{

GKDBusBroadcastSignal::GKDBusBroadcastSignal(DBusConnection* conn, const char* object,
	const char* interface, const char* signal) : connection_(conn), message_(nullptr)
{
	/* sanity checks */
	if(conn == nullptr)
		throw GLogiKExcept("current connection is NULL");

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

} // namespace GLogiK

