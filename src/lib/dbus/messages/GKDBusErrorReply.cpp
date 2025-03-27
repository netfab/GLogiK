/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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

#include "lib/utils/utils.hpp"

#include "GKDBusErrorReply.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

GKDBusErrorReply::GKDBusErrorReply(
	DBusConnection* const connection,
	DBusMessage* message,
	const char* errorMessage)
	:	GKDBusMessage(connection)
{
	GK_LOG_FUNC

	/* sanity check */
	if(message == nullptr)
		throw GKDBusMessageWrongBuild("DBus message is NULL");

	/* initialize reply from message */
	_message = dbus_message_new_error(message, DBUS_ERROR_FAILED, errorMessage);
	if(_message == nullptr)
		throw GKDBusMessageWrongBuild("can't allocate memory for DBus reply message");

#if DEBUG_GKDBUS
	GKLog(trace, "DBus error reply message initialized")
#endif
}

GKDBusErrorReply::~GKDBusErrorReply()
{
	GK_LOG_FUNC

	if(_hosedMessage) {
		LOG(warning) << "DBus hosed reply, giving up";
		dbus_message_unref(_message);
		return;
	}

	// TODO dbus_uint32_t serial;
	if( ! dbus_connection_send(_connection, _message, nullptr) ) {
		dbus_message_unref(_message);
		LOG(error) << "DBus error reply message sending failure";
		return;
	}

	dbus_connection_flush(_connection);
	dbus_message_unref(_message);
#if DEBUG_GKDBUS
	GKLog(trace, "DBus error reply message sent")
#endif
}

/* --- --- --- */
/* --- --- --- */
/* --- --- --- */

GKDBusMessageErrorReply::GKDBusMessageErrorReply() : _errorReply(nullptr)
{
}

GKDBusMessageErrorReply::~GKDBusMessageErrorReply()
{
}

void GKDBusMessageErrorReply::initializeErrorReply(
	DBusConnection* const connection,
	DBusMessage* message,
	const char* errorMessage)
{
	GK_LOG_FUNC

	if(_errorReply) /* sanity check */
		throw GKDBusMessageWrongBuild("DBus reply already allocated");

	try {
		_errorReply = new GKDBusErrorReply(connection, message, errorMessage);
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		LOG(error) << "GKDBus reply allocation failure : " << e.what();
		throw GKDBusMessageWrongBuild("allocation error");
	}
}

void GKDBusMessageErrorReply::sendErrorReply(void)
{
	GK_LOG_FUNC

	if(_errorReply) { /* sanity check */
		delete _errorReply;
		_errorReply = nullptr;
	}
	else {
		LOG(warning) << "tried to send NULL error reply";
	}
}

void GKDBusMessageErrorReply::buildAndSendErrorReply(
	DBusConnection* const connection,
	DBusMessage* message,
	const char* errorMessage)
{
	GK_LOG_FUNC

	try {
		this->initializeErrorReply(connection, message, errorMessage);
	}
	catch (const GLogiKExcept & e) {
		LOG(error) << "DBus error reply failure : " << e.what();
	}

	/* delete error reply if allocated */
	this->sendErrorReply();
}

} // namespace NSGKDBus

