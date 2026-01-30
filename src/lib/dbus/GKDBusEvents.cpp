/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2025  Fabrice Delliaux <netbox253@gmail.com>
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

#include <set>
#include <stdexcept>
#include <sstream>
#include <functional>

#include "lib/utils/utils.hpp"

#include "GKDBusEvents.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

thread_local BusConnection GKDBusEvents::currentBus(BusConnection::GKDBUS_SYSTEM);

GKDBusEvents::GKDBusEvents(const std::string & rootNodePath)
		: _rootNodePath(rootNodePath)
{
}

GKDBusEvents::~GKDBusEvents()
{
}

void GKDBusEvents::declareIntrospectableSignal(
	const BusConnection eventBus,
	const char* eventObjectPath,
	const char* eventInterface,
	const char* eventName,
	const std::vector<DBusMethodArgument> & args)
{
	GKDBusIntrospectableSignal signal(eventName, args);
	_DBusIntrospectableSignals[eventBus][eventObjectPath][eventInterface].push_back(signal);
	this->exposeIntrospectMethod(eventBus, eventObjectPath);
}

/*
void GKDBusEvents::removeMethod(
	const BusConnection eventBus,
	const char* eventObjectPath,
	const char* eventInterface,
	const char* eventName)
{
	this->removeEvent(eventBus, eventObjectPath, eventInterface, eventName);
}
*/

void GKDBusEvents::removeMethodsInterface(
	const BusConnection eventBus,
	const char* eventObjectPath,
	const char* eventInterface) noexcept
{
	this->removeInterface(eventBus, nullptr, eventObjectPath, eventInterface);
}

void GKDBusEvents::removeSignalsInterface(
	const BusConnection eventBus,
	const char* eventSender,
	const char* eventObjectPath,
	const char* eventInterface) noexcept
{
	this->removeInterface(eventBus, eventSender, eventObjectPath, eventInterface);
}

