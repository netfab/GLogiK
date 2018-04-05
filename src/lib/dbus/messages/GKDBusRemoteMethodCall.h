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

#ifndef __GLOGIK_GKDBUS_REMOTE_METHOD_CALL_H__
#define __GLOGIK_GKDBUS_REMOTE_METHOD_CALL_H__

#include <cstdint>

#include <string>

#include <dbus/dbus.h>

#include "include/keyEvent.h"
#include "lib/dbus/GKDBusConnection.h"
#include "lib/dbus/arguments/GKDBusArgString.h"

#include "GKDBusMessage.h"

namespace NSGKDBus
{

class GKDBusRemoteMethodCall
	:	public GKDBusMessage
{
	public:
		GKDBusRemoteMethodCall(
			DBusConnection* wanted_connection,
			const char* dest,
			const char* object_path,
			const char* interface,
			const char* method,
			DBusPendingCall** pending,
			const bool logoff
		);
		~GKDBusRemoteMethodCall();

	protected:
	private:
		DBusPendingCall** pending_;

};

class GKDBusMessageRemoteMethodCall
	:	virtual private GKDBusArgumentString
{
	public:
		/* Remote Method Call with Pending Reply */
		void initializeRemoteMethodCall(
			BusConnection current,
			const char* dest,
			const char* object_path,
			const char* interface,
			const char* method,
			const bool logoff=false
		);

		void appendStringToRemoteMethodCall(const std::string & value);
		void appendUInt8ToRemoteMethodCall(const uint8_t value);
		void appendUInt32ToRemoteMethodCall(const uint32_t value);
		void appendMacrosBankToRemoteMethodCall(const GLogiK::macros_bank_t & macros_bank);

		void sendRemoteMethodCall(void);
		void abandonRemoteMethodCall(void);

		void waitForRemoteMethodCallReply(void);

	protected:
		GKDBusMessageRemoteMethodCall();
		~GKDBusMessageRemoteMethodCall();

		/* Remote Method Call with Pending Reply */
		void initializeRemoteMethodCall(
			DBusConnection* connection,
			const char* dest,
			const char* object_path,
			const char* interface,
			const char* method,
			const bool logoff=false
		);

	private:
		GKDBusRemoteMethodCall* remote_method_call_;
		DBusPendingCall* pending_;
		bool log_off_;

		virtual DBusConnection* getConnection(BusConnection wanted_connection) = 0;
};

} // namespace NSGKDBus

#endif

