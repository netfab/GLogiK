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

#ifndef __GLOGIK_GKDBUS_EVENTS_H__
#define __GLOGIK_GKDBUS_EVENTS_H__

#if !defined (GKDBUS_INSIDE_GKDBUS_H) && !defined (GKDBUS_COMPILATION)
#error "Only "dbus/GKDBus.h" can be included directly, this file may disappear or change contents."
#endif

#include <string>
#include <vector>
#include <map>
#include <functional>

namespace GLogiK
{

enum class GKDBusEventType : int8_t
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

struct GKDBusEvent_StringToBool_Callback {
	const std::string eventName;
	std::vector<DBusMethodArgument> arguments;
	std::function<const bool(const std::string&)> callback;
	GKDBusEventType eventType;

	GKDBusEvent_StringToBool_Callback(const char* n, std::vector<DBusMethodArgument> & a,
		std::function<const bool(const std::string&)> c, GKDBusEventType t)
		: eventName(n), arguments(a), callback(c), eventType(t) {}
};

struct GKDBusEvent_TwoStringsToBool_Callback {
	const std::string eventName;
	std::vector<DBusMethodArgument> arguments;
	std::function<const bool(const std::string&, const std::string&)> callback;
	GKDBusEventType eventType;

	GKDBusEvent_TwoStringsToBool_Callback(const char* n, std::vector<DBusMethodArgument> & a,
		std::function<const bool(const std::string&, const std::string&)> c, GKDBusEventType t)
		: eventName(n), arguments(a), callback(c), eventType(t) {}
};

struct GKDBusEvent_VoidToString_Callback {
	const std::string eventName;
	std::vector<DBusMethodArgument> arguments;
	std::function<const std::string(void)> callback;
	GKDBusEventType eventType;

	GKDBusEvent_VoidToString_Callback(const char* n, std::vector<DBusMethodArgument> & a,
		std::function<const std::string(void)> c, GKDBusEventType t)
		: eventName(n), arguments(a), callback(c), eventType(t) {}
};

struct GKDBusEvent_StringToString_Callback {
	const std::string eventName;
	std::vector<DBusMethodArgument> arguments;
	std::function<const std::string(const std::string&)> callback;
	GKDBusEventType eventType;

	GKDBusEvent_StringToString_Callback(const char* n, std::vector<DBusMethodArgument> & a,
		std::function<const std::string(const std::string&)> c, GKDBusEventType t)
		: eventName(n), arguments(a), callback(c), eventType(t) {}
};

class GKDBusEvents
{
	public:
		void addEvent_StringToBool_Callback(const char* object, const char* interface, const char* eventName,
			std::vector<DBusMethodArgument> args, std::function<const bool(const std::string&)> callback,
			GKDBusEventType t=GKDBusEventType::GKDBUS_EVENT_METHOD);
		void addEvent_TwoStringsToBool_Callback(const char* object, const char* interface, const char* eventName,
			std::vector<DBusMethodArgument> args, std::function<const bool(const std::string&, const std::string&)> callback,
			GKDBusEventType t=GKDBusEventType::GKDBUS_EVENT_METHOD);
		void addEvent_VoidToString_Callback(const char* object, const char* interface, const char* eventName,
			std::vector<DBusMethodArgument> args, std::function<const std::string(void)> callback,
			GKDBusEventType t=GKDBusEventType::GKDBUS_EVENT_METHOD);
		void addEvent_StringToString_Callback(const char* object, const char* interface, const char* eventName,
			std::vector<DBusMethodArgument> args, std::function<const std::string(const std::string&)> callback,
			GKDBusEventType t=GKDBusEventType::GKDBUS_EVENT_METHOD);

	protected:
		GKDBusEvents(void);
		~GKDBusEvents(void);

		std::map< const std::string, /* object path */
			std::map< const std::string, /* interface */
				std::vector<GKDBusEvent_StringToBool_Callback> > > events_string_to_bool_;

		std::map< const std::string, /* object path */
			std::map< const std::string, /* interface */
				std::vector<GKDBusEvent_TwoStringsToBool_Callback> > > events_twostrings_to_bool_;

		std::map< const std::string, /* object path */
			std::map< const std::string, /* interface */
				std::vector<GKDBusEvent_VoidToString_Callback> > > events_void_to_string_;

		std::map< const std::string, /* object path */
			std::map< const std::string, /* interface */
				std::vector<GKDBusEvent_StringToString_Callback> > > events_string_to_string_;

		std::map<const std::string, bool> DBusObjects_;
		std::map<const std::string, bool> DBusInterfaces_;

		void defineRootNode(const std::string& rootnode);
		const std::string & getRootNode(void);
		const std::string getNode(const std::string & object);

		const std::string introspectRootNode(void);

	private:
		std::string root_node_;

		const std::string introspect(const std::string & object_path_asked);
		void addIntrospectableEvent(const char* object);
};

} // namespace GLogiK

#endif
