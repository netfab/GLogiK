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

#include <new>
#include <algorithm>
#include <sstream>
#include <vector>

#include "messages/GKDBusMessage.hpp"
#include "GKDBus.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

const BusConnection GKDBus::SystemBus(BusConnection::GKDBUS_SYSTEM);
const BusConnection GKDBus::SessionBus(BusConnection::GKDBUS_SESSION);

GKDBus::GKDBus(
	const std::string & rootNode,
	const std::string & rootNodePath)
		:	GKDBusEvents(rootNode, rootNodePath),
			_sessionConnection(nullptr),
			_systemConnection(nullptr),
			_initDone(false)
{
}

GKDBus::~GKDBus()
{
}

void GKDBus::init(void)
{
	GK_LOG_FUNC

	{
		int major_vers, minor_vers, micro_vers = 0;
		dbus_get_version(&major_vers, &minor_vers, &micro_vers);
		_currentDBusVersion += std::to_string(major_vers);
		_currentDBusVersion += ".";
		_currentDBusVersion += std::to_string(minor_vers);
		_currentDBusVersion += ".";
		_currentDBusVersion += std::to_string(micro_vers);
	}

	GKLog(trace, "GKDBus initialization")
	GKLog2(trace, "GKDBus built against DBus: ", _builtAgainstDBusVersion)
	GKLog2(trace, "GKDBus running with DBus: ", _currentDBusVersion)

	dbus_error_init(&_error);

	std::string s;
	GKLog4(trace, "checking std::string ::max_size(): ", s.max_size(), "UINT64_MAX: ", UINT64_MAX)
	if( s.max_size() > UINT64_MAX )
		throw GLogiKExcept("std::string ::max_size() overflow detected");

	const std::vector<std::string> v;
	GKLog4(trace, "checking std::vector<std::string> ::max_size(): ", v.max_size(), "UINT64_MAX: ", UINT64_MAX)
	if( v.max_size() > UINT64_MAX )
		throw GLogiKExcept("std::vector<std::string> ::max_size() overflow detected");

	_initDone = true;
}

const std::string GKDBus::getDBusVersion(void)
{
	struct DBusVersion {
		int major = -1;
		int minor = -1;
		int micro = -1;
	};

	DBusVersion version;
	int* major = &(version.major);
	int* minor = &(version.minor);
	int* micro = &(version.micro);

	dbus_get_version(major, minor, micro);

	std::string ret;

	ret += std::to_string(*major);
	ret += ".";
	ret += std::to_string(*minor);
	ret += ".";
	ret += std::to_string(*micro);

	return ret;
}

void GKDBus::connectToSystemBus(
	const char* connectionName,
	const ConnectionFlag flag)
{
	GK_LOG_FUNC

	if( ! _initDone )
		throw GLogiKExcept(_initError);

	_systemConnection = dbus_bus_get(DBUS_BUS_SYSTEM, &_error);
	this->checkDBusError("failed to open system bus connection");

	GKLog(trace, "opened system bus connection")

	_systemName.clear();
	int ret = dbus_bus_request_name(_systemConnection, connectionName, getDBusRequestFlags(flag), &_error);
	this->checkDBusError("failed to request system bus connection name");
	_systemName = connectionName;

	GKLog2(trace, "requested system bus connection name : ", connectionName)

	if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		throw GLogiKExcept("failed to request system bus connection name : not owner");
	}
}

void GKDBus::connectToSessionBus(
	const char* connectionName,
	const ConnectionFlag flag)
{
	GK_LOG_FUNC

	if( ! _initDone )
		throw GLogiKExcept(_initError);

	_sessionConnection = dbus_bus_get(DBUS_BUS_SESSION, &_error);
	this->checkDBusError("failed to open session bus connection");

	GKLog(trace, "opened session bus connection")

	_sessionName.clear();
	int ret = dbus_bus_request_name(_sessionConnection, connectionName, getDBusRequestFlags(flag), &_error);
	this->checkDBusError("failed to request session bus connection name");
	_sessionName = connectionName;

	GKLog2(trace, "requested session bus connection name : ", connectionName)

	if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		throw GLogiKExcept("failed to request session bus connection name : not owner");
	}
}

void GKDBus::exit(void) noexcept
{
	GK_LOG_FUNC

	GKLog(trace, "disconnecting and clearing DBusEvents")

	this->disconnectFromSystemBus();
	this->disconnectFromSessionBus();
	this->clearDBusEvents();
}

const std::string GKDBus::getObjectFromObjectPath(const std::string & objectPath)
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

