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

#include "GKDBusEventStringToString.h"

namespace GLogiK
{

void StringToStringEvent::runCallback(DBusConnection* connection, DBusMessage* message) {
	CBArgument::fillInArguments(message);

	/* call string to string callback */
	const std::string arg( this->getNextStringArgument() );
	const std::string ret( this->callback(arg) );

	try {
		this->initializeReply(connection, message);
		this->appendStringToReply(ret);
		/* extra values to send */
		//this->appendExtraStringsValuesToReply();
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

void EventStringToString::addStringToStringEvent(
	const char* object,
	const char* interface,
	const char* eventName,
	const std::vector<DBusMethodArgument> & args,
	std::function<const std::string(const std::string&)> callback,
	GKDBusEventType eventType
) {
	GKDBusEvent* e = new StringToStringEvent(eventName, args, callback, eventType);
	this->addIntrospectableEvent(object, interface, e);
}

} // namespace GLogiK

