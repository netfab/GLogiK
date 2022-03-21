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

#include "lib/utils/utils.hpp"

#include "GKDBusBroadcastSignal.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

GKDBusBroadcastSignal::GKDBusBroadcastSignal(
	DBusConnection* const connection,
	const char* destination,	/* destination, if NULL, broadcast */
	const char* objectPath,		/* the path to the object emitting the signal */
	const char* interface,		/* interface the signal is emitted from */
	const char* signal			/* name of signal */
	) : GKDBusMessage(connection)
{
	GK_LOG_FUNC

	if( ! dbus_validate_path(objectPath, nullptr) )
		throw GKDBusMessageWrongBuild("invalid object path");
	if( ! dbus_validate_interface(interface, nullptr) )
		throw GKDBusMessageWrongBuild("invalid interface");
	if( ! dbus_validate_member(signal, nullptr) )
		throw GKDBusMessageWrongBuild("invalid signal name");

	_message = dbus_message_new_signal(objectPath, interface, signal);
	if(_message == nullptr)
		throw GKDBusMessageWrongBuild("can't allocate memory for Signal DBus message");

	if( destination != nullptr ) {
#if DEBUG_GKDBUS_SUBOBJECTS
		GKLog2(trace, "prepare sending signal to ", destination)
#endif
		dbus_message_set_destination(_message, destination);
	}

	/* initialize potential arguments iterator */
	dbus_message_iter_init_append(_message, &_itMessage);
#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "DBus signal initialized")
#endif
}

GKDBusBroadcastSignal::~GKDBusBroadcastSignal()
{
	GK_LOG_FUNC

	if(_hosedMessage) {
		LOG(warning) << "DBus hosed message, giving up";
		dbus_message_unref(_message);
		return;
	}

	// TODO dbus_uint32_t serial;
	if( ! dbus_connection_send(_connection, _message, nullptr) ) {
		dbus_message_unref(_message);
		LOG(error) << "DBus signal sending failure";
		return;
	}

	dbus_connection_flush(_connection);
	dbus_message_unref(_message);
#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "DBus signal sent")
#endif
}


/* --- --- --- */
/* --- --- --- */
/* --- --- --- */


GKDBusMessageBroadcastSignal::GKDBusMessageBroadcastSignal()
	:	_signal(nullptr)
{
}

GKDBusMessageBroadcastSignal::~GKDBusMessageBroadcastSignal()
{
}

void GKDBusMessageBroadcastSignal::initializeBroadcastSignal(
	BusConnection wantedConnection,
	const char* objectPath,
	const char* interface,
	const char* signal)
{
	this->initializeBroadcastSignal(
		this->getConnection(wantedConnection),
		objectPath, interface, signal);
}

void GKDBusMessageBroadcastSignal::initializeBroadcastSignal(
	DBusConnection* const connection,
	const char* objectPath,
	const char* interface,
	const char* signal)
{
	GK_LOG_FUNC

	if(_signal) /* sanity check */
		throw GKDBusMessageWrongBuild("DBus signal already allocated");

	try {
		_signal = new GKDBusBroadcastSignal(connection, nullptr, objectPath, interface, signal);
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		LOG(error) << "GKDBus broadcast signal allocation failure : " << e.what();
		throw GKDBusMessageWrongBuild("allocation error");
	}
}

void GKDBusMessageBroadcastSignal::appendStringToBroadcastSignal(const std::string & value)
{
	if(_signal != nullptr) /* sanity check */
		_signal->appendString(value);
}

void GKDBusMessageBroadcastSignal::appendUInt8ToBroadcastSignal(const uint8_t value)
{
	if(_signal != nullptr) /* sanity check */
		_signal->appendUInt8(value);
}

void GKDBusMessageBroadcastSignal::appendGKeysIDToBroadcastSignal(const GLogiK::GKeysID keyID)
{
	if(_signal != nullptr) /* sanity check */
		_signal->appendGKeysID(keyID);
}

void GKDBusMessageBroadcastSignal::appendMKeysIDToBroadcastSignal(const GLogiK::MKeysID bankID)
{
	if(_signal != nullptr) /* sanity check */
		_signal->appendMKeysID(bankID);
}

void GKDBusMessageBroadcastSignal::appendStringVectorToBroadcastSignal(const std::vector<std::string> & list)
{
	if(_signal != nullptr) /* sanity check */
		_signal->appendStringVector(list);
}

void GKDBusMessageBroadcastSignal::sendBroadcastSignal(void)
{
	GK_LOG_FUNC

	if(_signal) { /* sanity check */
		delete _signal;
		_signal = nullptr;
	}
	else {
		LOG(warning) << "tried to send NULL signal";
		throw GKDBusMessageWrongBuild("tried to send NULL signal");
	}
}

void GKDBusMessageBroadcastSignal::abandonBroadcastSignal(void)
{
	GK_LOG_FUNC

	if(_signal) { /* sanity check */
		_signal->abandon();
		delete _signal;
		_signal = nullptr;
	}
	else {
		LOG(warning) << "tried to abandon NULL signal";
	}
}

} // namespace NSGKDBus

