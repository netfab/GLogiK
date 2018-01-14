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

#include "GKDBusEventVoidToVoid.h"

namespace GLogiK
{

void VoidToVoidEvent::runCallback(DBusConnection* connection, DBusMessage* message) {
	/* don't need arguments */

	/* call void to void callback */
	this->callback();

	/* don't need to send a reply */
}


/* -- -- -- */

void EventVoidToVoid::addVoidToVoidEvent(
	const BusConnection bus,
	const char* object,
	const char* interface,
	const char* eventName,
	const std::vector<DBusMethodArgument> & args,
	std::function<void(void)> callback,
	GKDBusEventType eventType,
	const bool introspectable
) {
	GKDBusEvent* e = new VoidToVoidEvent(eventName, args, callback, eventType, introspectable);
	this->addIntrospectableEvent(bus, object, interface, e);
}

void EventVoidToVoid::addVoidToVoidSignal(
	const BusConnection bus,
	const char* object,
	const char* interface,
	const char* eventName,
	const std::vector<DBusMethodArgument> & args,
	std::function<void(void)> callback
) {
	this->addVoidToVoidEvent(bus, object, interface, eventName, args, callback, GKDBusEventType::GKDBUS_EVENT_SIGNAL, true);
	DBusConnection* connection = this->getConnection(bus);
	this->addSignalRuleMatch(connection, interface, eventName);
}

} // namespace GLogiK

