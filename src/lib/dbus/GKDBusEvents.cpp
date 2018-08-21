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

namespace NSGKDBus
{

using namespace NSGKUtils;

const std::string GKDBusEvents::_rootNodeObject("RootNode");
thread_local BusConnection GKDBusEvents::currentBus(BusConnection::GKDBUS_SYSTEM);

GKDBusEvents::GKDBusEvents(const std::string & rootNode) : _rootNode(rootNode) {
	/* adding special event for root node introspection */
	// FIXME same event for GKDBUS_SESSION
	const char* object = GKDBusEvents::_rootNodeObject.c_str();
	const BusConnection systemBus(BusConnection::GKDBUS_SYSTEM);
#if DEBUGGING_ON
	LOG(DEBUG3) << "adding Introspectable interface : " << object;
#endif
	this->EventGKDBusCallback<StringToString>::exposeEvent(
		systemBus, object, "org.freedesktop.DBus.Introspectable", "Introspect",
		{{"s", "xml_data", "out", "xml data representing DBus interfaces"}},
		std::bind(&GKDBusEvents::introspect, this, std::placeholders::_1),
		GKDBusEventType::GKDBUS_EVENT_METHOD, false);
}

GKDBusEvents::~GKDBusEvents() {
	for(const auto & busPair : _DBusEvents) {
#if DEBUGGING_ON
		LOG(DEBUG1) << "current bus: " << to_uint(to_type(busPair.first));
#endif
		for(const auto & objectPathPair : busPair.second) {
#if DEBUGGING_ON
			LOG(DEBUG2) << "object_path: " << objectPathPair.first;
#endif
			for(const auto & interfacePair : objectPathPair.second) {
#if DEBUGGING_ON
				LOG(DEBUG3) << "interface: " << interfacePair.first;
#endif
				for(auto & DBusEvent : interfacePair.second) { /* vector of pointers */
					delete DBusEvent;
				}
			}
		}
	}
}

const std::string GKDBusEvents::getNode(const std::string & object) const {
	std::string node(_rootNode);
	node += "/";
	node += object;
	return node;
}

const std::string & GKDBusEvents::getRootNode(void) const {
	return _rootNode;
}

const std::string GKDBusEvents::introspectRootNode(void) {
	std::ostringstream xml;

	xml << "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n";
	xml << "		\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n";
	xml << "<node name=\"" << _rootNode << "\">\n";

	try {
		const auto & bus = _DBusEvents.at(GKDBusEvents::currentBus);

		for(const auto & objectPathPair : bus) {
			if(objectPathPair.first != GKDBusEvents::_rootNodeObject)
				xml << "  <node name=\"" << objectPathPair.first << "\"/>\n";
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(WARNING) << "can't get current bus container";
	}

	xml << "</node>\n";
	return xml.str();
}

/*
 * private
 */

void GKDBusEvents::addIntrospectableEvent(
	const BusConnection eventBus,
	const char* eventObject,
	const char* eventInterface,
	GKDBusEvent* event)
{
	if(event->introspectable) {
		try {
			const auto & bus = _DBusEvents.at(eventBus);
			const auto & obj = bus.at(eventObject);
			obj.at("org.freedesktop.DBus.Introspectable");
		}
		catch (const std::out_of_range& oor) {
#if DEBUGGING_ON
			LOG(DEBUG3) << "adding Introspectable interface : " << eventObject;
#endif
			this->EventGKDBusCallback<StringToString>::exposeEvent(
				eventBus, eventObject, "org.freedesktop.DBus.Introspectable", "Introspect",
				{{"s", "xml_data", "out", "xml data representing DBus interfaces"}},
				std::bind(&GKDBusEvents::introspect, this, std::placeholders::_1),
				GKDBusEventType::GKDBUS_EVENT_METHOD, false);
		}
	}
	_DBusInterfaces.insert(eventInterface);
	_DBusEvents[eventBus][eventObject][eventInterface].push_back(event);
}

void GKDBusEvents::openXMLInterface(
	std::ostringstream & xml,
	bool & interfaceOpened,
	const std::string & interface
) {
	if( ! interfaceOpened ) {
		xml << "  <interface name=\"" << interface << "\">\n";
		interfaceOpened = true;
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

const std::string GKDBusEvents::introspect(const std::string & askedObjectPath) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "object path asked : " << askedObjectPath;
#endif

	if( askedObjectPath == _rootNode )
		return this->introspectRootNode();

	std::ostringstream xml;

	xml << "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n";
	xml << "		\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n";
	xml << "<node name=\"" << askedObjectPath << "\">\n";

	try {
		const auto & bus = _DBusEvents.at(GKDBusEvents::currentBus);

		for(const auto & interface : _DBusInterfaces) {

			bool interfaceOpened = false;

			for(const auto & objectPathPair : bus) {
				/* object path must match */
				if( this->getNode(objectPathPair.first) != askedObjectPath )
					continue;
				for(const auto & interfacePair : objectPathPair.second) {
					if( interfacePair.first == interface ) {
						this->openXMLInterface(xml, interfaceOpened, interface);
						for(const auto & DBusEvent : interfacePair.second) { /* vector of pointers */
							this->eventToXMLMethod(xml, DBusEvent);
						}
					}
				}
			}

			if( interfaceOpened )
				xml << "  </interface>\n";
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(WARNING) << "can't get current bus container";
	}

	xml << "</node>\n";

	return xml.str();
}

/* -- */

} // namespace NSGKDBus

