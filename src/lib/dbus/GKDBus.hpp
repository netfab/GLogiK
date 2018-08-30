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

#ifndef SRC_LIB_DBUS_GKDBUS_HPP_
#define SRC_LIB_DBUS_GKDBUS_HPP_

#include <string>
#include <vector>

#include <dbus/dbus.h>

#include "lib/utils/utils.hpp"

#include "GKDBusConnection.hpp"
#include "GKDBusEvents.hpp"

#include "messages/GKDBusRemoteMethodCall.hpp"
#include "messages/GKDBusBroadcastSignal.hpp"
#include "messages/GKDBusTargetsSignal.hpp"
#include "messages/GKDBusAsyncContainer.hpp"

#include "arguments/GKDBusArgString.hpp"
#include "arguments/GKDBusArgBoolean.hpp"
#include "arguments/GKDBusArgByte.hpp"
#include "arguments/GKDBusArgUInt16.hpp"
#include "arguments/GKDBusArgUInt64.hpp"
#include "arguments/GKDBusArgMacro.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

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
		public GKDBusMessageAsyncContainer,
		virtual public GKDBusArgumentString,
		public GKDBusArgumentBoolean,
		virtual public GKDBusArgumentByte,
		virtual public GKDBusArgumentUInt16,
		public GKDBusArgumentUInt64,
		public GKDBusArgumentMacro
{
	public:
		GKDBus(const std::string & rootNode);
		~GKDBus();

		void connectToSystemBus(const char* connectionName);

		void checkForNextMessage(const BusConnection bus) noexcept;

	protected:

	private:
		DBusError _error;

		//DBusConnection* _sessionConnection;
		DBusConnection* _systemConnection;

		//std::string _sessionName;
		std::string _systemName;

		DBusMessage* _message;

		void checkDBusError(const char* error);
		void checkReleasedName(int ret);
		DBusConnection* getConnection(BusConnection bus);
};

} // namespace NSGKDBus

#endif
