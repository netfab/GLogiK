/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2025  Fabrice Delliaux <netbox253@gmail.com>
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

#include "callback.hpp"

namespace NSGKDBus
{

/*
 * SIGs2s explicit specialization functions implementations
 */

/*
void Callback<SIGs2s>::exposeMethod(
		const BusConnection bus,
		const char* objectPath,
		const char* interface,
		const char* eventName,
		const std::vector<DBusMethodArgument> & args,
		SIGs2s callback
	)
{
	this->exposeEvent(bus, nullptr, objectPath, interface, eventName, args, callback, GKDBusEventType::GKDBUS_EVENT_METHOD, true);
}

void Callback<SIGs2s>::receiveSignal(
		const BusConnection bus,
		const char* sender,
		const char* objectPath,
		const char* interface,
		const char* eventName,
		const std::vector<DBusMethodArgument> & args,
		SIGs2s callback
	)
{
	// signals declared as events with callback functions are not introspectable
	this->exposeEvent(bus, sender, objectPath, interface, eventName, args, callback, GKDBusEventType::GKDBUS_EVENT_SIGNAL, false);
}
*/

void Callback<SIGs2s>::exposeEvent(
		const BusConnection bus,
		const char* sender,
		const char* objectPath,
		const char* interface,
		const char* eventName,
		const std::vector<DBusMethodArgument> & args,
		SIGs2s callback,
		GKDBusEventType eventType,
		const bool introspectable
	)
{
	GKDBusEvent* event = nullptr;
	try {
		event = new callbackEvent<SIGs2s>(eventName, args, callback, eventType, introspectable);
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		throw NSGKUtils::GLogiKBadAlloc("DBus event bad allocation");
	}

	this->addEvent(bus, sender, objectPath, interface, event);
}

} // namespace NSGKDBus

