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

#include "lib/utils/utils.h"

#include "GKDBusEventTwoStringsToString.h"

namespace GLogiK
{

void TwoStringsToStringEvent::runCallback(DBusConnection* connection, DBusMessage* message) {
	GKDBusArgumentString::fillInArguments(message);

	std::string ret;

	try {
	 const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
	 const std::string arg2( GKDBusArgumentString::getNextStringArgument() );

		/* call two strings to string callback */
		ret = this->callback(arg1, arg2);
	}
	catch ( const EmptyContainer & e ) {
		LOG(WARNING) << e.what();
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendStringToReply(ret);
	}
	catch ( const GLogiKExcept & e ) {
		LOG(ERROR) << "DBus reply failure : " << e.what();
		/* delete reply object if allocated */
		this->sendReply();
		this->buildAndSendErrorReply(connection, message);
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}


/* -- -- -- */

void EventTwoStringsToString::addTwoStringsToStringEvent(
	const BusConnection bus,
	const char* object,
	const char* interface,
	const char* eventName,
	const std::vector<DBusMethodArgument> & args,
	std::function<const std::string(const std::string&, const std::string&)> callback,
	GKDBusEventType eventType,
	const bool introspectable
) {
	GKDBusEvent* e = new TwoStringsToStringEvent(eventName, args, callback, eventType, introspectable);
	this->addIntrospectableEvent(bus, object, interface, e);
}

} // namespace GLogiK