void GKDBusEvents::clearDBusEvents(void) noexcept
{
	GK_LOG_FUNC

	for(const auto & [bus, opMap] : _DBusEvents) /* objectPath map */
	{
		GKLog2(trace, "current bus : ", toUInt(toEnumType(bus)))
		for(const auto & [objectPath, interMap] : opMap) /* interface map */
		{
			GKLog2(trace, "object path : ", objectPath)
			for(const auto & [interface, pVec ] : interMap) /* vector of pointers */
			{
				GKLog2(trace, "interface : ", interface)
				for(auto & DBusEvent : pVec)
					delete DBusEvent;
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
	const char* eventObjectPath,
	const char* eventInterface) noexcept
{
	GK_LOG_FUNC

	auto find_interface = [this, &eventBus, &eventObjectPath, &eventInterface] () -> const bool
	{
		if(_DBusEvents.count(eventBus) == 1)
			if(_DBusEvents[eventBus].count(eventObjectPath) == 1)
				if(_DBusEvents[eventBus][eventObjectPath].count(eventInterface) == 1)
					return true;
		return false;
	};

	if( find_interface() )
	{
		GKLog4(trace,
			"removing interface : ", eventInterface,
			"from bus : ", toUInt(toEnumType(eventBus))
		)

		auto & objectPathMap = _DBusEvents[eventBus][eventObjectPath];
		for(auto & DBusEvent : objectPathMap[eventInterface]) /* vector of pointers */
		{
			// if event is a signal, build and remove signal rule match
			if(DBusEvent->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
				this->removeSignalRuleMatch(
					eventBus,
					eventSender,
					eventInterface,
					DBusEvent->eventName.c_str()
				);

			delete DBusEvent; DBusEvent = nullptr;
		}
		objectPathMap[eventInterface].clear();
		objectPathMap.erase(eventInterface);

		if( objectPathMap.empty() )
		{
			GKLog2(trace, "removing empty object path : ", eventObjectPath)
			_DBusEvents[eventBus].erase(eventObjectPath);
		}
		else if( (objectPathMap.size() == 1) and
			(objectPathMap.count(_FREEDESKTOP_DBUS_INTROSPECTABLE_STANDARD_INTERFACE) == 1) )
		{
			this->removeInterface(
				eventBus,
				nullptr,
				eventObjectPath,
				_FREEDESKTOP_DBUS_INTROSPECTABLE_STANDARD_INTERFACE
			);
		}
	}
	else
	{
		LOG(warning) << "Interface not found. bus: " << toUInt(toEnumType(eventBus))
			<< " - obj path: " << eventObjectPath
			<< " - int: " << eventInterface;
	}
}

const std::string GKDBusEvents::getObjectFromObjectPath(const std::string & objectPath)
{
	std::string object;
	std::istringstream path(objectPath);
	/* get last part of object path */
	while(std::getline(path, object, '/')) {}
#if 0 && DEBUGGING_ON
	LOG(trace) << "object path: " << objectPath;
	LOG(trace) << "     object: " << object;
#endif
	return object;
}

/*
void GKDBusEvents::removeEvent(
	const BusConnection eventBus,
	const char* eventObjectPath,
	const char* eventInterface,
	const char* eventName)
{
	GK_LOG_FUNC

	auto get_index = [this, &eventBus, &eventObjectPath, &eventInterface, &eventName] ()
		-> const std::size_t
	{
		if(_DBusEvents.count(eventBus) == 1)
		{
			if(_DBusEvents[eventBus].count(eventObjectPath) == 1)
			{
				if(_DBusEvents[eventBus][eventObjectPath].count(eventInterface) == 1)
				{
					// vector of pointers
					auto & vec = _DBusEvents[eventBus][eventObjectPath][eventInterface];

					for(auto it = vec.cbegin(); it != vec.cend(); ++it)
					{
						if( (*it)->eventName == eventName )
						{
							const std::size_t index = (it - vec.cbegin());
#if DEBUGGING_ON
							// TODO GKDebug
							LOG(trace) << "searched event found. bus: "
								<< toUInt(toEnumType(eventBus))
								<< " - obj path: " << eventObjectPath
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

	try
	{
		const std::size_t index = get_index();
		auto & vec = _DBusEvents[eventBus][eventObjectPath][eventInterface];

		// TODO fix eventSender and check
		auto & DBusEvent = vec[index];
		if(DBusEvent->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
			this->removeSignalRuleMatch(
				eventBus,
				eventSender,
				eventInterface,
				DBusEvent->eventName.c_str()
			);

		delete DBusEvent; DBusEvent = nullptr;
		vec.erase(vec.begin() + index);
	}
	catch ( const GLogiKExcept & e )
	{
		LOG(warning) << e.what()
			<< ". bus: " << toUInt(toEnumType(eventBus))
			<< " - obj path: " << eventObjectPath
			<< " - int: " << eventInterface
			<< " - name: " << eventName;
	}
}
*/

void GKDBusEvents::exposeIntrospectMethod(
	const BusConnection eventBus,
	const char* eventObjectPath)
{
	try
	{
		const auto & bus = _DBusEvents.at(eventBus);
		const auto & objpath = bus.at(eventObjectPath);
		objpath.at(_FREEDESKTOP_DBUS_INTROSPECTABLE_STANDARD_INTERFACE);
	}
	catch (const std::out_of_range& oor)
	{
		GKLog2(trace, "adding Introspectable object path: ", eventObjectPath)

		this->Callback<SIGs2s>::exposeEvent(
			eventBus,			/* bus */
			nullptr,			/* sender (used only if
								   eventType == GKDBUS_EVENT_SIGNAL below,
								   unused here --> nullptr) */
			eventObjectPath,	/* event object path */
			_FREEDESKTOP_DBUS_INTROSPECTABLE_STANDARD_INTERFACE,	/* event interface */
			"Introspect",		/* event name */
			{	{	"s",
					"xml_data",
					"out",
					"xml data representing DBus interfaces"
				} }, 								/* event arguments */
			std::bind(
				&GKDBusEvents::introspect,
				this,
				std::placeholders::_1),				/* callback method */
			GKDBusEventType::GKDBUS_EVENT_METHOD,	/* event type (method|signal) */
			false									/* introspectability */
		);
	}
}

void GKDBusEvents::addEvent(
	const BusConnection eventBus,
	const char* eventSender,
	const char* eventObjectPath,
	const char* eventInterface,
	GKDBusEvent* event)
{
	GK_LOG_FUNC

	if(event->introspectable)
		this->exposeIntrospectMethod(eventBus, eventObjectPath);

	if( event->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL )
		this->addSignalRuleMatch(eventBus, eventSender, eventInterface, event->eventName.c_str());

	_DBusInterfaces.insert(eventInterface);
	_DBusEvents[eventBus][eventObjectPath][eventInterface].push_back(event);
}

void GKDBusEvents::openXMLInterface(
	std::ostringstream & xml,
	bool & interfaceOpened,
	const std::string & interface)
{
	if( ! interfaceOpened )
	{
		xml << "  <interface name=\"" << interface << "\">\n";
		interfaceOpened = true;
	}
}

void GKDBusEvents::closeXMLInterface(
	std::ostringstream & xml,
	bool & interfaceOpened)
{
	if( interfaceOpened )
		xml << "  </interface>\n";
}

void GKDBusEvents::eventToXMLMethod(
	std::ostringstream & xml,
	const GKDBusEvent* DBusEvent)
{
	if( DBusEvent->eventType == GKDBusEventType::GKDBUS_EVENT_METHOD )
	{
		xml << "    <method name=\"" << DBusEvent->eventName << "\">\n";
		for(const auto & arg : DBusEvent->arguments)
		{
			xml << "      <!-- " << arg.comment << " -->\n";
			xml << "      <arg type=\"" << arg.type << "\" ";
			if( ! arg.name.empty() ) /* name attribute on arguments is optional */
				xml << "name=\"" << arg.name << "\" ";
			xml << "direction=\"" << arg.direction << "\" />\n";
		}
		xml << "    </method>\n";
	}
}

void GKDBusEvents::signalToXMLSignal(
	std::ostringstream & xml,
	const GKDBusIntrospectableSignal & signal)
{
	xml << "    <signal name=\"" << signal.name << "\">\n";
	for(const auto & arg : signal.arguments)
	{
		xml << "      <!-- " << arg.comment << " -->\n";
		xml << "      <arg type=\"" << arg.type << "\" ";
		if( ! arg.name.empty() ) /* name attribute on arguments is optional */
			xml << "name=\"" << arg.name << "\" ";
			//xml << "direction=\"out\" />\n";
		xml << "/>\n";
	}
	xml << "    </signal>\n";
}

const std::string GKDBusEvents::introspect(const std::string & askedObjectPath)
{
	GK_LOG_FUNC

	GKLog2(trace, "asked object path: ", askedObjectPath)

	std::set<std::string> xmlNodes;
	std::ostringstream xml;
	std::string::size_type n;

	xml << "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n";
	xml << "		\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n";
	xml << "<node name=\"" << askedObjectPath << "\">\n";

	auto xml_stream = [&xml] (void) -> const std::string
	{
		xml << "</node>\n";
		return xml.str();
	};

	// check that asked object path is valid
	// https://dbus.freedesktop.org/doc/dbus-specification.html#message-protocol-marshaling-object-path
	{
		n = askedObjectPath.find("//");
		const char & last = askedObjectPath.back();
		if(( (n != std::string::npos) or (last == '/') ) and (askedObjectPath != "/"))
		{
			LOG(warning) << "asked object path is not valid: '" << askedObjectPath << "'";
			return xml_stream();
		}
	}

	for(const auto & DBusInterface : _DBusInterfaces)
	{
		if(DBusInterface == _FREEDESKTOP_DBUS_INTROSPECTABLE_STANDARD_INTERFACE)
			continue;

#if DEBUG_GKDBUS
		GKLog2(trace, "DBus Interface: ", DBusInterface)
#endif

		bool interfaceOpened = false;
		bool aaa = false;

		try
		{ // <<<
			const auto & opMap = _DBusEvents.at(GKDBusEvents::currentBus); // objectPath map

			for(const auto & [objectPath, interMap] : opMap) // interface map
			{ // <<<
				std::string op(objectPath);

				if(askedObjectPath != objectPath)
				{ // if this does not match ...
					n = objectPath.find(askedObjectPath); // ... and is not a substring ...
					if(n == std::string::npos)
						continue; // ... then jumps to next loop iteration ...

					op.erase(n, askedObjectPath.size());
				}

				for(const auto & [interface, pVec] : interMap) // vector of pointers
				{
					if(DBusInterface != interface)
						continue;

					//GKLog2(trace, "	object path: ", objectPath)
					if(askedObjectPath != objectPath)
					{ // <<< if this does not match (again) ...
						bool skip_op = false; // skip current object path

						if(askedObjectPath != "/")
						{
							try
							{
								if(op.at(0) == '/')
									op.erase(0, 1); // erasing leading '/'
								else
									skip_op = true;
							}
							catch (const std::out_of_range& oor)
							{
								LOG(warning) << "logical error 1, wrong object path ?";
								skip_op = true;
							}
						}

						if( ! skip_op )
						{
							n = op.find('/'); // trying to find next '/'
							if(n != std::string::npos)
								op = op.substr(0, n);

							if(xmlNodes.find(op) == xmlNodes.end())
							{
#if DEBUG_GKDBUS
								GKLog6(trace,
										"	DBusInterface: ", DBusInterface,
										"objectPath: ", objectPath, "op: ", op
								)
#endif

								skip_op = true;
								for(const auto & DBusEvent : pVec)
								{ // we want to find at least one method on this interface
									if(DBusEvent->eventType == GKDBusEventType::GKDBUS_EVENT_METHOD)
									{
										skip_op = false;
										break;
									}
								}

								if( ! skip_op )
								{
									auto result = xmlNodes.insert(op);
									if( ! result.second )
									{
										LOG(warning) << "node insertion failure: " << op;
									}
									else
									{
										xml << "	<node name=\"" << op << "\"/>\n";
#if DEBUG_GKDBUS
										GKLog2(trace, "		node appended: ", op)
#endif
									}
								}
#if DEBUG_GKDBUS
								else
								{
									GKLog2(trace, "		node skipped (no method found): ", op)
								}
#endif
							}
#if DEBUG_GKDBUS
							else
							{
								GKLog2(trace, "		node already inserted: ", op)
							}
#endif
						}
					} // >>>
					else
					{
						aaa = true;

						this->openXMLInterface(xml, interfaceOpened, DBusInterface);
						for(const auto & DBusEvent : pVec)
							this->eventToXMLMethod(xml, DBusEvent);
					}
				}
			} // >>> for
		} // >>>
		catch (const std::out_of_range& oor)
		{
			GKLog2(trace,
				"can't iterate over DBusEvents. No bus container: ",
				toUInt(toEnumType(GKDBusEvents::currentBus))
			)
		}

		if( aaa )
		{
			try
			{
				const auto & opMap =
					_DBusIntrospectableSignals.at(GKDBusEvents::currentBus); // objectPath map
				for(const auto & [objectPath, interMap] : opMap) // interface map
				{
					/* object path must match */
					if(askedObjectPath != objectPath)
						continue;
					for(const auto & [interface, oVec] : interMap) // vector of objects
					{
						if(DBusInterface == interface)
						{
							this->openXMLInterface(xml, interfaceOpened, DBusInterface);
							for(const auto & signal : oVec)
								this->signalToXMLSignal(xml, signal);
						}
					}
				}
			}
			catch (const std::out_of_range& oor)
			{
				GKLog2(trace,
					"can't iterate over DBusIntrospectableSignals. No bus container: ",
					toUInt(toEnumType(GKDBusEvents::currentBus))
				)
			}
		}

		this->closeXMLInterface(xml, interfaceOpened);
	}

	return xml_stream();
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
	try
	{
		connection = this->getDBusConnection(eventBus);
	}
	catch ( const GLogiKExcept & e )
	{
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
	try
	{
		connection = this->getDBusConnection(eventBus);
	}
	catch ( const GLogiKExcept & e )
	{
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

