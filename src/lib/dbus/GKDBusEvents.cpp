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

#include <stdexcept>
#include <sstream>
#include <functional>

#include "lib/utils/utils.h"

#define GKDBUS_COMPILATION 1
#include "GKDBusEvents.h"
#undef GKDBUS_COMPILATION

namespace GLogiK
{

GKDBusEvent::GKDBusEvent(const char* n, std::vector<DBusMethodArgument> & a, GKDBusEventType t)
	: eventName(n), arguments(a), eventType(t)
{
}

GKDBusEvent::~GKDBusEvent() {
}

/* -- */

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

void GKDBusEvents::addEvent_StringToBool_Callback(const char* object, const char* interface, const char* eventName,
	std::vector<DBusMethodArgument> args, std::function<const bool(const std::string&)> callback,
	GKDBusEventType eventType)
{
	GKDBusEvent_StringToBool_Callback e(eventName, args, callback, eventType);
	this->DBusObjects_[object] = true;
	this->DBusInterfaces_[interface] = true;
	this->events_string_to_bool_[object][interface].push_back(e);

	this->addIntrospectableEvent(object);
}

void GKDBusEvents::addEvent_TwoStringsToBool_Callback(const char* object, const char* interface, const char* eventName,
	std::vector<DBusMethodArgument> args, std::function<const bool(const std::string&, const std::string&)> callback,
	GKDBusEventType eventType)
{
	GKDBusEvent_TwoStringsToBool_Callback e(eventName, args, callback, eventType);
	this->DBusObjects_[object] = true;
	this->DBusInterfaces_[interface] = true;
	this->events_twostrings_to_bool_[object][interface].push_back(e);

	this->addIntrospectableEvent(object);
}

void GKDBusEvents::addEvent_VoidToVoid_Callback(const char* object, const char* interface, const char* eventName,
	std::vector<DBusMethodArgument> args, std::function<void(void)> callback,
	GKDBusEventType eventType)
{
	GKDBusEvent_VoidToVoid_Callback e(eventName, args, callback, eventType);
	this->DBusObjects_[object] = true;
	this->DBusInterfaces_[interface] = true;
	this->events_void_to_void_[object][interface].push_back(e);

	this->addIntrospectableEvent(object);
}

void GKDBusEvents::addEvent_VoidToString_Callback(const char* object, const char* interface, const char* eventName,
	std::vector<DBusMethodArgument> args, std::function<const std::string(void)> callback,
	GKDBusEventType eventType)
{
	GKDBusEvent_VoidToString_Callback e(eventName, args, callback, eventType);
	this->DBusObjects_[object] = true;
	this->DBusInterfaces_[interface] = true;
	this->events_void_to_string_[object][interface].push_back(e);

	this->addIntrospectableEvent(object);
}

void GKDBusEvents::addEvent_VoidToStringsArray_Callback(const char* object, const char* interface, const char* eventName,
	std::vector<DBusMethodArgument> args, std::function<const std::vector<std::string>(void)> callback,
	GKDBusEventType eventType)
{
	GKDBusEvent_VoidToStringsArray_Callback e(eventName, args, callback, eventType);
	this->DBusObjects_[object] = true;
	this->DBusInterfaces_[interface] = true;
	this->events_void_to_stringsarray_[object][interface].push_back(e);

	this->addIntrospectableEvent(object);
}

void GKDBusEvents::addEvent_StringToString_Callback(const char* object, const char* interface, const char* eventName,
	std::vector<DBusMethodArgument> args, std::function<const std::string(const std::string&)> callback,
	GKDBusEventType eventType)
{
	GKDBusEvent_StringToString_Callback e(eventName, args, callback, eventType);
	this->DBusObjects_[object] = true;
	this->DBusInterfaces_[interface] = true;
	this->events_string_to_string_[object][interface].push_back(e);

	this->addIntrospectableEvent(object);
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

/*
 * private
 */

void GKDBusEvents::addIntrospectableEvent(const char* object) {
	try {
		const auto & obj = this->events_string_to_string_.at(object);
		const auto & interf = obj.at("org.freedesktop.DBus.Introspectable");
	}
	catch (const std::out_of_range& oor) {
		LOG(DEBUG3) << "adding Introspectable interface : " << object;
		this->addEvent_StringToString_Callback(
			object, "org.freedesktop.DBus.Introspectable", "Introspect",
			{{"s", "xml_data", "out", "xml data representing DBus interfaces"}},
			std::bind(&GKDBusEvents::introspect, this, std::placeholders::_1));
	}
}

void GKDBusEvents::openXMLInterface(std::ostringstream & xml, bool& interface_opened, const std::string & interface) {
	if( ! interface_opened ) {
		xml << "  <interface name=\"" << interface << "\">\n";
		interface_opened = true;
	}
}

void GKDBusEvents::eventToXMLMethod(std::ostringstream & xml, const GKDBusEvent & DBusEvent) {
	if( DBusEvent.eventType == GKDBusEventType::GKDBUS_EVENT_METHOD ) {
		xml << "    <method name=\"" << DBusEvent.eventName << "\">\n";
		for(const auto & arg : DBusEvent.arguments) {
			xml << "      <!-- " << arg.comment << " -->\n";
			xml << "      <arg type=\"" << arg.type << "\" name=\"" << arg.name << "\" direction=\"" << arg.direction << "\" />\n";
		}
		xml << "    </method>\n";
	}
}

const std::string GKDBusEvents::introspect(const std::string & object_asked) {
	std::ostringstream xml;

	std::string asked_object_path = this->getNode(object_asked);

	LOG(DEBUG2) << "object path asked : " << asked_object_path;

	xml << "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n";
	xml << "		\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n";
	xml << "<node name=\"" << asked_object_path << "\">\n";

	for(const auto & interface_it : this->DBusInterfaces_) {
		const std::string & interface = interface_it.first;

		bool interface_opened = false;

		for(const auto & object_it : this->events_string_to_string_) {
			/* object must match */
			if(object_it.first != object_asked)
				continue;
			for(const auto & inter_it : object_it.second) {
				if( inter_it.first == interface ) {
					this->openXMLInterface(xml, interface_opened, interface);
					for(const auto & DBusEvent : inter_it.second) { /* vector of struct */
						this->eventToXMLMethod(xml, DBusEvent);
					}
				}
			}
		}

		for(const auto & object_it : this->events_void_to_string_) {
			/* object must match */
			if(object_it.first != object_asked)
				continue;
			for(const auto & inter_it : object_it.second) {
				if( inter_it.first == interface ) {
					this->openXMLInterface(xml, interface_opened, interface);
					for(const auto & DBusEvent : inter_it.second) { /* vector of struct */
						this->eventToXMLMethod(xml, DBusEvent);
					}
				}
			}
		}

		for(const auto & object_it : this->events_void_to_stringsarray_) {
			/* object must match */
			if(object_it.first != object_asked)
				continue;
			for(const auto & inter_it : object_it.second) {
				if( inter_it.first == interface ) {
					this->openXMLInterface(xml, interface_opened, interface);
					for(const auto & DBusEvent : inter_it.second) { /* vector of struct */
						this->eventToXMLMethod(xml, DBusEvent);
					}
				}
			}
		}

		for(const auto & object_it : this->events_string_to_bool_) {
			/* object must match */
			if(object_it.first != object_asked)
				continue;
			for(const auto & inter_it : object_it.second) {
				if( inter_it.first == interface ) {
					this->openXMLInterface(xml, interface_opened, interface);
					for(const auto & DBusEvent : inter_it.second) { /* vector of struct */
						this->eventToXMLMethod(xml, DBusEvent);
					}
				}
			}
		}

		for(const auto & object_it : this->events_twostrings_to_bool_) {
			/* object must match */
			if(object_it.first != object_asked)
				continue;
			for(const auto & inter_it : object_it.second) {
				if( inter_it.first == interface ) {
					this->openXMLInterface(xml, interface_opened, interface);
					for(const auto & DBusEvent : inter_it.second) { /* vector of struct */
						this->eventToXMLMethod(xml, DBusEvent);
					}
				}
			}
		}

		if( interface_opened )
			xml << "  </interface>\n";
	}

	xml << "</node>\n";
	return xml.str();
}

/* -- */

} // namespace GLogiK

