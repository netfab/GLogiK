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

#include "GKDBusMsgReply.h"

#include "include/log.h"

#include "lib/utils/exception.h"

namespace GLogiK
{

GKDBusMsgReply::GKDBusMsgReply(DBusConnection* conn, DBusMessage* message) : connection_(conn), reply_(nullptr) {
	/* sanity checks */
	if(conn == nullptr)
		throw GLogiKExcept("current connection is NULL");
	if(message == nullptr)
		throw GLogiKExcept("DBus message is NULL");

	/* initialize reply from message */
	this->reply_ = dbus_message_new_method_return(message);
	if(this->reply_ == nullptr)
		throw GLogiKExcept("can't allocate memory for DBus reply message");

	/* initialize potential arguments iterator */
	dbus_message_iter_init_append(this->reply_, &this->rep_args_it_);
	LOG(DEBUG2) << "DBus reply initialized";
}

GKDBusMsgReply::~GKDBusMsgReply() {
	// TODO dbus_uint32_t serial;
	if( ! dbus_connection_send(this->connection_, this->reply_, nullptr) ) {
		dbus_message_unref(this->reply_);
		LOG(ERROR) << "DBus reply sending failure";
		return;
	}

	dbus_connection_flush(this->connection_);
	dbus_message_unref(this->reply_);
	LOG(DEBUG2) << "DBus reply sent";
}

void GKDBusMsgReply::appendToReply(const dbus_bool_t value) {
	if( ! dbus_message_iter_append_basic(&this->rep_args_it_, DBUS_TYPE_BOOLEAN, &value) )
		throw GLogiKExcept("DBus reply append boolean value failure, not enough memory");
	LOG(DEBUG2) << "DBus reply boolean value appended";
}

} // namespace GLogiK

