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

#define GKDBUS_INSIDE_GKDBUS_H 1
#include "GKDBusEvents.h"
#undef GKDBUS_INSIDE_GKDBUS_H

#include "messages/GKDBusRemoteMethodCall.h"

namespace GLogiK
{

enum class BusConnection : uint8_t
{
	GKDBUS_SESSION = 0,
	GKDBUS_SYSTEM,
};

class GKDBus
	:	public GKDBusEvents,
		public GKDBusMessageRemoteMethodCall
{
	public:
		GKDBus(const std::string & rootnode);
		~GKDBus();

		void connectToSystemBus(const char* connection_name);

	protected:

	private:
		std::ostringstream buffer_;
		DBusError error_;

		DBusConnection* current_conn_;
		DBusConnection* session_conn_;
		DBusConnection* system_conn_;

		std::string session_name_;
		std::string system_name_;

		void checkDBusError(const char* error_message);
		void checkReleasedName(int ret);
};

} // namespace GLogiK

#endif
