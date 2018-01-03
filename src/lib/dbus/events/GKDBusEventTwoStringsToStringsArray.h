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

#ifndef __GLOGIK_GKDBUS_EVENT_TWO_STRINGS_TO_STRINGS_ARRAY_H__
#define __GLOGIK_GKDBUS_EVENT_TWO_STRINGS_TO_STRINGS_ARRAY_H__

#include <string>
#include <vector>
#include <functional>

#include <dbus/dbus.h>

/* -- */
#include "GKDBusEvent.h"

#include "lib/dbus/arguments/GKDBusArgString.h"

#include "lib/dbus/messages/GKDBusReply.h"
#include "lib/dbus/messages/GKDBusErrorReply.h"

namespace GLogiK
{

class TwoStringsToStringsArrayEvent
 :
	public GKDBusEvent,
	virtual public GKDBusArgumentString,
	public GKDBusMessageReply,
	public GKDBusMessageErrorReply
{
	public:
		TwoStringsToStringsArrayEvent(
			const char* n,
			const std::vector<DBusMethodArgument> & a,
			std::function<const std::vector<std::string>(const std::string&, const std::string&)> c,
			GKDBusEventType t,
			const bool i
			)	: GKDBusEvent(n, a, t, i), callback(c) {}
		~TwoStringsToStringsArrayEvent() {};

		void runCallback(DBusConnection* connection, DBusMessage* message);

	private:
		std::function<const std::vector<std::string>(const std::string&, const std::string&)> callback;

};

class EventTwoStringsToStringsArray
{
	public:
		void addTwoStringsToStringsArrayEvent(
			const char* object,
			const char* interface,
			const char* eventName,
			const std::vector<DBusMethodArgument> & args,
			std::function<const std::vector<std::string>(const std::string&, const std::string&)> callback,
			GKDBusEventType t=GKDBusEventType::GKDBUS_EVENT_METHOD,
			const bool introspectable=true
		);

	protected:
		EventTwoStringsToStringsArray() = default;
		virtual ~EventTwoStringsToStringsArray() = default;

		virtual void addIntrospectableEvent(const char* object, const char* interface, GKDBusEvent* event) = 0;

};

} // namespace GLogiK

#endif
