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

#include "GKDBusEventStringToVoid.h"

namespace NSGKDBus
{

using namespace NSGKUtils;

void StringToVoidEvent::runCallback(DBusConnection* connection, DBusMessage* message) {
	GKDBusArgumentString::fillInArguments(message);

	try {
		const std::string arg( GKDBusArgumentString::getNextStringArgument() );

		/* call string to void callback */
		this->callback(arg);
	}
	catch ( const EmptyContainer & e ) {
		LOG(WARNING) << e.what();
	}

	/* don't need to send a reply */
}


/* -- -- -- */

void EventStringToVoid::addStringToVoidEvent(
	const BusConnection bus,
	const char* object,
	const char* interface,
	const char* eventName,
	const std::vector<DBusMethodArgument> & args,
	std::function<void(const std::string&)> callback,
	GKDBusEventType eventType,
	const bool introspectable
) {
	GKDBusEvent* e = new StringToVoidEvent(eventName, args, callback, eventType, introspectable);
	this->addIntrospectableEvent(bus, object, interface, e);
}

void EventStringToVoid::addStringToVoidSignal(
	const BusConnection bus,
	const char* object,
	const char* interface,
	const char* eventName,
	const std::vector<DBusMethodArgument> & args,
	std::function<void(const std::string&)> callback
) {
	this->addStringToVoidEvent(bus, object, interface, eventName, args, callback, GKDBusEventType::GKDBUS_EVENT_SIGNAL, true);
	DBusConnection* connection = this->getConnection(bus);
	this->addSignalRuleMatch(connection, interface, eventName);
}

} // namespace NSGKDBus