void GKDBus::checkForMessages(void) noexcept
{
	std::lock_guard<std::mutex> lock(_lockMutex);

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

void GKDBus::disconnectFromSystemBus(void) noexcept
{
	GK_LOG_FUNC

	if(_systemConnection) {
		GKLog(trace, "closing system bus connection")

		if( ! _systemName.empty() ) {
			int ret = dbus_bus_release_name(_systemConnection, _systemName.c_str(), &_error);
			this->checkReleasedName(ret);
		}
		dbus_connection_unref(_systemConnection);
		_systemConnection = nullptr;
	}
}

void GKDBus::disconnectFromSessionBus(void) noexcept
{
	GK_LOG_FUNC

	if(_sessionConnection) {
		GKLog(trace, "closing session bus connection")

		if( ! _sessionName.empty() ) {
			int ret = dbus_bus_release_name(_sessionConnection, _sessionName.c_str(), &_error);
			this->checkReleasedName(ret);
		}
		dbus_connection_unref(_sessionConnection);
		_sessionConnection = nullptr;
	}
}

void GKDBus::checkDBusMessage(
	DBusConnection* const connection,
	DBusMessage* message)
{
	const std::string object = this->getObjectFromObjectPath(
		toString(dbus_message_get_path(message))
	);

	for(const auto & objectPair : _DBusEvents.at(GKDBusEvents::currentBus)) {
		/* handle root node introspection special case */
		if( object != this->getRootNode() )
			/* object must match */
			if(object != objectPair.first) {
				//GKLog4(trace, "skipping object : ", objectPair.first, "not : ", object)
				continue;
			}

		for(const auto & interfacePair : objectPair.second) {
			const char* interface = interfacePair.first.c_str();
			//GKLog2(trace, "checking interface : ", interface)

			for(const auto & DBusEvent : interfacePair.second) { /* vector of pointers */
				const char* eventName = DBusEvent->eventName.c_str();
				//GKLog2(trace, "checking event : ", eventName)

				switch(DBusEvent->eventType) {
					case GKDBusEventType::GKDBUS_EVENT_METHOD:
					{
						if( dbus_message_is_method_call(message, interface, eventName) )
						{
							GKLog2(trace, "receipted DBus method call : ", eventName)
							DBusMessage* asyncContainer = this->getAsyncContainer();
							DBusEvent->runCallback(connection, message, asyncContainer);
							this->resetAsyncContainer();
							return;
						}
						break;
					}
					case GKDBusEventType::GKDBUS_EVENT_SIGNAL:
					{
						if( dbus_message_is_signal(message, interface, eventName) )
						{
							GKLog2(trace, "receipted DBus signal : ", eventName)
							DBusMessage* asyncContainer = this->getAsyncContainer();
							DBusEvent->runCallback(connection, message, asyncContainer);
							this->resetAsyncContainer();
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

#if DEBUGGING_ON
	uint16_t c = 0;
#endif

	while( true ) {
		dbus_connection_read_write(connection, 0);
		DBusMessage* message = dbus_connection_pop_message(connection);

		/* no message */
		if(message == nullptr) {
#if DEBUGGING_ON
			if(c > 0) {
				GKLog3(trace, "processed ", c, " DBus messages")
			}
#endif
			return;
		}

		try {
			this->checkDBusMessage(connection, message);
		}
		catch (const std::out_of_range& oor) {
			LOG(error) << "current bus connection oor";
		}
		catch ( const GLogiKExcept & e ) {
			LOG(error) << e.what();
		}

		//GKLog(trace, "freeing DBus message")
		dbus_message_unref(message);
		message = nullptr;

#if DEBUGGING_ON
		c++;
#endif
	}
}

void GKDBus::checkReleasedName(int ret) noexcept
{
	GK_LOG_FUNC

	switch(ret) {
		case DBUS_RELEASE_NAME_REPLY_RELEASED:
			GKLog(trace, "name released")
			break;
		case DBUS_RELEASE_NAME_REPLY_NOT_OWNER:
			GKLog(trace, "not owner, cannot release")
			break;
		case DBUS_RELEASE_NAME_REPLY_NON_EXISTENT:
			GKLog(trace, "nobody owned the name")
			break;
		case -1:
			if( dbus_error_is_set(&_error) ) {
				LOG(error) << "release_name returns -1, error is : " << _error.message;
				dbus_error_free(&_error);
			}
			else {
				LOG(error) << "release_name returns -1, but unknown error :-(";
			}
			break;
		default:
			LOG(error) << "return value : " << ret;
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

DBusConnection* const GKDBus::getDBusConnection(BusConnection bus) const
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

