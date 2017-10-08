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

#ifndef __GLOGIK_GKDBUS_MESSAGE_ERROR_REPLY_H__
#define __GLOGIK_GKDBUS_MESSAGE_ERROR_REPLY_H__

#if !defined (GKDBUS_INSIDE_GKDBUS_H) && !defined (GKDBUS_COMPILATION)
#error "Only "dbus/GKDBus.h" can be included directly, this file may disappear or change contents."
#endif

#include <dbus/dbus.h>

#include "GKDBusMessage.h"

namespace GLogiK
{

class GKDBusMessageErrorReply : public GKDBusMessage
{
	public:
		GKDBusMessageErrorReply(DBusConnection* conn, DBusMessage* message);
		~GKDBusMessageErrorReply();

	protected:
	private:

};

} // namespace GLogiK

#endif