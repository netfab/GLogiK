/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2026  Fabrice Delliaux <netbox253@gmail.com>
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

#pragma once

#include "config.h"

#include <cstdint>

#include <string>
#include <mutex>

#include <dbus/dbus.h>

#include "GKDBusConnection.hpp"
#include "GKDBusEvents.hpp"

#include "messages/GKDBusRemoteMethodCall.hpp"
#include "messages/GKDBusBroadcastSignal.hpp"
#include "messages/GKDBusAsyncContainer.hpp"
#include "messages/GKDBusReply.hpp"
#include "messages/GKDBusErrorReply.hpp"

#include "ArgTypes/boolean.hpp"
#include "ArgTypes/int32.hpp"
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
#include "ArgTypes/DepsMap.hpp"

namespace GKDBUS
{

enum class ConnectionFlag : std::uint8_t
{
	GKDBUS_SINGLE = 0,
	GKDBUS_MULTIPLE,
};

class GKDBus
	:	public GKDBUS::Events::GKDBusEvents,
		virtual public GKDBUS::Messages::GKDBusMessageRemoteMethodCall,
		public GKDBUS::Messages::GKDBusMessageBroadcastSignal,
		public GKDBUS::Messages::GKDBusMessageAsyncContainer,
		private GKDBUS::Messages::GKDBusMessageReply,
		private GKDBUS::Messages::GKDBusMessageErrorReply,
		virtual public Types::ArgString,
		virtual public Types::ArgStringArray,
		public Types::ArgBoolean,
		virtual public Types::ArgInt32,
		virtual public Types::ArgUInt8,
		virtual public Types::ArgUInt16,
		virtual public Types::ArgUInt64,
		public Types::ArgMacro,
		public Types::ArgDevicesMap,
		public Types::ArgGKeysIDArray,
		public Types::ArgMKeysIDArray,
		public Types::ArgLCDPPArray,
		public Types::ArgGKDepsMap
{
	private:
		using BusConnection = GKDBUS::BusConnection;

	public:
		GKDBus();
		~GKDBus();

		static const BusConnection SystemBus;
		static const BusConnection SessionBus;
		static std::string getDBusVersion();

		void init();

		const std::string & getBuiltAgainstDBusVersion()
		{
			return _builtAgainstDBusVersion;
		}

		void connectToSystemBus(
			const std::string & connectionName,
			ConnectionFlag flag = ConnectionFlag::GKDBUS_SINGLE
		);
		void connectToSessionBus(
			const std::string & connectionName,
			ConnectionFlag flag = ConnectionFlag::GKDBUS_SINGLE
		);

		void exit() noexcept;

		void checkForMessages() noexcept;

	protected:

	private:
		DBusError _error;

		std::string _currentDBusVersion;
		/* see also DBUS_VERSION_STRING from dbus.h */
		const std::string _builtAgainstDBusVersion = GK_DEP_DBUS_VERSION_STRING;
		const std::string _initError = "GKDBus must be initialized before anything else";

		std::string _sessionName;
		std::string _systemName;

		std::mutex _lockMutex;

		DBusConnection* _sessionConnection;
		DBusConnection* _systemConnection;

		bool _initDone;

		void disconnectFromSystemBus() noexcept;
		void disconnectFromSessionBus() noexcept;

		bool checkDBusMessage(
			DBusConnection* const connection,
			DBusMessage* message
		);
		void checkForBusMessages(
			BusConnection bus,
			DBusConnection* const connection
		) noexcept;
		void checkReleasedName(int ret) noexcept;
		void checkDBusError(const std::string & errorMessage);
		DBusConnection* const getDBusConnection(BusConnection bus) const;
		unsigned int getDBusRequestFlags(ConnectionFlag flag) noexcept;
};

} // namespace GKDBUS
