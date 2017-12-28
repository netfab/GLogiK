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

#ifndef __GLOGIK_GKDBUS_EVENT_H__
#define __GLOGIK_GKDBUS_EVENT_H__

#include <cstdint>

#include <string>
#include <vector>

#include <dbus/dbus.h>

namespace GLogiK
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

class GKDBusEvent {
	public:
		const std::string eventName;
		std::vector<DBusMethodArgument> arguments;
		GKDBusEventType eventType;

		GKDBusEvent(
			const char* n,
			const std::vector<DBusMethodArgument> & a,
			GKDBusEventType t
		);
		virtual ~GKDBusEvent(void);

		virtual void runCallback(DBusConnection* connection, DBusMessage* message) = 0;

	protected:
};

} // namespace GLogiK

#endif
