/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
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

using namespace NSGKUtils;

GKDBus::GKDBus(
	const std::string & rootNode,
	const std::string & rootNodePath)
		:	GKDBusEvents(rootNode, rootNodePath),
			_sessionConnection(nullptr),
			_systemConnection(nullptr)
{
	GK_LOG_FUNC

#if DEBUGGING_ON
	LOG(DEBUG1) << "dbus object initialization";
#endif
	dbus_error_init(&_error);
}

GKDBus::~GKDBus()
{
	GK_LOG_FUNC

#if DEBUGGING_ON
	LOG(DEBUG1) << "dbus object destruction";
#endif

	this->disconnectFromSessionBus();
	this->disconnectFromSystemBus();
}

void GKDBus::connectToSystemBus(
	const char* connectionName,
	const ConnectionFlag flag)
{
	GK_LOG_FUNC

	_systemConnection = dbus_bus_get(DBUS_BUS_SYSTEM, &_error);
	this->checkDBusError("DBus System connection failure");
#if DEBUGGING_ON
	LOG(DEBUG1) << "DBus System connection opened";
#endif

#if DEBUGGING_ON
	LOG(DEBUG2) << "requesting system connection name : " << connectionName;
#endif
	_systemName.clear();
	int ret = dbus_bus_request_name(_systemConnection, connectionName, getDBusRequestFlags(flag), &_error);
	this->checkDBusError("DBus System request name failure");
	_systemName = connectionName;

	if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		throw GLogiKExcept("DBus System request name failure : not owner");
	}
}

void GKDBus::disconnectFromSystemBus(void) noexcept
{
	GK_LOG_FUNC

	if(_systemConnection) {
#if DEBUGGING_ON
		LOG(DEBUG) << "closing DBus System connection";
#endif

		if( ! _systemName.empty() ) {
			int ret = dbus_bus_release_name(_systemConnection, _systemName.c_str(), &_error);
			this->checkReleasedName(ret);
		}
		dbus_connection_unref(_systemConnection);
		_systemConnection = nullptr;
	}
}

void GKDBus::connectToSessionBus(
	const char* connectionName,
	const ConnectionFlag flag)
{
	GK_LOG_FUNC

	_sessionConnection = dbus_bus_get(DBUS_BUS_SESSION, &_error);
	this->checkDBusError("DBus Session connection failure");
#if DEBUGGING_ON
	LOG(DEBUG1) << "DBus Session connection opened";
#endif

#if DEBUGGING_ON
	LOG(DEBUG2) << "requesting session connection name : " << connectionName;
#endif
	_sessionName.clear();
	int ret = dbus_bus_request_name(_sessionConnection, connectionName, getDBusRequestFlags(flag), &_error);
	this->checkDBusError("DBus Session request name failure");
	_sessionName = connectionName;

	if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		throw GLogiKExcept("DBus Session request name failure : not owner");
	}
}

void GKDBus::disconnectFromSessionBus(void) noexcept
{
	GK_LOG_FUNC

	if(_sessionConnection) {
#if DEBUGGING_ON
		LOG(DEBUG) << "closing DBus Session connection";
#endif

		if( ! _sessionName.empty() ) {
			int ret = dbus_bus_release_name(_sessionConnection, _sessionName.c_str(), &_error);
			this->checkReleasedName(ret);
		}
		dbus_connection_unref(_sessionConnection);
		_sessionConnection = nullptr;
	}
}

const std::string GKDBus::getObjectFromObjectPath(const std::string & objectPath)
{
	std::string object;
	std::istringstream path(objectPath);
	/* get last part of object path */
	while(std::getline(path, object, '/')) {}
#if 0 && DEBUGGING_ON
	LOG(DEBUG3) << "object path: " << objectPath;
	LOG(DEBUG3) << "     object: " << object;
#endif
	return object;
}

