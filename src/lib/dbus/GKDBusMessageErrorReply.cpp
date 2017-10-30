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
#include "GKDBusMessageErrorReply.h"
#undef GKDBUS_COMPILATION

namespace GLogiK
{

GKDBusMessageErrorReply::GKDBusMessageErrorReply(DBusConnection* conn, DBusMessage* message) : GKDBusMessage(conn)
{
	/* sanity check */
	if(message == nullptr)
		throw GLogiKExcept("DBus message is NULL");

	/* initialize reply from message */
	this->message_ = dbus_message_new_error(message, DBUS_ERROR_FAILED, "something was wrong");
	if(this->message_ == nullptr)
		throw GLogiKExcept("can't allocate memory for DBus reply message");

#if DEBUG_GKDBUS_SUBOBJECTS
	LOG(DEBUG2) << "DBus error reply message initialized";
#endif
}

GKDBusMessageErrorReply::~GKDBusMessageErrorReply()
{
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
		LOG(ERROR) << "DBus error reply message sending failure";
#endif
		return;
	}

	dbus_connection_flush(this->connection_);
	dbus_message_unref(this->message_);
#if DEBUG_GKDBUS_SUBOBJECTS
	LOG(DEBUG2) << "DBus error reply message sent";
#endif
}

} // namespace GLogiK

