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

GKDBusEvents::~GKDBusEvents()
{
}

const std::string & GKDBusEvents::getRootNode(void) const
{
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

/*
void GKDBusEvents::removeMethod(
	const BusConnection eventBus,
	const char* eventObject,
	const char* eventInterface,
	const char* eventName)
{
	this->removeEvent(eventBus, eventObject, eventInterface, eventName);
}
*/

void GKDBusEvents::removeMethodsInterface(
	const BusConnection eventBus,
	const char* eventObject,
	const char* eventInterface) noexcept
{
	this->removeInterface(eventBus, nullptr, eventObject, eventInterface);
}

void GKDBusEvents::removeSignalsInterface(
	const BusConnection eventBus,
	const char* eventSender,
	const char* eventObject,
	const char* eventInterface) noexcept
{
	this->removeInterface(eventBus, eventSender, eventObject, eventInterface);
}

void GKDBusEvents::clearDBusEvents(void) noexcept
{
	GK_LOG_FUNC

	for(const auto & busPair : _DBusEvents) {
		GKLog2(trace, "current bus : ", toUInt(toEnumType(busPair.first)))
		for(const auto & objectPair : busPair.second) {
			GKLog2(trace, "object : ", objectPair.first)
			for(const auto & interfacePair : objectPair.second) {
				GKLog2(trace, "interface : ", interfacePair.first)
				for(auto & DBusEvent : interfacePair.second) { /* vector of pointers */
					delete DBusEvent;
				}
			}
		}
	}

	_DBusEvents.clear();
}

/*
 * private
 */

void GKDBusEvents::removeInterface(
	const BusConnection eventBus,
	const char* eventSender,
	const char* eventObject,
	const char* eventInterface) noexcept
{
	GK_LOG_FUNC

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
		GKLog4(trace, "removing interface : ", eventInterface, "from bus : ", toUInt(toEnumType(eventBus)))

		auto & objectMap = _DBusEvents[eventBus][eventObject];
		for(auto & DBusEvent : objectMap[eventInterface]) { /* vector of pointers */
			// if event is a signal, build and remove signal rule match
			if(DBusEvent->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL) {
				this->removeSignalRuleMatch(eventBus, eventSender, eventInterface, DBusEvent->eventName.c_str());
			}

			delete DBusEvent; DBusEvent = nullptr;
		}
		objectMap[eventInterface].clear();
		objectMap.erase(eventInterface);

		if( objectMap.empty() ) {
			GKLog2(trace, "removing empty object : ", eventObject)
			_DBusEvents[eventBus].erase(eventObject);
		}
		else if( (objectMap.size() == 1) and
			(objectMap.count("org.freedesktop.DBus.Introspectable") == 1) ) {
			this->removeInterface(eventBus, nullptr, eventObject, "org.freedesktop.DBus.Introspectable");
		}
	}
	else {
		LOG(warning) << "Interface not found. bus: " << toUInt(toEnumType(eventBus))
			<< " - obj: " << eventObject
			<< " - int: " << eventInterface;
	}
}

const std::string GKDBusEvents::introspectRootNode(void)
{
	GK_LOG_FUNC

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
		LOG(warning) << "can't get current bus container";
	}

	xml << "</node>\n";
	return xml.str();
}

/*
void GKDBusEvents::removeEvent(
	const BusConnection eventBus,
	const char* eventObject,
	const char* eventInterface,
	const char* eventName)
{
	GK_LOG_FUNC

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
							// TODO GKDebug
							LOG(trace) << "searched event found. bus: "
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

		// TODO fix eventSender and check
		auto & DBusEvent = vec[index];
		if(DBusEvent->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL) {
			this->removeSignalRuleMatch(eventBus, eventSender, eventInterface, DBusEvent->eventName.c_str());
		}

		delete DBusEvent; DBusEvent = nullptr;
		vec.erase(vec.begin() + index);
	}
	catch ( const GLogiKExcept & e ) {
		LOG(warning) << e.what()
			<< ". bus: " << toUInt(toEnumType(eventBus))
			<< " - obj: " << eventObject
			<< " - int: " << eventInterface
			<< " - name: " << eventName;
	}
}
*/

