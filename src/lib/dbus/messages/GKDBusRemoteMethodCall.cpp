/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2022  Fabrice Delliaux <netbox253@gmail.com>
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

#include "lib/utils/utils.hpp"

#include "GKDBusRemoteMethodCall.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

GKDBusRemoteMethodCall::GKDBusRemoteMethodCall(
	DBusConnection* const connection,
	const char* busName,
	const char* objectPath,
	const char* interface,
	const char* method,
	DBusPendingCall** pending)
		:	GKDBusMessage(connection), _pendingCall(pending)
{
	GK_LOG_FUNC

	if( ! dbus_validate_bus_name(busName, nullptr) )
		throw GKDBusMessageWrongBuild("invalid bus name");
	if( ! dbus_validate_path(objectPath, nullptr) )
		throw GKDBusMessageWrongBuild("invalid object path");
	if( ! dbus_validate_interface(interface, nullptr) )
		throw GKDBusMessageWrongBuild("invalid interface");
	if( ! dbus_validate_member(method, nullptr) )
		throw GKDBusMessageWrongBuild("invalid method name");

	_message = dbus_message_new_method_call(busName, objectPath, interface, method);
	if(_message == nullptr)
		throw GKDBusMessageWrongBuild("can't allocate memory for Remote Object Method Call DBus message");
	
	/* initialize potential arguments iterator */
	dbus_message_iter_init_append(_message, &_itMessage);

#if DEBUG_GKDBUS_SUBOBJECTS
	if(GKLogging::GKDebug) {
		LOG(trace) << "Remote Object Method Call DBus message initialized";
		LOG(trace) << "bus name    : " << busName;
		LOG(trace) << "object path : " << objectPath;
		LOG(trace) << "interface   : " << interface;
		LOG(trace) << "method      : " << method;
	}
#endif
}

GKDBusRemoteMethodCall::~GKDBusRemoteMethodCall()
{
	GK_LOG_FUNC

	if(_hosedMessage) {
		LOG(warning) << "DBus hosed message, giving up";
		dbus_message_unref(_message);
		return;
	}

	if( ! dbus_connection_send_with_reply(_connection, _message, _pendingCall, DBUS_TIMEOUT_USE_DEFAULT)) {
		dbus_message_unref(_message);
		LOG(error) << "DBus remote method call with pending reply sending failure";
		return;
	}

	dbus_connection_flush(_connection);
	dbus_message_unref(_message);

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "DBus remote method call with pending reply sent")
#endif
}

/* --- --- --- */
/* --- --- --- */
/* --- --- --- */

GKDBusMessageRemoteMethodCall::GKDBusMessageRemoteMethodCall()
	:	_remoteMethodCall(nullptr),
		_pendingCall(nullptr)
{
}

GKDBusMessageRemoteMethodCall::~GKDBusMessageRemoteMethodCall()
{
}

void GKDBusMessageRemoteMethodCall::initializeRemoteMethodCall(
	BusConnection wantedConnection,
	const char* busName,
	const char* objectPath,
	const char* interface,
	const char* method)
{
	this->initializeRemoteMethodCall(
		this->getConnection(wantedConnection),
		busName, objectPath, interface, method);
}

void GKDBusMessageRemoteMethodCall::initializeRemoteMethodCall(
	DBusConnection* const connection,
	const char* busName,
	const char* objectPath,
	const char* interface,
	const char* method)
{
	GK_LOG_FUNC

	if(_remoteMethodCall) /* sanity check */
		throw GKDBusMessageWrongBuild("DBus remote_method_call already allocated");

	try {
		_remoteMethodCall = new GKDBusRemoteMethodCall(
			connection, busName, objectPath, interface, method,
			&_pendingCall);
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		LOG(error) << "GKDBus remote_method_call allocation failure : " << e.what();
		throw GKDBusMessageWrongBuild("allocation error");
	}
}

void GKDBusMessageRemoteMethodCall::appendStringToRemoteMethodCall(const std::string & value)
{
	if(_remoteMethodCall != nullptr) /* sanity check */
		_remoteMethodCall->appendString(value);
}

void GKDBusMessageRemoteMethodCall::appendUInt8ToRemoteMethodCall(const uint8_t value)
{
	if(_remoteMethodCall != nullptr) /* sanity check */
		_remoteMethodCall->appendUInt8(value);
}

void GKDBusMessageRemoteMethodCall::appendUInt32ToRemoteMethodCall(const uint32_t value)
{
	if(_remoteMethodCall != nullptr) /* sanity check */
		_remoteMethodCall->appendUInt32(value);
}

void GKDBusMessageRemoteMethodCall::appendUInt64ToRemoteMethodCall(const uint64_t value)
{
	if(_remoteMethodCall != nullptr) /* sanity check */
		_remoteMethodCall->appendUInt64(value);
}

void GKDBusMessageRemoteMethodCall::appendMKeysIDToRemoteMethodCall(const GLogiK::MKeysID bankID)
{
	if(_remoteMethodCall != nullptr) /* sanity check */
		_remoteMethodCall->appendMKeysID(bankID);
}

void GKDBusMessageRemoteMethodCall::appendMacrosBankToRemoteMethodCall(const GLogiK::mBank_type & bank)
{
	if(_remoteMethodCall != nullptr) /* sanity check */
		_remoteMethodCall->appendMacrosBank(bank);
}

void GKDBusMessageRemoteMethodCall::sendRemoteMethodCall(void)
{
	GK_LOG_FUNC

	if(_remoteMethodCall) { /* sanity check */
		delete _remoteMethodCall;
		_remoteMethodCall = nullptr;
	}
	else {
		LOG(warning) << "tried to send NULL remote method call";
		throw GKDBusMessageWrongBuild("tried to send NULL remote method call");
	}
}

void GKDBusMessageRemoteMethodCall::abandonRemoteMethodCall(void)
{
	GK_LOG_FUNC

	if(_remoteMethodCall) { /* sanity check */
		_remoteMethodCall->abandon();
		delete _remoteMethodCall;
		_remoteMethodCall = nullptr;
	}
	else {
		LOG(warning) << "tried to abandon NULL remote method call";
	}
}

void GKDBusMessageRemoteMethodCall::waitForRemoteMethodCallReply(void)
{
	GK_LOG_FUNC

	dbus_pending_call_block(_pendingCall);

	uint16_t c = 0;

	DBusMessage* message = nullptr;
	// TODO could set a timer between retries ?
	while( message == nullptr and c < 10 ) {
		message = dbus_pending_call_steal_reply(_pendingCall);
		c++; /* bonus point */
	}

	dbus_pending_call_unref(_pendingCall);

	if(message == nullptr) {
		LOG(warning) << "message is NULL, retried 10 times";
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

	GKDBusArgumentString::fillInArguments(message);
	dbus_message_unref(message);
}

} // namespace NSGKDBus

