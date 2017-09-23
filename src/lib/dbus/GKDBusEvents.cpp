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

#include <sstream>
#include <stdexcept>
#include <functional>

#include "GKDBusEvents.h"
#include "include/log.h"

namespace GLogiK
{

GKDBusEvents::GKDBusEvents() {
}

GKDBusEvents::~GKDBusEvents() {
}

void GKDBusEvents::addEvent_StringToBool_Callback(const char* object_path, const char* interface, const char* method,
	std::vector<DBusMethodArgument> args, std::function<const bool(const std::string&)> callback)
{
	GKDBusEvent_StringToBool_Callback e(method, args, callback);
	this->events_string_to_bool_[object_path][interface].push_back(e);

	try {
		auto obj = this->events_string_to_string_.at(object_path);
		auto interf = obj.at("org.freedesktop.DBus.Introspectable");
	}
	catch (const std::out_of_range& oor) {
		LOG(DEBUG3) << "adding Introspectable interface : " << object_path << " " << interface;
		this->addEvent_StringToString_Callback(
			object_path, "org.freedesktop.DBus.Introspectable", "Introspect",
			{{"s", "xml_data", "out", "xml data representing DBus interfaces"}},
			std::bind(&GKDBusEvents::introspect, this, std::placeholders::_1));
	}
}

void GKDBusEvents::addEvent_VoidToString_Callback(const char* object_path, const char* interface, const char* method,
	std::vector<DBusMethodArgument> args, std::function<const std::string(void)> callback)
{
	GKDBusEvent_VoidToString_Callback e(method, args, callback);
	this->events_void_to_string_[object_path][interface].push_back(e);

	// FIXME introspect
}

void GKDBusEvents::addEvent_StringToString_Callback(const char* object_path, const char* interface, const char* method,
	std::vector<DBusMethodArgument> args, std::function<const std::string(const std::string&)> callback)
{
	GKDBusEvent_StringToString_Callback e(method, args, callback);
	this->events_string_to_string_[object_path][interface].push_back(e);

	// FIXME introspect
}

const std::string GKDBusEvents::introspect(const std::string & object_path_asked) {
	LOG(DEBUG2) << "object path asked : " << object_path_asked;

	std::ostringstream xml;
	xml << "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n";
	xml << "		\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n";
	xml << "<node name=\"" << object_path_asked << "\">\n";

	for(const auto & object_path : this->events_string_to_string_) {
		/* object path must match */
		if(object_path.first != object_path_asked)
			continue;
		for(const auto & interface : object_path.second) {
			xml << "  <interface name=\"" << interface.first << "\">\n";
			for(const auto & DBusEvent : interface.second) { /* vector of struct */
				xml << "    <method name=\"" << DBusEvent.method << "\">\n";
				for(const auto & arg : DBusEvent.arguments) {
					xml << "      <!-- " << arg.comment << " -->\n";
					xml << "      <arg type=\"" << arg.type << "\" name=\"" << arg.name << "\" direction=\"" << arg.direction << "\" />\n";
				}
				xml << "    </method>\n";
			}
			xml << "  </interface>\n";
		}
	}

	for(const auto & object_path : this->events_void_to_string_) {
		/* object path must match */
		if(object_path.first != object_path_asked)
			continue;
		for(const auto & interface : object_path.second) {
			xml << "  <interface name=\"" << interface.first << "\">\n";
			for(const auto & DBusEvent : interface.second) { /* vector of struct */
				xml << "    <method name=\"" << DBusEvent.method << "\">\n";
				for(const auto & arg : DBusEvent.arguments) {
					xml << "      <!-- " << arg.comment << " -->\n";
					xml << "      <arg type=\"" << arg.type << "\" name=\"" << arg.name << "\" direction=\"" << arg.direction << "\" />\n";
				}
				xml << "    </method>\n";
			}
			xml << "  </interface>\n";
		}
	}

	for(const auto & object_path : this->events_string_to_bool_) {
		/* object path must match */
		if(object_path.first != object_path_asked)
			continue;
		for(const auto & interface : object_path.second) {
			xml << "  <interface name=\"" << interface.first << "\">\n";
			for(const auto & DBusEvent : interface.second) { /* vector of struct */
				xml << "    <method name=\"" << DBusEvent.method << "\">\n";
				for(const auto & arg : DBusEvent.arguments) {
					xml << "      <!-- " << arg.comment << " -->\n";
					xml << "      <arg type=\"" << arg.type << "\" name=\"" << arg.name << "\" direction=\"" << arg.direction << "\" />\n";
				}
				xml << "    </method>\n";
			}
			xml << "  </interface>\n";
		}
	}

	xml << "</node>\n";
	return xml.str();
}

} // namespace GLogiK