void GKDBus::checkForMessages(void) noexcept
{
	if(_systemConnection != nullptr)
		this->checkForBusMessages(BusConnection::GKDBUS_SYSTEM, _systemConnection);
	if(_sessionConnection != nullptr)
		this->checkForBusMessages(BusConnection::GKDBUS_SESSION, _sessionConnection);
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

void GKDBus::checkDBusMessage(DBusConnection* const connection)
{
	const std::string object = this->getObjectFromObjectPath(toString(dbus_message_get_path(_message)));

	for(const auto & objectPair : _DBusEvents.at(GKDBusEvents::currentBus)) {
		/* handle root node introspection special case */
		if( object != this->getRootNode() )
			/* object must match */
			if(object != objectPair.first) {
#if 0 && DEBUGGING_ON
				LOG(DEBUG3) << "skipping " << objectPair.first << " object - not " << object;
#endif
				continue;
			}

		for(const auto & interfacePair : objectPair.second) {
			const char* interface = interfacePair.first.c_str();
#if 0 && DEBUGGING_ON
			LOG(DEBUG2) << "checking " << interface << " interface";
#endif

			for(const auto & DBusEvent : interfacePair.second) { /* vector of pointers */
				const char* eventName = DBusEvent->eventName.c_str();
#if 0 && DEBUGGING_ON
				LOG(DEBUG3) << "checking " << eventName << " event";
#endif
				switch(DBusEvent->eventType) {
					case GKDBusEventType::GKDBUS_EVENT_METHOD:
					{
						if( dbus_message_is_method_call(_message, interface, eventName) )
						{
#if DEBUGGING_ON
							LOG(DEBUG1) << "DBus " << eventName << " method called !";
#endif
							DBusEvent->runCallback(connection, _message);
							return;
						}
						break;
					}
					case GKDBusEventType::GKDBUS_EVENT_SIGNAL:
					{
						if( dbus_message_is_signal(_message, interface, eventName) )
						{
#if DEBUGGING_ON
							LOG(DEBUG1) << "DBus " << eventName << " signal receipted !";
#endif
							DBusEvent->runCallback(connection, _message);
							return;
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

/* -- */
/*
 * check if :
 *      one of the registered methods
 *   on one of the registered interfaces
 *   on one of the registered object path
 * was called. if yes, then run the corresponding callback function
 * and send DBus reply after appending the return value
 */
void GKDBus::checkForBusMessages(
	const BusConnection bus,
	DBusConnection* const connection) noexcept
{
	GK_LOG_FUNC

	GKDBusEvents::currentBus = bus; /* used on introspection */

	uint16_t c = 0;
	while( true ) {
		dbus_connection_read_write(connection, 0);
		_message = dbus_connection_pop_message(connection);

		/* no message */
		if(_message == nullptr) {
#if DEBUGGING_ON
			if(c > 0) {
				LOG(DEBUG) << "processed " << c << " DBus messages";
			}
#endif
			return;
		}

		try {
			this->checkDBusMessage(connection);
		}
		catch (const std::out_of_range& oor) {
			LOG(ERROR) << "current bus connection oor";
		}
		catch ( const GLogiKExcept & e ) {
			LOG(ERROR) << e.what();
		}

#if 0 && DEBUGGING_ON
		LOG(DEBUG3) << "freeing DBus message";
#endif
		dbus_message_unref(_message);
		_message = nullptr;

		c++;
	}
}

void GKDBus::checkReleasedName(int ret) noexcept
{
	GK_LOG_FUNC

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
		case -1:
			if( dbus_error_is_set(&_error) ) {
				LOG(ERROR) << "release_name returns -1, error is : " << _error.message;
				dbus_error_free(&_error);
			}
			else {
				LOG(ERROR) << "release_name returns -1, but unknown error :-(";
			}
			break;
		default:
			LOG(ERROR) << "return value : " << ret;
			break;
	}
}

void GKDBus::checkDBusError(const char* error)
{
	if( dbus_error_is_set(&_error) ) {
		std::ostringstream buffer;
		buffer << error << " : " << _error.message;
		dbus_error_free(&_error);
		throw GLogiKExcept(buffer.str());
	}
}

DBusConnection* const GKDBus::getConnection(BusConnection bus) const
{
	switch(bus) {
		case BusConnection::GKDBUS_SESSION :
			if(_sessionConnection == nullptr)
				throw GLogiKExcept("DBus Session connection not opened");
			return _sessionConnection;
			break;
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

const unsigned int GKDBus::getDBusRequestFlags(const ConnectionFlag flag) noexcept
{
	unsigned int ret = 0;
	switch(flag) {
		case ConnectionFlag::GKDBUS_MULTIPLE :
			ret = DBUS_NAME_FLAG_REPLACE_EXISTING|DBUS_NAME_FLAG_ALLOW_REPLACEMENT;
			break;
		case ConnectionFlag::GKDBUS_SINGLE :
			ret = DBUS_NAME_FLAG_DO_NOT_QUEUE;
			break;
	}
	return ret;
}

} // namespace NSGKDBus

