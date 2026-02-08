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

#ifndef SRC_LIB_DBUS_EVENTS_GKDBUS_EVENT_HPP_
#define SRC_LIB_DBUS_EVENTS_GKDBUS_EVENT_HPP_

#include <cstdint>

#include <string>
#include <vector>

#include <dbus/dbus.h>

namespace NSGKDBus
{

enum class DBusEventType : uint8_t
{
	DBUS_METHOD_EVENT = 0,
	DBUS_SIGNAL_EVENT
};

/* structure for introspection */
struct DBusEventArgument
{
	const std::string type;
	const std::string name;
	const std::string direction;
	const std::string comment;
};

class DBusEvent
{
	public:
		const std::string eventName;
		std::vector<DBusEventArgument> arguments;
		DBusEventType eventType;
		const bool introspectable;

		void callback(
			DBusConnection* const connection,
			DBusMessage* message,
			DBusMessage* asyncContainer
		);

		virtual ~DBusEvent(void);

	protected:
		DBusEvent(
			const std::string & name,
			const std::vector<DBusEventArgument> & args,
			DBusEventType type,
			const bool intr
		);

	private:
		DBusEvent(void) = delete;

		virtual void runCallback(
			DBusConnection* const connection,
			DBusMessage* message,
			DBusMessage* asyncContainer
		) = 0;
};


class GKDBusEvent
	:	public DBusEvent
{
	public:
		~GKDBusEvent(void) = default;

	protected:
		GKDBusEvent(
			const std::string & name,
			const std::vector<DBusEventArgument> & args,
			DBusEventType type,
			const bool intr)
				:	DBusEvent(name, args, type, intr) {}

	private:
		GKDBusEvent(void) = delete;
};


class introspectableSignalEvent
	:	public DBusEvent
{
	public:
		introspectableSignalEvent(
			const std::string & name,
			const std::vector<DBusEventArgument> & args)
				:	DBusEvent(name, args, DBusEventType::DBUS_SIGNAL_EVENT, true) {}
		~introspectableSignalEvent(void) = default;

	protected:

	private:
		introspectableSignalEvent(void) = delete;

		inline void runCallback(
			DBusConnection* const connection,
			DBusMessage* message,
			DBusMessage* asyncContainer
		) {	/* do nothing */ }
};

} // namespace NSGKDBus

#endif
