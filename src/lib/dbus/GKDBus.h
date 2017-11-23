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
#include "GKDBusMessageReply.h"
#include "GKDBusRemoteMethodCall.h"
#include "GKDBusBroadcastSignal.h"
#include "GKDBusMessageErrorReply.h"
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
		const bool checkMessageForSignalReceipt(const char* interface, const char* signal);
		const bool checkMessageForMethodCall(const char* interface, const char* method);
		void freeMessage(void);

		void initializeBroadcastSignal(BusConnection current, const char* object_path,
			const char* interface, const char* signal);
		//void appendToBroadcastSignal(const std::string & value);
		void sendBroadcastSignal(void);

		void initializeTargetsSignal(BusConnection current, const char* dest, const char* object_path,
			const char* interface, const char* signal);
		void sendTargetsSignal(void);

		void initializeMethodCallReply(BusConnection current);
		void appendToMethodCallReply(const bool value);
		void appendToMethodCallReply(const std::string & value);
		void appendToMethodCallReply(const std::vector<std::string> & list);
		void appendExtraToMethodCallReply(const std::string & value);
		void sendMethodCallReply(void);

		void initializeRemoteMethodCall(BusConnection current, const char* dest,
			const char* object_path, const char* interface, const char* method, const bool logoff=false);
		void appendToRemoteMethodCall(const std::string & value);
		void appendToRemoteMethodCall(const uint32_t value);
		void sendRemoteMethodCall(void);
		void waitForRemoteMethodCallReply(void);

		std::string getNextStringArgument(void);
		const std::vector<std::string> & getAllStringArguments(void) const;
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
		std::vector<std::string> extra_strings_;

		GKDBusMessageReply* reply_;
		GKDBusRemoteMethodCall* method_call_;
		GKDBusBroadcastSignal* signal_;
		GKDBusMessageErrorReply* error_reply_;

		std::vector<GKDBusBroadcastSignal*> signals_;

		void setCurrentConnection(BusConnection current);
		void checkDBusError(const char* error_message);
		void fillInArguments(DBusMessage* message);
		void addSignalRuleMatch(const char* interface, const char* eventName);
		void appendExtraValuesToReply(void);

		void buildAndSendErrorReply(BusConnection current);
		void initializeMessageErrorReply(BusConnection current);
		void sendMessageErrorReply(void);
};

} // namespace GLogiK

#endif
