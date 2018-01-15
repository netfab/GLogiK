/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2018  Fabrice Delliaux <netbox253@gmail.com>
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

#include "GKDBusErrorReply.h"

namespace NSGKDBus
{

using namespace NSGKUtils;

GKDBusErrorReply::GKDBusErrorReply(DBusConnection* connection, DBusMessage* message) : GKDBusMessage(connection)
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

GKDBusErrorReply::~GKDBusErrorReply()
{
	if(this->hosed_message_) {
		LOG(WARNING) << "DBus hosed reply, giving up";
		dbus_message_unref(this->message_);
		return;
	}

	// TODO dbus_uint32_t serial;
	if( ! dbus_connection_send(this->connection_, this->message_, nullptr) ) {
		dbus_message_unref(this->message_);
		LOG(ERROR) << "DBus error reply message sending failure";
		return;
	}

	dbus_connection_flush(this->connection_);
	dbus_message_unref(this->message_);
#if DEBUG_GKDBUS_SUBOBJECTS
	LOG(DEBUG2) << "DBus error reply message sent";
#endif
}

/* --- --- --- */
/* --- --- --- */
/* --- --- --- */

GKDBusMessageErrorReply::GKDBusMessageErrorReply() : error_reply_(nullptr)
{
}

GKDBusMessageErrorReply::~GKDBusMessageErrorReply()
{
}

void GKDBusMessageErrorReply::initializeErrorReply(DBusConnection* connection, DBusMessage* message) {
	if(this->error_reply_) /* sanity check */
		throw GLogiKExcept("DBus reply already allocated");

	try {
		this->error_reply_ = new GKDBusErrorReply(connection, message);
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		LOG(ERROR) << "GKDBus reply allocation failure : " << e.what();
		throw GLogiKBadAlloc();
	}
}

void GKDBusMessageErrorReply::sendErrorReply(void) {
	if(this->error_reply_) { /* sanity check */
		delete this->error_reply_;
		this->error_reply_ = nullptr;
	}
	else {
		LOG(WARNING) << __func__ << " failure because reply not contructed";
	}
}

void GKDBusMessageErrorReply::buildAndSendErrorReply(DBusConnection* connection, DBusMessage* message) {
	try {
		this->initializeErrorReply(connection, message);
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << "DBus error reply failure : " << e.what();
	}

	/* delete error reply if allocated */
	this->sendErrorReply();
}

} // namespace NSGKDBus

