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

#include <stdexcept>
#include <sstream>
#include <functional>

#include "lib/utils/utils.h"

#include "GKDBusEvents.h"

namespace GLogiK
{

const std::string GKDBusEvents::rootNodeObject_("RootNode");

GKDBusEvents::GKDBusEvents(const std::string & rootnode) : root_node_(rootnode) {
	/* adding special event for root node introspection */
	const char* object = GKDBusEvents::rootNodeObject_.c_str();
#if DEBUGGING_ON
	LOG(DEBUG3) << "adding Introspectable interface : " << object;
#endif
	this->addStringToStringEvent(
		object, "org.freedesktop.DBus.Introspectable", "Introspect",
		{{"s", "xml_data", "out", "xml data representing DBus interfaces"}},
		std::bind(&GKDBusEvents::introspect, this, std::placeholders::_1),
		GKDBusEventType::GKDBUS_EVENT_METHOD, false);
}

GKDBusEvents::~GKDBusEvents() {
	for(const auto & object_path_pair : this->DBusEvents_) {
#if DEBUGGING_ON
		LOG(DEBUG1) << "object_path: " << object_path_pair.first;
#endif
		for(const auto & interface_pair : object_path_pair.second) {
#if DEBUGGING_ON
			LOG(DEBUG2) << "interface: " << interface_pair.first;
#endif
			for(auto & DBusEvent : interface_pair.second) { /* vector of pointers */
				delete DBusEvent;
			}
		}
	}
}

const std::string GKDBusEvents::getNode(const std::string & object) const {
	std::string node(this->root_node_);
	node += "/";
	node += object;
	return node;
}

const std::string & GKDBusEvents::getRootNode(void) const {
	return this->root_node_;
}

const std::string GKDBusEvents::introspectRootNode(void) {
	std::ostringstream xml;

	xml << "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n";
	xml << "		\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n";
	xml << "<node name=\"" << this->root_node_ << "\">\n";

	for(const auto & object_pair : this->DBusObjects_) {
		if(object_pair.first != GKDBusEvents::rootNodeObject_)
			xml << "  <node name=\"" << object_pair.first << "\"/>\n";
	}

	xml << "</node>\n";
	return xml.str();
}

/*
 * private
 */

void GKDBusEvents::addIntrospectableEvent(const char* object, const char* interface, GKDBusEvent* event) {
	if(event->introspectable) {
		try {
			const auto & obj = this->DBusEvents_.at(object);
			obj.at("org.freedesktop.DBus.Introspectable");
		}
		catch (const std::out_of_range& oor) {
#if DEBUGGING_ON
			LOG(DEBUG3) << "adding Introspectable interface : " << object;
#endif
			this->addStringToStringEvent(
				object, "org.freedesktop.DBus.Introspectable", "Introspect",
				{{"s", "xml_data", "out", "xml data representing DBus interfaces"}},
				std::bind(&GKDBusEvents::introspect, this, std::placeholders::_1),
				GKDBusEventType::GKDBUS_EVENT_METHOD, false);
		}
	}
	this->DBusObjects_[object] = true;
	this->DBusInterfaces_[interface] = true;
	this->DBusEvents_[object][interface].push_back(event);
}

void GKDBusEvents::openXMLInterface(
	std::ostringstream & xml,
	bool & interface_opened,
	const std::string & interface
) {
	if( ! interface_opened ) {
		xml << "  <interface name=\"" << interface << "\">\n";
		interface_opened = true;
	}
}

void GKDBusEvents::eventToXMLMethod(
	std::ostringstream & xml,
	const GKDBusEvent* DBusEvent
) {
	if( DBusEvent->eventType == GKDBusEventType::GKDBUS_EVENT_METHOD ) {
		xml << "    <method name=\"" << DBusEvent->eventName << "\">\n";
		for(const auto & arg : DBusEvent->arguments) {
			xml << "      <!-- " << arg.comment << " -->\n";
			xml << "      <arg type=\"" << arg.type << "\" name=\"" << arg.name << "\" direction=\"" << arg.direction << "\" />\n";
		}
		xml << "    </method>\n";
	}
}

const std::string GKDBusEvents::introspect(const std::string & asked_object_path) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "object path asked : " << asked_object_path;
#endif

	if( asked_object_path == this->root_node_ )
		return this->introspectRootNode();

	std::ostringstream xml;

	xml << "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n";
	xml << "		\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n";
	xml << "<node name=\"" << asked_object_path << "\">\n";

	for(const auto & interface_it : this->DBusInterfaces_) {
		const std::string & interface = interface_it.first;

		bool interface_opened = false;

		for(const auto & object_path_pair : this->DBusEvents_) {
			/* object path must match */
			if( this->getNode(object_path_pair.first) != asked_object_path )
				continue;
			for(const auto & interface_pair : object_path_pair.second) {
				if( interface_pair.first == interface ) {
					this->openXMLInterface(xml, interface_opened, interface);
					for(const auto & DBusEvent : interface_pair.second) { /* vector of pointers */
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

