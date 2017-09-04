/*
 *
 *	This file is part of GLogiK project.
 *	GLogiKd, daemon to handle special features on gaming keyboards
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

#ifndef __GLOGIK_DBUS_H__
#define __GLOGIK_DBUS_H__

#define GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME "com.glogik.daemon"

#include <string>
#include <vector>
#include <sstream>
#include <dbus/dbus.h>

#include "GKDBusMsgReply.h"

namespace GLogiK
{

enum class BusConnection : int8_t
{
	GKDBUS_SESSION = 0,
	GKDBUS_SYSTEM,
};

class DBus
{
	public:
		DBus();
		~DBus();

		void connectToSessionBus(const char* connection_name);

		const bool checkForNextMessage(BusConnection current);
		const bool checkMessageForSignalOnInterface(const char* interface, const char* signal_name);
		const bool checkMessageForMethodCallOnInterface(const char* interface, const char* method);

		void addSignalMatch(BusConnection current, const char* interface);

		void initializeMethodCallReply(BusConnection current);
		void appendToMethodCallReply(const bool value);
		void sendMethodCallReply(void);

		std::string getNextStringArgument(void);

	protected:

	private:
		std::ostringstream buffer_;
		DBusError error_;
		DBusMessage* message_;
		DBusConnection* current_conn_;
		DBusConnection* session_conn_;
		DBusConnection* system_conn_;
		std::vector<std::string> string_arguments_;

		GKDBusMsgReply* reply_;

		void setCurrentConnection(BusConnection current);
		void checkDBusError(const char* error_message);
		void fillInArguments(void);
};

} // namespace GLogiK

#endif
