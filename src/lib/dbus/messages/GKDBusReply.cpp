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

#include <new>

#include "lib/utils/utils.h"

#include "GKDBusReply.h"

namespace NSGKDBus
{

using namespace NSGKUtils;

GKDBusReply::GKDBusReply(DBusConnection* connection, DBusMessage* message)
	:	GKDBusMessage(connection)
{
	/* sanity check */
	if(message == nullptr)
		throw GKDBusMessageWrongBuild("DBus message is NULL");

	/* initialize reply from message */
	this->message_ = dbus_message_new_method_return(message);
	if(this->message_ == nullptr)
		throw GKDBusMessageWrongBuild("can't allocate memory for DBus reply message");

	/* initialize potential arguments iterator */
	dbus_message_iter_init_append(this->message_, &this->args_it_);
#if DEBUG_GKDBUS_SUBOBJECTS
	LOG(DEBUG2) << "DBus reply initialized";
#endif
}

GKDBusReply::~GKDBusReply() {
	if(this->hosed_message_) {
		LOG(WARNING) << "DBus hosed reply, giving up";
		dbus_message_unref(this->message_);
		return;
	}

	// TODO dbus_uint32_t serial;
	if( ! dbus_connection_send(this->connection_, this->message_, nullptr) ) {
		dbus_message_unref(this->message_);
		LOG(ERROR) << "DBus reply sending failure";
		return;
	}

	dbus_connection_flush(this->connection_);
	dbus_message_unref(this->message_);
#if DEBUG_GKDBUS_SUBOBJECTS
	LOG(DEBUG2) << "DBus reply sent";
#endif
}

/* --- --- --- */
/* --- --- --- */
/* --- --- --- */

GKDBusMessageReply::GKDBusMessageReply() : reply_(nullptr)
{
}

GKDBusMessageReply::~GKDBusMessageReply()
{
}

void GKDBusMessageReply::initializeReply(DBusConnection* connection, DBusMessage* message) {
	if(this->reply_) /* sanity check */
		throw GKDBusMessageWrongBuild("DBus reply already allocated");

	try {
		this->reply_ = new GKDBusReply(connection, message);
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		LOG(ERROR) << "GKDBus reply allocation failure : " << e.what();
		throw GKDBusMessageWrongBuild("allocation error");
	}
}

void GKDBusMessageReply::appendBooleanToReply(const bool value) {
	if(this->reply_ != nullptr) /* sanity check */
		this->reply_->appendBoolean(value);
}

void GKDBusMessageReply::appendStringToReply(const std::string & value) {
	if(this->reply_ != nullptr) /* sanity check */
		this->reply_->appendString(value);
}

void GKDBusMessageReply::appendStringVectorToReply(const std::vector<std::string> & list) {
	if(this->reply_ != nullptr) /* sanity check */
		this->reply_->appendStringVector(list);
}

void GKDBusMessageReply::appendMacroToReply(const GLogiK::macro_type & macro_array) {
	if(this->reply_ != nullptr) /* sanity check */
		this->reply_->appendMacro(macro_array);
}

void GKDBusMessageReply::appendUInt64ToReply(const uint64_t value) {
	if(this->reply_ != nullptr) /* sanity check */
		this->reply_->appendUInt64(value);
}

void GKDBusMessageReply::appendAsyncArgsToReply(void)
{
	if( this->isAsyncContainerEmpty() )
		return;

	DBusMessage* asyncContainer = this->getAsyncContainerPointer();
	DBusMessageIter arg_it;

#if DEBUG_GKDBUS_SUBOBJECTS
	LOG(DEBUG2) << "init Async parsing";
#endif

	if( ! dbus_message_iter_init(asyncContainer, &arg_it) ) {
		return; /* no arguments */
	}

	try {
		do {
			const int arg_type = GKDBusArgument::decodeNextArgument(&arg_it);
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
						LOG(ERROR) << "invalid argument iterator";
					}
					break;
				default: // other dbus type
					LOG(ERROR) << "unhandled argument type: " << static_cast<char>(arg_type);
					break;
			}
		}
		while( dbus_message_iter_next(&arg_it) );
	}
	catch ( const GLogiKExcept & e ) { /* WrongBuild or EmptyContainer */
		LOG(ERROR) << e.what();
		throw GKDBusMessageWrongBuild("Async args append failure");
	}
}

void GKDBusMessageReply::sendReply(void) {
	this->destroyAsyncContainer();

	if(this->reply_ == nullptr) { /* sanity check */
		LOG(WARNING) << __func__ << " failure because reply not contructed";
		return;
	}

	delete this->reply_;
	this->reply_ = nullptr;
}

void GKDBusMessageReply::abandonReply(void) {
	this->destroyAsyncContainer();

	if(this->reply_) { /* sanity check */
		this->reply_->abandon();
		delete this->reply_;
		this->reply_ = nullptr;
	}
	else {
		LOG(WARNING) << __func__ << " failure because reply not contructed";
	}
}

} // namespace NSGKDBus

