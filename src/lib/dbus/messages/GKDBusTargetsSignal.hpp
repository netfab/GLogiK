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

#ifndef SRC_LIB_DBUS_MSG_GKDBUS_TARGETS_SIGNAL_HPP_
#define SRC_LIB_DBUS_MSG_GKDBUS_TARGETS_SIGNAL_HPP_

#include <cstdint>

#include <string>
#include <vector>

#include <dbus/dbus.h>

#include "GKDBusRemoteMethodCall.hpp"
#include "GKDBusBroadcastSignal.hpp"

namespace NSGKDBus
{

class GKDBusMessageTargetsSignal
	:	virtual private GKDBusMessageRemoteMethodCall,
		virtual private GKDBusArgumentString
{
	public:
		void initializeTargetsSignal(
			BusConnection wantedConnection,
			const char* destination,
			const char* objectPath,
			const char* interface,
			const char* signal
		);

		void appendStringToTargetsSignal(const std::string & value);
		void appendUInt8ToTargetsSignal(const uint8_t value);
		void appendStringVectorToTargetsSignal(
			const std::vector<std::string> & list
		);

		void sendTargetsSignal(void);
		void abandonTargetsSignal(void);

	protected:
		GKDBusMessageTargetsSignal() = default;
		~GKDBusMessageTargetsSignal() = default;

		void initializeTargetsSignal(
			DBusConnection* connection,
			const char* destination,
			const char* objectPath,
			const char* interface,
			const char* signal
		);

	private:
		std::vector<GKDBusBroadcastSignal*> _signals;

		virtual DBusConnection* getConnection(BusConnection wantedConnection) = 0;

};

} // namespace NSGKDBus

#endif

