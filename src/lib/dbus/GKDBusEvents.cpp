/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2020  Fabrice Delliaux <netbox253@gmail.com>
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

#include "lib/utils/utils.hpp"

#include "GKDBusEvents.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

const std::string GKDBusEvents::_rootNodeObject("RootNode");
thread_local BusConnection GKDBusEvents::currentBus(BusConnection::GKDBUS_SYSTEM);

GKDBusEvents::GKDBusEvents(
	const std::string & rootNode,
	const std::string & rootNodePath)
		:	_rootNode(rootNode),
			_rootNodePath(rootNodePath)
{
}

GKDBusEvents::~GKDBusEvents() {
	for(const auto & busPair : _DBusEvents) {
#if DEBUGGING_ON
		LOG(DEBUG1) << "current bus: " << toUInt(toEnumType(busPair.first));
#endif
		for(const auto & objectPair : busPair.second) {
#if DEBUGGING_ON
			LOG(DEBUG2) << "object: " << objectPair.first;
#endif
			for(const auto & interfacePair : objectPair.second) {
#if DEBUGGING_ON
				LOG(DEBUG3) << "interface: " << interfacePair.first;
#endif
				for(auto & DBusEvent : interfacePair.second) { /* vector of pointers */
					delete DBusEvent;
				}
			}
		}
	}

	_DBusEvents.clear();
}

const std::string & GKDBusEvents::getRootNode(void) const {
	return _rootNode;
}

void GKDBusEvents::declareIntrospectableSignal(
	const BusConnection bus,
	const char* object,
	const char* interface,
	const char* name,
	const std::vector<DBusMethodArgument> & args)
{
	GKDBusIntrospectableSignal signal(name, args);
	_DBusIntrospectableSignals[bus][object][interface].push_back(signal);
}

void GKDBusEvents::removeMethod(
	const BusConnection eventBus,
	const char* eventObject,
	const char* eventInterface,
	const char* eventName)
{
	this->removeEvent(eventBus, eventObject, eventInterface, eventName);
}

void GKDBusEvents::removeInterface(
	const BusConnection eventBus,
	const char* eventObject,
	const char* eventInterface)
{
	auto find_interface = [this, &eventBus, &eventObject, &eventInterface] () -> const bool {
		if(_DBusEvents.count(eventBus) == 1) {
			if(_DBusEvents[eventBus].count(eventObject) == 1) {
				if(_DBusEvents[eventBus][eventObject].count(eventInterface) == 1)
					return true;
			}
		}
		return false;
	};

	if( find_interface() ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "removing interface: " << eventInterface
					<< " from bus : " << toUInt(toEnumType(eventBus));
#endif
		auto & objectMap = _DBusEvents[eventBus][eventObject];
		for(auto & DBusEvent : objectMap[eventInterface]) { /* vector of pointers */
			delete DBusEvent; DBusEvent = nullptr;
		}
		objectMap[eventInterface].clear();
		objectMap.erase(eventInterface);

		if( objectMap.empty() ) {
#if DEBUGGING_ON
			LOG(DEBUG2) << "removing empty object: " << eventObject;
#endif
			_DBusEvents[eventBus].erase(eventObject);
		}
		else if( (objectMap.size() == 1) and
			(objectMap.count("org.freedesktop.DBus.Introspectable") == 1) ) {
			this->removeInterface(eventBus, eventObject, "org.freedesktop.DBus.Introspectable");
		}
	}
	else {
		LOG(WARNING) << "Interface not found. bus: " << toUInt(toEnumType(eventBus))
			<< " - obj: " << eventObject
			<< " - int: " << eventInterface;
	}
}

/*
 * private
 */

