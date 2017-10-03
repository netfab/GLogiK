/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2017  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef __GLOGIK_GKDBUS_H__
#define __GLOGIK_GKDBUS_H__

#include <string>
#include <vector>
#include <sstream>

#include <dbus/dbus.h>

#define GKDBUS_INSIDE_GKDBUS_H 1
#include "GKDBusEvents.h"
#include "GKDBusMsgReply.h"
#include "GKDBusRemoteMethodCall.h"
#include "GKDBusBroadcastSignal.h"
#undef GKDBUS_INSIDE_GKDBUS_H

namespace GLogiK
{

enum class BusConnection : int8_t
{
	GKDBUS_SESSION = 0,
	GKDBUS_SYSTEM,
};

class GKDBus : public GKDBusEvents
{
	public:
		GKDBus(const std::string & rootnode);
		~GKDBus();

		void connectToSessionBus(const char* connection_name);
		void connectToSystemBus(const char* connection_name);

		const bool checkForNextMessage(BusConnection current);
		const bool checkMessageForSignalReceiptOnInterface(const char* interface, const char* signal_name);
		const bool checkMessageForMethodCallOnInterface(const char* interface, const char* method);
		void freeMessage(void);

		void initializeBroadcastSignal(BusConnection current, const char* object,
			const char* interface, const char* signal);
		void appendToBroadcastSignal(const std::string & value);
		void sendBroadcastSignal(void);

		void initializeMethodCallReply(BusConnection current);
		void appendToMethodCallReply(const bool value);
		void appendToMethodCallReply(const std::string & value);
		void appendToMethodCallReply(const std::vector<std::string> & list);
		void sendMethodCallReply(void);

		void initializeRemoteMethodCall(BusConnection current, const char* dest,
			const char* object, const char* interface, const char* method, const bool logoff=false);
		void appendToRemoteMethodCall(const std::string & value);
		void sendRemoteMethodCall(void);
		void waitForRemoteMethodCallReply(void);

		std::string getNextStringArgument(void);
		const bool getNextBooleanArgument(void);
		void checkForMethodCall(BusConnection current);
		void checkForSignalReceipt(BusConnection current);

		/* wrappers */
		void addSignal_StringToBool_Callback(BusConnection current,
			const char* object, const char* interface, const char* eventName,
			std::vector<DBusMethodArgument> args, std::function<const bool(const std::string&)> callback);
		void addSignal_VoidToVoid_Callback(BusConnection current,
			const char* object, const char* interface, const char* eventName,
			std::vector<DBusMethodArgument> args, std::function<void(void)> callback);

	protected:

	private:
		std::ostringstream buffer_;
		DBusError error_;
		DBusMessage* message_;
		DBusPendingCall* pending_;
		DBusConnection* current_conn_;
		DBusConnection* session_conn_;
		DBusConnection* system_conn_;
		std::vector<std::string> string_arguments_;
		std::vector<bool> boolean_arguments_;

		GKDBusMsgReply* reply_;
		GKDBusRemoteMethodCall* method_call_;
		GKDBusBroadcastSignal* signal_;

		void setCurrentConnection(BusConnection current);
		void checkDBusError(const char* error_message);
		void fillInArguments(void);
		void addSignalRuleMatch(const char* interface, const char* eventName);
};

} // namespace GLogiK

#endif