/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2019  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_LIB_DBUS_MSG_GKDBUS_REPLY_HPP_
#define SRC_LIB_DBUS_MSG_GKDBUS_REPLY_HPP_

#include <string>
#include <vector>

#include <dbus/dbus.h>

#include "include/keyEvent.hpp"

#include "GKDBusMessage.hpp"

#include "lib/dbus/arguments/GKDBusArgString.hpp"
#include "lib/dbus/arguments/GKDBusArgUInt64.hpp"

#include "lib/dbus/messages/GKDBusAsyncContainer.hpp"

namespace NSGKDBus
{

class GKDBusReply
	:	public GKDBusMessage
{
	public:
		GKDBusReply(DBusConnection* connection, DBusMessage* message);
		~GKDBusReply();

	protected:

	private:

};

class GKDBusMessageReply
	:	virtual private GKDBusArgumentString,
		virtual private GKDBusArgumentUInt64,
		private GKDBusMessageAsyncContainer
{
	public:

	protected:
		GKDBusMessageReply();
		~GKDBusMessageReply();

		void initializeReply(DBusConnection* connection, DBusMessage* message);

		void appendBooleanToReply(const bool value);
		void appendStringToReply(const std::string & value);
		void appendStringVectorToReply(
			const std::vector<std::string> & list
		);
		void appendMacroToReply(const GLogiK::macro_type & macro);

		void appendUInt64ToReply(const uint64_t value);

		void appendAsyncArgsToReply(void);

		void sendReply(void);
		void abandonReply(void);

	private:
		GKDBusReply* _reply;
};

} // namespace NSGKDBus

#endif