const std::string GKDBusEvents::introspectRootNode(void) {
	std::ostringstream xml;

	xml << "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n";
	xml << "		\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n";
	xml << "<node name=\"" << _rootNodePath << "\">\n";

	try {
		const auto & bus = _DBusEvents.at(GKDBusEvents::currentBus);

		for(const auto & objectPair : bus) {
			if(objectPair.first != GKDBusEvents::_rootNodeObject)
				xml << "  <node name=\"" << objectPair.first << "\"/>\n";
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(WARNING) << "can't get current bus container";
	}

	xml << "</node>\n";
	return xml.str();
}

void GKDBusEvents::removeEvent(
	const BusConnection eventBus,
	const char* eventObject,
	const char* eventInterface,
	const char* eventName)
{
	auto get_index = [this, &eventBus, &eventObject, &eventInterface, &eventName] () -> const std::size_t {
		if(_DBusEvents.count(eventBus) == 1) {
			if(_DBusEvents[eventBus].count(eventObject) == 1) {
				if(_DBusEvents[eventBus][eventObject].count(eventInterface) == 1) {
					// vector of pointers
					auto & vec = _DBusEvents[eventBus][eventObject][eventInterface];

					for(auto it = vec.cbegin(); it != vec.cend(); ++it) {
						if( (*it)->eventName == eventName ) {
							const std::size_t index = (it - vec.cbegin());
#if DEBUGGING_ON
							LOG(DEBUG2) << "searched event found. bus: "
								<< toUInt(toEnumType(eventBus))
								<< " - obj: " << eventObject
								<< " - int: " << eventInterface
								<< " - ind: " << index;
#endif
							return index;
						}
					}

				}
			}
		}

		throw GLogiKExcept("event not found");
	};

	try {
		const std::size_t index = get_index();
		auto & vec = _DBusEvents[eventBus][eventObject][eventInterface];
		delete vec[index]; vec[index] = nullptr;
		vec.erase(vec.begin() + index);
	}
	catch ( const GLogiKExcept & e ) {
		LOG(WARNING) << e.what()
			<< ". bus: " << toUInt(toEnumType(eventBus))
			<< " - obj: " << eventObject
			<< " - int: " << eventInterface
			<< " - name: " << eventName;
	}

}

void GKDBusEvents::addEvent(
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
			LOG(DEBUG3) << "adding Introspectable object : " << eventObject;
#endif
			this->EventGKDBusCallback<StringToString>::exposeEvent(
				eventBus, nullptr, eventObject, "org.freedesktop.DBus.Introspectable", "Introspect",
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
			xml << "      <arg type=\"" << arg.type << "\" ";
			if( ! arg.name.empty() ) /* name attribute on arguments is optional */
				xml << "name=\"" << arg.name << "\" ";
			xml << "direction=\"" << arg.direction << "\" />\n";
		}
		xml << "    </method>\n";
	}
}

const std::string GKDBusEvents::introspect(const std::string & askedObjectPath) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "object path asked : " << askedObjectPath;
#endif

	if( askedObjectPath == _rootNodePath )
		return this->introspectRootNode();

	std::ostringstream xml;

	xml << "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n";
	xml << "		\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n";
	xml << "<node name=\"" << askedObjectPath << "\">\n";

	try {
		for(const auto & interface : _DBusInterfaces) {

			bool interfaceOpened = false;

			for(const auto & objectPair : _DBusEvents.at(GKDBusEvents::currentBus)) {
				std::string objectPath(_rootNodePath);
				objectPath += "/"; objectPath += objectPair.first;
				/* object path must match */
				if( objectPath != askedObjectPath )
					continue;
				for(const auto & interfacePair : objectPair.second) {
					if( interfacePair.first == interface ) {
						this->openXMLInterface(xml, interfaceOpened, interface);
						for(const auto & DBusEvent : interfacePair.second) { /* vector of pointers */
							this->eventToXMLMethod(xml, DBusEvent);
						}
					}
				}
			}

			for(const auto & objectPair : _DBusIntrospectableSignals.at(GKDBusEvents::currentBus)) {
				std::string objectPath(_rootNodePath);
				objectPath += "/"; objectPath += objectPair.first;
				/* object path must match */
				if( objectPath != askedObjectPath )
					continue;
				for(const auto & interfacePair : objectPair.second) {
					if( interfacePair.first == interface ) {
						this->openXMLInterface(xml, interfaceOpened, interface);
						for(const auto & signal : interfacePair.second) { /* vector of  objects */
							xml << "    <signal name=\"" << signal.name << "\">\n";
							for(const auto & arg : signal.arguments) {
								xml << "      <!-- " << arg.comment << " -->\n";
								xml << "      <arg type=\"" << arg.type << "\" ";
								if( ! arg.name.empty() ) /* name attribute on arguments is optional */
									xml << "name=\"" << arg.name << "\" ";
								//xml << "direction=\"out\" />\n";
								xml << "/>\n";
							}
							xml << "    </signal>\n";
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

