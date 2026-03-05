/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2026  Fabrice Delliaux <netbox253@gmail.com>
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

DBusEvent::DBusEvent(
	const std::string & name,
	const std::string & sender,
	const std::vector<DBusEventArgument> & args,
	DBusEventType type,
	const bool intr)
		:	arguments(args),
			eventName(name),
			eventSender(sender),
			eventType(type),
			introspectable(intr)
{
}

DBusEvent::~DBusEvent()
{
	GK_LOG_FUNC

	std::string tr("destroying ");
	tr += (this->eventType == DBusEventType::DBUS_SIGNAL_EVENT) ? "signal" : "method";
	tr += " event: ";
	tr += this->eventName;

	GKLog(trace, tr)
}

void DBusEvent::callback(
	DBusConnection* const connection,
	DBusMessage* message,
	DBusMessage* asyncContainer)
{
	this->runCallback(connection, message, asyncContainer);
}

} // namespace NSGKDBus

