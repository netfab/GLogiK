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

#include <new>
#include <algorithm>
#include <sstream>

#include "messages/GKDBusMessage.hpp"
#include "GKDBus.hpp"

namespace NSGKDBus
{

GKDBus::GKDBus(const std::string & rootNode)
	:	GKDBusEvents(rootNode),
		//_sessionConnection(nullptr),
		_systemConnection(nullptr)
{
#if DEBUGGING_ON
	LOG(DEBUG1) << "dbus object initialization";
#endif
	dbus_error_init(&_error);
}

GKDBus::~GKDBus()
{
#if DEBUGGING_ON
	LOG(DEBUG1) << "dbus object destruction";
#endif

/*
	if(_sessionConnection) {
#if DEBUGGING_ON
		LOG(DEBUG) << "closing DBus Session connection";
#endif

		if( ! _sessionName.empty() ) {
			int ret = dbus_bus_release_name(_sessionConnection, _sessionName.c_str(), nullptr);
			this->checkReleasedName(ret);
		}
		dbus_connection_unref(_sessionConnection);
	}
*/

	if(_systemConnection) {
#if DEBUGGING_ON
		LOG(DEBUG) << "closing DBus System connection";
#endif

		if( ! _systemName.empty() ) {
			int ret = dbus_bus_release_name(_systemConnection, _systemName.c_str(), nullptr);
			this->checkReleasedName(ret);
		}
		dbus_connection_unref(_systemConnection);
	}
}

void GKDBus::connectToSystemBus(const char* connectionName) {
	_systemConnection = dbus_bus_get(DBUS_BUS_SYSTEM, &_error);
	this->checkDBusError("DBus System connection failure");
#if DEBUGGING_ON
	LOG(DEBUG1) << "DBus System connection opened";
#endif

#if DEBUGGING_ON
	LOG(DEBUG2) << "requesting system connection name : " << connectionName;
#endif
	_systemName.clear();
	int ret = dbus_bus_request_name(_systemConnection, connectionName,
		DBUS_NAME_FLAG_REPLACE_EXISTING|DBUS_NAME_FLAG_ALLOW_REPLACEMENT, &_error);
	this->checkDBusError("DBus System request name failure");
	_systemName = connectionName;

	if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		throw GLogiKExcept("DBus System request name failure : not owner");
	}
}

/* -- */
/*
 * check if :
 *      one of the registered methods
 *   on one of the registered interfaces
 *   on one of the registered object path
 * was called. if yes, then run the corresponding callback function
 * and send DBus reply after appending the return value
 */
void GKDBus::checkForNextMessage(const BusConnection bus) {
	DBusConnection* connection = nullptr;
	GKDBusEvents::currentBus = bus; /* used on introspection */

	try {
		connection = this->getConnection(GKDBusEvents::currentBus);
	}
	catch ( const GLogiKExcept & e ) {
		LOG(ERROR) << e.what();
		return;
	}

	dbus_connection_read_write(connection, 0);
	_message = dbus_connection_pop_message(connection);

	/* no message */
	if(_message == nullptr) {
		return;
	}

	try {
		const std::string askedObjectPath( to_string( dbus_message_get_path(_message) ) );
		const auto & bus = _DBusEvents.at(GKDBusEvents::currentBus);

		for(const auto & objectPair : bus) {
			/* handle root node introspection special case */
			if( askedObjectPath != this->getRootNode() )
				/* object path must match */
				if(askedObjectPath != this->getNode(objectPair.first))
					continue;

			for(const auto & interfacePair : objectPair.second) {
				const char* interface = interfacePair.first.c_str();
#if DEBUGGING_ON
				LOG(DEBUG2) << "checking " << interface << " interface";
#endif

				for(const auto & DBusEvent : interfacePair.second) { /* vector of pointers */
					const char* eventName = DBusEvent->eventName.c_str();
#if 0 && DEBUGGING_ON
					LOG(DEBUG3) << "checking " << eventName << " event";
#endif
					switch(DBusEvent->eventType) {
						case GKDBusEventType::GKDBUS_EVENT_METHOD: {
							if( dbus_message_is_method_call(_message, interface, eventName) ) {
#if DEBUGGING_ON
								LOG(DEBUG1) << "DBus " << eventName << " method called !";
#endif
								DBusEvent->runCallback(connection, _message);
								throw GKDBusEventFound();
							}
							break;
						}
						case GKDBusEventType::GKDBUS_EVENT_SIGNAL: {
							if( dbus_message_is_signal(_message, interface, eventName) ) {
#if DEBUGGING_ON
								LOG(DEBUG1) << "DBus " << eventName << " signal receipted !";
#endif
								DBusEvent->runCallback(connection, _message);
								throw GKDBusEventFound();
							}
							break;
						}
						default:
							throw GLogiKExcept("wrong event type");
							break;
					}
				}
			}
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(ERROR) << "current bus connection oor";
	}
	catch ( const GKDBusEventFound & e ) {
		LOG(DEBUG2) << e.what();
	}
	catch ( const GLogiKExcept & e ) {
		LOG(ERROR) << e.what();
	}

#if DEBUGGING_ON
	LOG(DEBUG3) << "freeing DBus message";
#endif
	dbus_message_unref(_message);
	_message = nullptr;
}


/*
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 *
 * === private === private === private === private === private ===
 *
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 */

void GKDBus::checkReleasedName(int ret) {
	switch(ret) {
		case DBUS_RELEASE_NAME_REPLY_RELEASED:
			LOG(DEBUG1) << "name released";
			break;
		case DBUS_RELEASE_NAME_REPLY_NOT_OWNER:
			LOG(DEBUG1) << "not owner, cannot release";
			break;
		case DBUS_RELEASE_NAME_REPLY_NON_EXISTENT:
			LOG(DEBUG1) << "nobody owned the name";
			break;
		default:
			LOG(ERROR) << "return value : " << ret;
			break;
	}
}

void GKDBus::checkDBusError(const char* error) {
	if( dbus_error_is_set(&_error) ) {
		std::ostringstream buffer;
		buffer << error << " : " << _error.message;
		dbus_error_free(&_error);
		throw GLogiKExcept(buffer.str());
	}
}

DBusConnection* GKDBus::getConnection(BusConnection bus) {
	switch(bus) {
/*
		case BusConnection::GKDBUS_SESSION :
			if(_sessionConnection == nullptr)
				throw GLogiKExcept("DBus Session connection not opened");
			return _sessionConnection;
			break;
*/
		case BusConnection::GKDBUS_SYSTEM :
			if(_systemConnection == nullptr)
				throw GLogiKExcept("DBus System connection not opened");
			return _systemConnection;
			break;
		default:
			throw GLogiKExcept("asked connection not handled");
			break;
	}
}

} // namespace NSGKDBus

