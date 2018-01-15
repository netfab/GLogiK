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

#ifndef __GLOGIK_GKDBUS_EVENT_VOID_TO_VOID_H__
#define __GLOGIK_GKDBUS_EVENT_VOID_TO_VOID_H__

#include <vector>

#include <functional>

#include <dbus/dbus.h>

/* -- */

#include "GKDBusEvent.h"

#include "lib/dbus/GKDBusConnection.h"


namespace NSGKDBus
{

class VoidToVoidEvent
	 :	public GKDBusEvent
{
	public:
		VoidToVoidEvent(
			const char* n,
			const std::vector<DBusMethodArgument> & a,
			std::function<void(void)> c,
			GKDBusEventType t,
			const bool i
			)	: GKDBusEvent(n, a, t, i), callback(c) {}
		~VoidToVoidEvent() = default;

		void runCallback(DBusConnection* connection, DBusMessage* message);

	private:
		std::function<void(void)> callback;
};

class EventVoidToVoid
	:	public EventCanBeSignal
{
	public:
		void addVoidToVoidEvent(
			const BusConnection bus,
			const char* object,
			const char* interface,
			const char* eventName,
			const std::vector<DBusMethodArgument> & args,
			std::function<void(void)> callback,
			GKDBusEventType t=GKDBusEventType::GKDBUS_EVENT_METHOD,
			const bool introspectable=true
		);

		void addVoidToVoidSignal(
			const BusConnection bus,
			const char* object,
			const char* interface,
			const char* eventName,
			const std::vector<DBusMethodArgument> & args,
			std::function<void(void)> callback
		);

	protected:
		EventVoidToVoid() = default;
		virtual ~EventVoidToVoid() = default;

		virtual void addIntrospectableEvent(
			const BusConnection bus,
			const char* object,
			const char* interface,
			GKDBusEvent* event
		) = 0;
};

} // namespace NSGKDBus

#endif
