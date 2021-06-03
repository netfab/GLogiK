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

#include "lib/utils/utils.hpp"

#include "GKDBusEvent.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

GKDBusEvent::GKDBusEvent(
	const char* n,
	const std::vector<DBusMethodArgument> & a,
	GKDBusEventType t,
	const bool i
	)	: eventName(n), arguments(a), eventType(t), introspectable(i)
{
}

GKDBusEvent::~GKDBusEvent()
{
	GK_LOG_FUNC

	GKLog2(trace, "destroying event : ", eventName)
}

/*
 * exception was thrown while building reply
 */
void GKDBusEvent::sendReplyError(
	DBusConnection* const connection,
	DBusMessage* message,
	const char* errorString)
{
	GK_LOG_FUNC

	LOG(error) << "DBus reply failure : " << errorString;
	this->abandonReply();	/* delete reply object if allocated */
	this->buildAndSendErrorReply(connection, message, errorString);
}

/*
 * exception was thrown before or while running callback
 */
void GKDBusEvent::sendCallbackError(
	DBusConnection* const connection,
	DBusMessage* message,
	const char* errorString)
{
	GK_LOG_FUNC

	LOG(error) << errorString;

	if(this->eventType != GKDBusEventType::GKDBUS_EVENT_SIGNAL) {
		/* send error if something was wrong when running callback */
		this->buildAndSendErrorReply(connection, message, errorString);
	}
}

GKDBusIntrospectableSignal::GKDBusIntrospectableSignal(
	const char* signalName,
	const std::vector<DBusMethodArgument> & args)
		:	name(signalName), arguments(args)
{
}


} // namespace NSGKDBus

