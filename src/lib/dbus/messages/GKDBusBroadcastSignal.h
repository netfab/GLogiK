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

#ifndef __GLOGIK_GKDBUS_BROADCAST_SIGNAL_H__
#define __GLOGIK_GKDBUS_BROADCAST_SIGNAL_H__

#include <dbus/dbus.h>

#include "GKDBusMessage.h"

namespace NSGKDBus
{

class GKDBusBroadcastSignal : public GKDBusMessage
{
	public:
		GKDBusBroadcastSignal(
			DBusConnection* connection,		/* connection to send the signal on */
			const char* dest,			/* destination, if NULL, broadcast */
			const char* object_path,	/* the path to the object emitting the signal */
			const char* interface,		/* interface the signal is emitted from */
			const char* signal			/* name of signal */
		);
		~GKDBusBroadcastSignal();

	protected:
	private:

};

class GKDBusMessageBroadcastSignal
{
	public:

	protected:
		GKDBusMessageBroadcastSignal();
		~GKDBusMessageBroadcastSignal();

		void initializeBroadcastSignal(
			DBusConnection* connection,
			const char* object_path,
			const char* interface,
			const char* signal
		);
		void sendBroadcastSignal(void);

	private:
		GKDBusBroadcastSignal* signal_;

};

} // namespace NSGKDBus

#endif

