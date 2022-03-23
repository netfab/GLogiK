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

#ifndef SRC_LIB_DBUS_GKDBUS_HPP_
#define SRC_LIB_DBUS_GKDBUS_HPP_

#include <cstdint>

#include <string>
#include <vector>

#include <dbus/dbus.h>

#include "lib/utils/utils.hpp"

#include "GKDBusConnection.hpp"
#include "GKDBusEvents.hpp"

#include "messages/GKDBusRemoteMethodCall.hpp"
#include "messages/GKDBusBroadcastSignal.hpp"
#include "messages/GKDBusAsyncContainer.hpp"

#include "arguments/GKDBusArgString.hpp"
#include "arguments/GKDBusArgBoolean.hpp"
#include "arguments/GKDBusArgByte.hpp"
#include "arguments/GKDBusArgUInt16.hpp"
#include "arguments/GKDBusArgUInt64.hpp"
#include "arguments/GKDBusArgMacro.hpp"
#include "arguments/GKDBusArgDevicesMap.hpp"
#include "arguments/GKDBusArgGKeysIDArray.hpp"
#include "arguments/GKDBusArgLCDPluginsArray.hpp"

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
		virtual public GKDBusArgumentString,
		public GKDBusArgumentBoolean,
		virtual public GKDBusArgumentByte,
		virtual public GKDBusArgumentUInt16,
		virtual public GKDBusArgumentUInt64,
		public GKDBusArgumentMacro,
		public GKDBusArgumentDevicesMap,
		public GKDBusArgumentGKeysIDArray,
		public GKDBusArgumentLCDPluginsArray
{
	public:
		GKDBus(
			const std::string & rootNode,
			const std::string & rootNodePath
		);
		~GKDBus();

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

		std::string _sessionName;
		std::string _systemName;

		DBusConnection* _sessionConnection;
		DBusConnection* _systemConnection;

		DBusMessage* _message;

		void checkDBusMessage(DBusConnection* const connection);
		void checkForBusMessages(
			const BusConnection bus,
			DBusConnection* const connection) noexcept;
		void checkReleasedName(int ret) noexcept;
		void checkDBusError(const char* error);
		DBusConnection* const getConnection(BusConnection bus) const;
		const unsigned int getDBusRequestFlags(const ConnectionFlag flag) noexcept;
};

} // namespace NSGKDBus

#endif
