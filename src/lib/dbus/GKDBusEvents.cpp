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

#include "lib/utils/utils.h"

#include "GKDBusEvents.h"

namespace GLogiK
{

GKDBusEvents::GKDBusEvents() {
}

GKDBusEvents::~GKDBusEvents() {
}

void GKDBusEvents::defineRootNode(const std::string& rootnode) {
	this->root_node_ = rootnode;
}

const std::string & GKDBusEvents::getRootNode(void) {
	return this->root_node_;
}

const std::string GKDBusEvents::getNode(const std::string & object) {
	std::string node(this->root_node_);
	node += "/";
	node += object;
	return node;
}

void GKDBusEvents::addEvent_StringToBool_Callback(const char* object, const char* interface, const char* method,
	std::vector<DBusMethodArgument> args, std::function<const bool(const std::string&)> callback)
{
	GKDBusEvent_StringToBool_Callback e(method, args, callback);
	this->DBusObjects_[object] = true;
	this->events_string_to_bool_[object][interface].push_back(e);

	try {
		const auto & obj = this->events_string_to_string_.at(object);
		const auto & interf = obj.at("org.freedesktop.DBus.Introspectable");
	}
	catch (const std::out_of_range& oor) {
		LOG(DEBUG3) << "adding Introspectable interface : " << object << " " << interface;
		this->addEvent_StringToString_Callback(
			object, "org.freedesktop.DBus.Introspectable", "Introspect",
			{{"s", "xml_data", "out", "xml data representing DBus interfaces"}},
			std::bind(&GKDBusEvents::introspect, this, std::placeholders::_1));
	}
}

void GKDBusEvents::addEvent_VoidToString_Callback(const char* object, const char* interface, const char* method,
	std::vector<DBusMethodArgument> args, std::function<const std::string(void)> callback)
{
	GKDBusEvent_VoidToString_Callback e(method, args, callback);
	this->DBusObjects_[object] = true;
	this->events_void_to_string_[object][interface].push_back(e);

	// FIXME introspect
}

void GKDBusEvents::addEvent_StringToString_Callback(const char* object, const char* interface, const char* method,
	std::vector<DBusMethodArgument> args, std::function<const std::string(const std::string&)> callback)
{
	GKDBusEvent_StringToString_Callback e(method, args, callback);
	this->DBusObjects_[object] = true;
	this->events_string_to_string_[object][interface].push_back(e);

	// FIXME introspect
}

const std::string GKDBusEvents::introspect(const std::string & object_asked) {
	std::ostringstream xml;

	std::string asked_object_path = this->getNode(object_asked);

	LOG(DEBUG2) << "object path asked : " << asked_object_path;

	xml << "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n";
	xml << "		\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n";
	xml << "<node name=\"" << asked_object_path << "\">\n";

	for(const auto & object_it : this->events_string_to_string_) {
		/* object must match */
		if(object_it.first != object_asked)
			continue;
		for(const auto & interface : object_it.second) {
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

	for(const auto & object_it : this->events_void_to_string_) {
		/* object must match */
		if(object_it.first != object_asked)
			continue;
		for(const auto & interface : object_it.second) {
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

	for(const auto & object_it : this->events_string_to_bool_) {
		/* object must match */
		if(object_it.first != object_asked)
			continue;
		for(const auto & interface : object_it.second) {
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

const std::string GKDBusEvents::introspectRootNode(void) {
	std::ostringstream xml;

	LOG(DEBUG2) << "root node : " << this->root_node_;

	xml << "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n";
	xml << "		\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n";
	xml << "<node name=\"" << this->root_node_ << "\">\n";

	for(const auto & object_it : this->DBusObjects_) {
		xml << "  <node name=\"" << object_it.first << "\"/>\n";
	}

	xml << "</node>\n";
	return xml.str();
}

} // namespace GLogiK