void GKDBusEvents::addEvent(
	const BusConnection eventBus,
	const char* eventSender,
	const char* eventObject,
	const char* eventInterface,
	GKDBusEvent* event)
{
	GK_LOG_FUNC

	if(event->introspectable) {
		try {
			const auto & bus = _DBusEvents.at(eventBus);
			const auto & obj = bus.at(eventObject);
			obj.at("org.freedesktop.DBus.Introspectable");
		}
		catch (const std::out_of_range& oor) {
			GKLog2(trace, "adding Introspectable object : ", eventObject)
			this->Callback<SIGs2s>::exposeEvent(
				eventBus, nullptr, eventObject, "org.freedesktop.DBus.Introspectable", "Introspect",
				{{"s", "xml_data", "out", "xml data representing DBus interfaces"}},
				std::bind(&GKDBusEvents::introspect, this, std::placeholders::_1),
				GKDBusEventType::GKDBUS_EVENT_METHOD, false);
		}
	}

	if( event->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL ) {
		this->addSignalRuleMatch(eventBus, eventSender, eventInterface, event->eventName.c_str());
	}

	_DBusInterfaces.insert(eventInterface);
	_DBusEvents[eventBus][eventObject][eventInterface].push_back(event);
}

void GKDBusEvents::openXMLInterface(
	std::ostringstream & xml,
	bool & interfaceOpened,
	const std::string & interface)
{
	if( ! interfaceOpened ) {
		xml << "  <interface name=\"" << interface << "\">\n";
		interfaceOpened = true;
	}
}

void GKDBusEvents::eventToXMLMethod(
	std::ostringstream & xml,
	const GKDBusEvent* DBusEvent)
{
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

const std::string GKDBusEvents::introspect(const std::string & askedObjectPath)
{
	GK_LOG_FUNC

	GKLog2(trace, "object path asked : ", askedObjectPath)

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
		LOG(warning) << "can't get current bus container";
	}

	xml << "</node>\n";

	return xml.str();
}

const std::string GKDBusEvents::buildSignalRuleMatch(
	const char* sender,
	const char* interface,
	const char* eventName) noexcept
{
	std::string rule = "type='signal',sender='";
	rule += sender;
	rule += "',interface='";
	rule += interface;
	rule += "',member='";
	rule += eventName;
	rule += "'";
	return rule;
}

void GKDBusEvents::addSignalRuleMatch(
	const BusConnection eventBus,
	const char* sender,
	const char* interface,
	const char* eventName) noexcept
{
	GK_LOG_FUNC

	DBusConnection* connection = nullptr;
	try {
		connection = this->getDBusConnection(eventBus);
	}
	catch ( const GLogiKExcept & e ) {
		LOG(warning) << e.what();
		return;
	}

	const std::string rule = this->buildSignalRuleMatch(sender, interface, eventName);

	dbus_bus_add_match(connection, rule.c_str(), nullptr);
	dbus_connection_flush(connection);

	GKLog2(trace, "added DBus signal match rule : ", eventName)
}

void GKDBusEvents::removeSignalRuleMatch(
	const BusConnection eventBus,
	const char* sender,
	const char* interface,
	const char* eventName) noexcept
{
	GK_LOG_FUNC

	DBusConnection* connection = nullptr;
	try {
		connection = this->getDBusConnection(eventBus);
	}
	catch ( const GLogiKExcept & e ) {
		LOG(warning) << e.what();
		return;
	}

	const std::string rule = this->buildSignalRuleMatch(sender, interface, eventName);

	dbus_bus_remove_match(connection, rule.c_str(), nullptr);
	dbus_connection_flush(connection);

	GKLog2(trace, "removed DBus signal match rule : ", eventName)
}

/* -- */

} // namespace NSGKDBus

