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

#include <cstdint>

#include <string>
#include <vector>
#include <sstream>

#include <dbus/dbus.h>

#include "lib/shared/keyEvent.h"

#include "GKDBusEvents.h"

#include "messages/GKDBusBroadcastSignal.h"
#include "messages/GKDBusTargetsSignal.h"

namespace GLogiK
{

class GKDBus
	:	public GKDBusEvents,
		public GKDBusMessageBroadcastSignal,
		public GKDBusMessageTargetsSignal
{
	public:
		GKDBus(const std::string & rootnode);
		~GKDBus();

		void connectToSystemBus(const char* connection_name);

		const bool checkForNextMessage(BusConnection current);
		void checkForMethodCall(BusConnection current);

	protected:

	private:
		std::ostringstream buffer_;
		DBusError error_;

		DBusConnection* session_conn_;
		DBusConnection* system_conn_;

		std::string session_name_;
		std::string system_name_;

		DBusMessage* message_;

		void checkDBusError(const char* error_message);
		void checkReleasedName(int ret);
		DBusConnection* getConnection(BusConnection wanted_connection);
};

} // namespace GLogiK

#endif
