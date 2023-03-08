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

#ifndef SRC_LIB_DBUS_GKDBUS_HPP_
#define SRC_LIB_DBUS_GKDBUS_HPP_

#include <cstdint>

#include <string>
#include <mutex>

#include <dbus/dbus.h>

#include "lib/utils/utils.hpp"

#include "GKDBusConnection.hpp"
#include "GKDBusEvents.hpp"

#include "messages/GKDBusRemoteMethodCall.hpp"
#include "messages/GKDBusBroadcastSignal.hpp"
#include "messages/GKDBusAsyncContainer.hpp"

#include "ArgTypes/boolean.hpp"
#include "ArgTypes/uint8.hpp"
#include "ArgTypes/uint16.hpp"
#include "ArgTypes/uint64.hpp"
#include "ArgTypes/string.hpp"
#include "ArgTypes/stringArray.hpp"
#include "ArgTypes/GKeysIDArray.hpp"
#include "ArgTypes/MKeysIDArray.hpp"
#include "ArgTypes/macro.hpp"
#include "ArgTypes/DevicesMap.hpp"
#include "ArgTypes/LCDPPArray.hpp"

namespace NSGKDBus
{

enum class ConnectionFlag : uint8_t
{
	GKDBUS_SINGLE = 0,
	GKDBUS_MULTIPLE,
};

class GKDBus
	:	public GKDBusEvents,
		virtual public GKDBusMessageRemoteMethodCall,
		public GKDBusMessageBroadcastSignal,
		public GKDBusMessageAsyncContainer,
		virtual public ArgString,
		virtual public ArgStringArray,
		public ArgBoolean,
		virtual public ArgUInt8,
		virtual public ArgUInt16,
		virtual public ArgUInt64,
		public ArgMacro,
		public ArgDevicesMap,
		public ArgGKeysIDArray,
		public ArgMKeysIDArray,
		public ArgLCDPPArray
{
	public:
		GKDBus(
			const std::string & rootNode,
			const std::string & rootNodePath
		);
		~GKDBus();

		static const std::string getDBusVersion(void);

		const std::string & getBuiltAgainstDBusVersion(void) {
			return _builtAgainstDBusVersion;
		}

		void connectToSystemBus(
			const char* connectionName,
			const ConnectionFlag flag = ConnectionFlag::GKDBUS_MULTIPLE
		);
		void connectToSessionBus(
			const char* connectionName,
			const ConnectionFlag flag = ConnectionFlag::GKDBUS_MULTIPLE
		);

		void disconnectFromSystemBus(void) noexcept;
		void disconnectFromSessionBus(void) noexcept;

		const std::string getObjectFromObjectPath(const std::string & objectPath);
		void checkForMessages(void) noexcept;

	protected:

	private:
		DBusError _error;

		std::string _currentDBusVersion;
		/* see also DBUS_VERSION_STRING from dbus.h */
		const std::string _builtAgainstDBusVersion = GK_DEP_DBUS_VERSION_STRING;

		std::string _sessionName;
		std::string _systemName;

		std::mutex _lockMutex;

		DBusConnection* _sessionConnection;
		DBusConnection* _systemConnection;

		void checkDBusMessage(
			DBusConnection* const connection,
			DBusMessage* message
		);
		void checkForBusMessages(
			const BusConnection bus,
			DBusConnection* const connection
		) noexcept;
		void checkReleasedName(int ret) noexcept;
		void checkDBusError(const char* error);
		DBusConnection* const getConnection(BusConnection bus) const;
		const unsigned int getDBusRequestFlags(const ConnectionFlag flag) noexcept;
};

} // namespace NSGKDBus

#endif
