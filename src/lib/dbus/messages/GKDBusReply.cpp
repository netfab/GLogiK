/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
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

#include "GKDBusReply.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

GKDBusReply::GKDBusReply(DBusConnection* const connection, DBusMessage* message)
	:	GKDBusMessage(connection)
{
	GK_LOG_FUNC

	/* sanity check */
	if(message == nullptr)
		throw GKDBusMessageWrongBuild("DBus message is NULL");

	/* initialize reply from message */
	_message = dbus_message_new_method_return(message);
	if(_message == nullptr)
		throw GKDBusMessageWrongBuild("can't allocate memory for DBus reply message");

	/* initialize potential arguments iterator */
	dbus_message_iter_init_append(_message, &_itMessage);

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "DBus reply initialized")
#endif
}

GKDBusReply::~GKDBusReply()
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
		LOG(error) << "DBus reply sending failure";
		return;
	}

	dbus_connection_flush(_connection);
	dbus_message_unref(_message);

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "DBus reply sent")
#endif
}

/* --- --- --- */
/* --- --- --- */
/* --- --- --- */

GKDBusMessageReply::GKDBusMessageReply() : _reply(nullptr)
{
}

GKDBusMessageReply::~GKDBusMessageReply()
{
}

void GKDBusMessageReply::initializeReply(DBusConnection* const connection, DBusMessage* message)
{
	GK_LOG_FUNC

	if(_reply) /* sanity check */
		throw GKDBusMessageWrongBuild("DBus reply already allocated");

	try {
		_reply = new GKDBusReply(connection, message);
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		LOG(error) << "GKDBus reply allocation failure : " << e.what();
		throw GKDBusMessageWrongBuild("allocation error");
	}
}

void GKDBusMessageReply::appendBooleanToReply(const bool value)
{
	if(_reply != nullptr) /* sanity check */
		_reply->appendBoolean(value);
}

void GKDBusMessageReply::appendStringToReply(const std::string & value)
{
	if(_reply != nullptr) /* sanity check */
		_reply->appendString(value);
}

void GKDBusMessageReply::appendStringVectorToReply(const std::vector<std::string> & list)
{
	if(_reply != nullptr) /* sanity check */
		_reply->appendStringVector(list);
}

void GKDBusMessageReply::appendGKeysIDArrayToReply(const GLogiK::GKeysIDArray_type & array)
{
	if(_reply != nullptr) /* sanity check */
		_reply->appendGKeysIDArray(array);
}

void GKDBusMessageReply::appendMacroToReply(const GLogiK::macro_type & macro)
{
	if(_reply != nullptr) /* sanity check */
		_reply->appendMacro(macro);
}

void GKDBusMessageReply::appendLCDPluginsPropertiesArrayToReply(
	const GLogiK::LCDPluginsPropertiesArray_type & array)
{
	if(_reply != nullptr) /* sanity check */
		_reply->appendLCDPluginsPropertiesArray(array);
}

void GKDBusMessageReply::appendUInt64ToReply(const uint64_t value)
{
	if(_reply != nullptr) /* sanity check */
		_reply->appendUInt64(value);
}

void GKDBusMessageReply::appendAsyncArgsToReply(void)
{
	GK_LOG_FUNC

	if( this->isAsyncContainerEmpty() )
		return;

	DBusMessage* asyncContainer = this->getAsyncContainerPointer();
	DBusMessageIter itArgument;

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "init Async parsing")
#endif

	if( ! dbus_message_iter_init(asyncContainer, &itArgument) ) {
		return; /* no arguments */
	}

	try {
		do {
			const int arg_type = GKDBusArgument::decodeNextArgument(&itArgument);
			switch(arg_type) {
				case DBUS_TYPE_STRING:
				//case DBUS_TYPE_OBJECT_PATH:
					this->appendStringToReply( GKDBusArgumentString::getNextStringArgument() );
					break;
				case DBUS_TYPE_UINT64:
					this->appendUInt64ToReply( GKDBusArgumentUInt64::getNextUInt64Argument() );
					break;
				case DBUS_TYPE_INVALID:
					{
						LOG(error) << "invalid argument iterator";
					}
					break;
				default: // other dbus type
					LOG(error) << "unhandled argument type: " << static_cast<char>(arg_type);
					break;
			}
		}
		while( dbus_message_iter_next(&itArgument) );
	}
	catch ( const GLogiKExcept & e ) { /* WrongBuild or EmptyContainer */
		LOG(error) << e.what();
		throw GKDBusMessageWrongBuild("Async args append failure");
	}
}

void GKDBusMessageReply::sendReply(void)
{
	GK_LOG_FUNC

	this->destroyAsyncContainer();

	if(_reply == nullptr) { /* sanity check */
		LOG(warning) << "tried to send NULL reply";
		return;
	}

	delete _reply;
	_reply = nullptr;
}

void GKDBusMessageReply::abandonReply(void)
{
	GK_LOG_FUNC

	this->destroyAsyncContainer();

	if(_reply) { /* sanity check */
		_reply->abandon();
		delete _reply;
		_reply = nullptr;
	}
	else {
		LOG(warning) << "tried to abandon NULL reply";
	}
}

} // namespace NSGKDBus

