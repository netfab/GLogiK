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

#ifndef __GLOGIK_GKDBUS_MESSAGE_REPLY_H__
#define __GLOGIK_GKDBUS_MESSAGE_REPLY_H__

#include <dbus/dbus.h>

namespace GLogiK
{

class GKDBusMsgReply
{
	public:
		GKDBusMsgReply(DBusConnection* conn, DBusMessage* message);
		~GKDBusMsgReply();

		void appendToReply(const dbus_bool_t value);

	protected:
	private:
		DBusConnection* connection_;
		DBusMessage* reply_;
		DBusMessageIter rep_args_it_;

};

} // namespace GLogiK

#endif
