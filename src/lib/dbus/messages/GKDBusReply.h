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

#ifndef __GLOGIK_GKDBUS_REPLY_H__
#define __GLOGIK_GKDBUS_REPLY_H__

#include <string>
#include <vector>

#include <dbus/dbus.h>

#include "include/keyEvent.h"

#include "GKDBusMessage.h"

#include "lib/dbus/arguments/GKDBusArgString.h"
#include "lib/dbus/arguments/GKDBusArgUInt64.h"

#include "lib/dbus/messages/GKDBusAsyncContainer.h"

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
		private GKDBusArgumentUInt64,
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
		void appendMacroToReply(const GLogiK::macro_t & macro_array);

		void appendUInt64ToReply(const uint64_t value);

		void appendAsyncArgsToReply(void);

		void sendReply(void);
		void abandonReply(void);

	private:
		GKDBusReply* reply_;
};

} // namespace NSGKDBus

#endif
