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
#include <sstream>

#include "lib/utils/utils.h"

#include "GKDBusRemoteMethodCall.h"

namespace NSGKDBus
{

using namespace NSGKUtils;

GKDBusRemoteMethodCall::GKDBusRemoteMethodCall(
	DBusConnection* connection,
	const char* dest,
	const char* object_path,
	const char* interface,
	const char* method,
	DBusPendingCall** pending,
	const bool logoff
	)	: GKDBusMessage(connection, logoff), pending_(pending)
{
	if( ! dbus_validate_bus_name(dest, nullptr) )
		throw GKDBusMessageWrongBuild("invalid destination name");
	if( ! dbus_validate_path(object_path, nullptr) )
		throw GKDBusMessageWrongBuild("invalid object path");
	if( ! dbus_validate_interface(interface, nullptr) )
		throw GKDBusMessageWrongBuild("invalid interface");
	if( ! dbus_validate_member(method, nullptr) )
		throw GKDBusMessageWrongBuild("invalid method name");

	this->message_ = dbus_message_new_method_call(dest, object_path, interface, method);
	if(this->message_ == nullptr)
		throw GKDBusMessageWrongBuild("can't allocate memory for Remote Object Method Call DBus message");
	
	/* initialize potential arguments iterator */
	dbus_message_iter_init_append(this->message_, &this->args_it_);

#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ ) {
		LOG(DEBUG2) << "Remote Object Method Call DBus message initialized";
		LOG(DEBUG3) << "dest        : " << dest;
		LOG(DEBUG3) << "object_path : " << object_path;
		LOG(DEBUG3) << "interface   : " << interface;
		LOG(DEBUG3) << "method      : " << method;
	}
#endif
}

GKDBusRemoteMethodCall::~GKDBusRemoteMethodCall() {
	if(this->hosed_message_) {
		LOG(WARNING) << "DBus hosed message, giving up";
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

/* --- --- --- */
/* --- --- --- */
/* --- --- --- */

GKDBusMessageRemoteMethodCall::GKDBusMessageRemoteMethodCall()
	:	remote_method_call_(nullptr),
		pending_(nullptr),
		log_off_(false)
{
}

GKDBusMessageRemoteMethodCall::~GKDBusMessageRemoteMethodCall()
{
}

void GKDBusMessageRemoteMethodCall::initializeRemoteMethodCall(
	BusConnection wanted_connection,
	const char* dest,
	const char* object_path,
	const char* interface,
	const char* method,
	const bool logoff
) {
	this->initializeRemoteMethodCall(
		this->getConnection(wanted_connection),
		dest, object_path, interface, method, logoff);
}

void GKDBusMessageRemoteMethodCall::initializeRemoteMethodCall(
	DBusConnection* connection,
	const char* dest,
	const char* object_path,
	const char* interface,
	const char* method,
	const bool logoff
) {
	if(this->remote_method_call_) /* sanity check */
		throw GKDBusMessageWrongBuild("DBus remote_method_call already allocated");

	try {
		this->remote_method_call_ = new GKDBusRemoteMethodCall(
			connection, dest, object_path, interface, method,
			&this->pending_, logoff
		);
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		LOG(ERROR) << "GKDBus remote_method_call allocation failure : " << e.what();
		throw GKDBusMessageWrongBuild("allocation error");
	}

	this->log_off_ = logoff;
}

void GKDBusMessageRemoteMethodCall::appendStringToRemoteMethodCall(const std::string & value) {
	if(this->remote_method_call_ != nullptr) /* sanity check */
		this->remote_method_call_->appendString(value);
}

void GKDBusMessageRemoteMethodCall::appendUInt8ToRemoteMethodCall(const uint8_t value) {
	if(this->remote_method_call_ != nullptr) /* sanity check */
		this->remote_method_call_->appendUInt8(value);
}

void GKDBusMessageRemoteMethodCall::appendUInt32ToRemoteMethodCall(const uint32_t value) {
	if(this->remote_method_call_ != nullptr) /* sanity check */
		this->remote_method_call_->appendUInt32(value);
}

void GKDBusMessageRemoteMethodCall::appendMacrosBankToRemoteMethodCall(const GLogiK::macros_bank_type & macros_bank) {
	if(this->remote_method_call_ != nullptr) /* sanity check */
		this->remote_method_call_->appendMacrosBank(macros_bank);
}

void GKDBusMessageRemoteMethodCall::sendRemoteMethodCall(void)
{
	if(this->remote_method_call_) { /* sanity check */
		delete this->remote_method_call_;
		this->remote_method_call_ = nullptr;
	}
	else {
		LOG(WARNING) << __func__ << " failure because remote_method_call not contructed";
		throw GKDBusMessageWrongBuild("DBus remote_method_call not contructed");
	}
}

void GKDBusMessageRemoteMethodCall::abandonRemoteMethodCall(void)
{
	if(this->remote_method_call_) { /* sanity check */
		this->remote_method_call_->abandon();
		delete this->remote_method_call_;
		this->remote_method_call_ = nullptr;
	}
	else {
		LOG(WARNING) << __func__ << " failure because remote_method_call not contructed";
	}
}

void GKDBusMessageRemoteMethodCall::waitForRemoteMethodCallReply(void) {
	dbus_pending_call_block(this->pending_);

	unsigned int c = 0;

	DBusMessage* message = nullptr;
	// TODO could set a timer between retries ?
	while( message == nullptr and c < 10 ) {
		message = dbus_pending_call_steal_reply(this->pending_);
		c++; /* bonus point */
	}

	dbus_pending_call_unref(this->pending_);

	if(message == nullptr) {
		LOG(WARNING) << __func__ << " message is NULL, retried 10 times";
		throw GKDBusRemoteCallNoReply("can't get pending call reply");
	}

	if(dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_ERROR) {
		std::ostringstream buffer("got DBus error as reply : ", std::ios_base::app);
		GKDBusArgumentString::fillInArguments(message);

		try {
			buffer << GKDBusArgumentString::getNextStringArgument();
		}
		catch ( const EmptyContainer & e ) {
			buffer << "no string argument with DBus error !";
		}

		dbus_message_unref(message);
		throw GKDBusRemoteCallNoReply( buffer.str() );
	}

	GKDBusArgumentString::fillInArguments(message, this->log_off_);
	dbus_message_unref(message);
}

} // namespace NSGKDBus

