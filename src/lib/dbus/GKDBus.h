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

#include "lib/utils/utils.h"
#include "lib/shared/keyEvent.h"

#include "GKDBusEvents.h"

#include "messages/GKDBusRemoteMethodCall.h"
#include "messages/GKDBusBroadcastSignal.h"
#include "messages/GKDBusTargetsSignal.h"

#include "arguments/GKDBusArgString.h"
#include "arguments/GKDBusArgBoolean.h"
#include "arguments/GKDBusArgByte.h"
#include "arguments/GKDBusArgUInt16.h"

namespace GLogiK
{

class GKDBusEventFound : public GLogiKExcept
{
	public:
		GKDBusEventFound(const std::string & msg = "event found") : GLogiKExcept(msg) {};
		virtual ~GKDBusEventFound( void ) throw() {};
};

class GKDBus
	:	public GKDBusEvents,
		virtual public GKDBusMessageRemoteMethodCall,
		public GKDBusMessageBroadcastSignal,
		public GKDBusMessageTargetsSignal,
		virtual public GKDBusArgumentString,
		public GKDBusArgumentBoolean,
		public GKDBusArgumentByte,
		public GKDBusArgumentUInt16
{
	public:
		GKDBus(const std::string & rootnode);
		~GKDBus();

		void connectToSystemBus(const char* connection_name);

		const bool checkForNextMessage(BusConnection current);
		void checkMessageType(BusConnection current);

		void appendExtraStringToMessage(const std::string & value);

		// helper function rebuilding macro_t vector
		// mirror of GKDBusMessage::appendMacro
		const macro_t getMacro(void);

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
