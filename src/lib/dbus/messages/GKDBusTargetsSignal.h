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

#ifndef __GLOGIK_GKDBUS_TARGETS_SIGNAL_H__
#define __GLOGIK_GKDBUS_TARGETS_SIGNAL_H__

#include <cstdint>

#include <string>
#include <vector>

#include <dbus/dbus.h>

#include "GKDBusRemoteMethodCall.h"
#include "GKDBusBroadcastSignal.h"

namespace GLogiK
{

class GKDBusMessageTargetsSignal
	:	virtual public GKDBusMessageRemoteMethodCall
{
	public:
		void initializeTargetsSignal(
			BusConnection wanted_connection,
			const char* dest,
			const char* object_path,
			const char* interface,
			const char* signal
		);

		void appendStringToTargetsSignal(const std::string & value);
		void appendUInt8ToTargetsSignal(const uint8_t value);

		void sendTargetsSignal(void);

	protected:
		GKDBusMessageTargetsSignal() = default;
		~GKDBusMessageTargetsSignal() = default;

		void initializeTargetsSignal(
			DBusConnection* connection,
			const char* dest,
			const char* object_path,
			const char* interface,
			const char* signal
		);

	private:
		std::vector<GKDBusBroadcastSignal*> signals_;

		virtual DBusConnection* getConnection(BusConnection wanted_connection) = 0;

};

} // namespace GLogiK

#endif

