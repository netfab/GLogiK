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

#ifndef SRC_LIB_DBUS_MSG_GKDBUS_REMOTE_METHOD_CALL_HPP_
#define SRC_LIB_DBUS_MSG_GKDBUS_REMOTE_METHOD_CALL_HPP_

#include <cstdint>

#include <string>

#include <dbus/dbus.h>

#include "include/base.hpp"
#include "include/MBank.hpp"
#include "lib/dbus/GKDBusConnection.hpp"
#include "lib/dbus/ArgTypes/string.hpp"

#include "GKDBusMessage.hpp"

namespace NSGKDBus
{

class GKDBusRemoteMethodCall
	:	public GKDBusMessage
{
	public:
		GKDBusRemoteMethodCall(
			DBusConnection* wantedConnection,
			const char* busName,
			const char* objectPath,
			const char* interface,
			const char* method,
			DBusPendingCall** pending
		);
		~GKDBusRemoteMethodCall();

	protected:
	private:
		DBusPendingCall** _pendingCall;

};

class GKDBusMessageRemoteMethodCall
	:	virtual private ArgString
{
	public:
		/* Remote Method Call with Pending Reply */
		void initializeRemoteMethodCall(
			BusConnection wantedConnection,
			const char* busName,
			const char* objectPath,
			const char* interface,
			const char* method
		);

		void appendStringToRemoteMethodCall(const std::string & value);
		void appendUInt8ToRemoteMethodCall(const uint8_t value);
		void appendUInt32ToRemoteMethodCall(const uint32_t value);
		void appendUInt64ToRemoteMethodCall(const uint64_t value);
		void appendGKeysIDToRemoteMethodCall(const GLogiK::GKeysID keyID);
		void appendMKeysIDToRemoteMethodCall(const GLogiK::MKeysID bankID);

		void sendRemoteMethodCall(void);
		void abandonRemoteMethodCall(void);

		void waitForRemoteMethodCallReply(void);

	protected:
		GKDBusMessageRemoteMethodCall();
		~GKDBusMessageRemoteMethodCall();

		/* Remote Method Call with Pending Reply */
		void initializeRemoteMethodCall(
			DBusConnection* const connection,
			const char* busName,
			const char* objectPath,
			const char* interface,
			const char* method
		);

	private:
		GKDBusRemoteMethodCall* _remoteMethodCall;
		DBusPendingCall* _pendingCall;

		virtual DBusConnection* const getDBusConnection(BusConnection wantedConnection) const = 0;
};

} // namespace NSGKDBus

#endif

