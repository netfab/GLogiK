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

#include "lib/utils/utils.h"

#include "GKDBusEventTwoStringsToStringsArray.h"

namespace GLogiK
{

void TwoStringsToStringsArrayEvent::runCallback(DBusConnection* connection, DBusMessage* message) {
	GKDBusArgumentString::fillInArguments(message);

	std::vector<std::string> ret;

	try {
		const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
		const std::string arg2( GKDBusArgumentString::getNextStringArgument() );

		/* call two strings to strings array callback */
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
		this->appendStringVectorToReply(ret);
	}
	catch (const GKDBusOOMWrongBuild & e) {
		LOG(ERROR) << "DBus build reply failure : " << e.what();
		/* delete reply object if allocated */
		this->sendReply();
		this->buildAndSendErrorReply(connection, message);
		return;
	}
	catch ( const GLogiKExcept & e ) {
		LOG(ERROR) << "DBus reply failure : " << e.what();
		/* delete reply object if allocated */
		this->sendReply();
		throw;
	}

	/* delete reply object if allocated */
	this->sendReply();
}


/* -- -- -- */

void EventTwoStringsToStringsArray::addTwoStringsToStringsArrayEvent(
	const char* object,
	const char* interface,
	const char* eventName,
	const std::vector<DBusMethodArgument> & args,
	std::function<const std::vector<std::string>(const std::string&, const std::string&)> callback,
	GKDBusEventType eventType,
	const bool introspectable
) {
	GKDBusEvent* e = new TwoStringsToStringsArrayEvent(eventName, args, callback, eventType, introspectable);
	this->addIntrospectableEvent(object, interface, e);
}

} // namespace GLogiK

