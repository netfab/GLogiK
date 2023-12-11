/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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

#include "lib/dbus/messages/GKDBusReply.hpp"
#include "lib/dbus/messages/GKDBusErrorReply.hpp"

namespace NSGKDBus
{

enum class GKDBusEventType : uint8_t
{
	GKDBUS_EVENT_METHOD = 0,
	GKDBUS_EVENT_SIGNAL
};

/* structure for introspection */
struct DBusMethodArgument {
	const std::string type;
	const std::string name;
	const std::string direction;
	const std::string comment;
};

class GKDBusEvent
	:	protected GKDBusMessageReply,
		protected GKDBusMessageErrorReply
{
	public:
		const std::string eventName;
		std::vector<DBusMethodArgument> arguments;
		GKDBusEventType eventType;
		const bool introspectable;

		virtual void runCallback(
			DBusConnection* const connection,
			DBusMessage* message,
			DBusMessage* asyncContainer
		) = 0;

		virtual ~GKDBusEvent(void);

	protected:
		GKDBusEvent(
			const char* n,
			const std::vector<DBusMethodArgument> & a,
			GKDBusEventType t,
			const bool i
		);

		void sendReplyError(
			DBusConnection* const connection,
			DBusMessage* message,
			const char* errorString
		);

		void sendCallbackError(
			DBusConnection* const connection,
			DBusMessage* message,
			const char* errorString
		);

	private:
		GKDBusEvent(void) = delete;
};

class GKDBusIntrospectableSignal
{
	public:
		GKDBusIntrospectableSignal(
			const char* name,
			const std::vector<DBusMethodArgument> & args
		);
		~GKDBusIntrospectableSignal() = default;

		const std::string name;
		const std::vector<DBusMethodArgument> arguments;

	protected:
	private:
		GKDBusIntrospectableSignal(void) = delete;
};

} // namespace NSGKDBus

#endif
