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

#ifndef __GLOGIK_GKDBUS_EVENT_TWO_STRINGS_ONE_BYTE_TO_BOOL_H__
#define __GLOGIK_GKDBUS_EVENT_TWO_STRINGS_ONE_BYTE_TO_BOOL_H__

#include <string>
#include <functional>

#include <dbus/dbus.h>

/* -- */
#include "GKDBusEvent.h"

#include "lib/dbus/GKDBusConnection.h"
#include "lib/dbus/arguments/GKDBusArgString.h"
#include "lib/dbus/arguments/GKDBusArgByte.h"

#include "lib/dbus/messages/GKDBusReply.h"
#include "lib/dbus/messages/GKDBusErrorReply.h"

namespace NSGKDBus
{

class TwoStringsOneByteToBoolEvent
 :
	public GKDBusEvent,
	virtual public GKDBusArgumentString,
	public GKDBusArgumentByte,
	public GKDBusMessageReply,
	public GKDBusMessageErrorReply
{
	public:
		TwoStringsOneByteToBoolEvent(
			const char* n,
			const std::vector<DBusMethodArgument> & a,
			std::function<const bool(const std::string&, const std::string&, const uint8_t)> c,
			GKDBusEventType t,
			const bool i
			)	: GKDBusEvent(n, a, t, i), callback(c) {}
		~TwoStringsOneByteToBoolEvent() = default;

		void runCallback(DBusConnection* connection, DBusMessage* message);

	private:
		std::function<const bool(const std::string&, const std::string&, const uint8_t)> callback;

};

class EventTwoStringsOneByteToBool
	:	public EventCanBeSignal
{
	public:
		void addTwoStringsOneByteToBoolEvent(
			const BusConnection bus,
			const char* object,
			const char* interface,
			const char* eventName,
			const std::vector<DBusMethodArgument> & args,
			std::function<const bool(const std::string&, const std::string&, const uint8_t)> callback,
			GKDBusEventType t=GKDBusEventType::GKDBUS_EVENT_METHOD,
			const bool introspectable=true
		);

		void addTwoStringsOneByteToBoolSignal(
			const BusConnection bus,
			const char* object,
			const char* interface,
			const char* eventName,
			const std::vector<DBusMethodArgument> & args,
			std::function<const bool(const std::string&, const std::string&, const uint8_t)> callback
		);

	protected:
		EventTwoStringsOneByteToBool() = default;
		virtual ~EventTwoStringsOneByteToBool() = default;

		virtual void addIntrospectableEvent(
			const BusConnection bus,
			const char* object,
			const char* interface,
			GKDBusEvent* event
		) = 0;

};

} // namespace NSGKDBus

#endif