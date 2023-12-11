/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_LIB_DBUS_MSG_GKDBUS_BROADCAST_SIGNAL_HPP_
#define SRC_LIB_DBUS_MSG_GKDBUS_BROADCAST_SIGNAL_HPP_

#include <dbus/dbus.h>

#include "include/base.hpp"

#include "lib/dbus/GKDBusConnection.hpp"

#include "GKDBusMessage.hpp"

namespace NSGKDBus
{

class GKDBusBroadcastSignal : public GKDBusMessage
{
	public:
		GKDBusBroadcastSignal(
			DBusConnection* const connection,	/* connection to send the signal on */
			const char* destination,	/* destination, if NULL, broadcast */
			const char* objectPath,		/* the path to the object emitting the signal */
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
		void initializeBroadcastSignal(
			BusConnection wantedConnection,
			const char* objectPath,
			const char* interface,
			const char* signal
		);

		void appendStringToBroadcastSignal(const std::string & value);
		void appendUInt8ToBroadcastSignal(const uint8_t value);
		void appendUInt16ToBroadcastSignal(const uint16_t value);
		void appendGKeysIDToBroadcastSignal(const GLogiK::GKeysID keyID);
		void appendMKeysIDToBroadcastSignal(const GLogiK::MKeysID bankID);
		void appendStringArrayToBroadcastSignal(
			const std::vector<std::string> & list
		);
		void appendMacroToBroadcastSignal(const GLogiK::macro_type & macro);

		void sendBroadcastSignal(void);
		void abandonBroadcastSignal(void);

	protected:
		GKDBusMessageBroadcastSignal();
		~GKDBusMessageBroadcastSignal();

		void initializeBroadcastSignal(
			DBusConnection* const connection,
			const char* objectPath,
			const char* interface,
			const char* signal
		);

	private:
		GKDBusBroadcastSignal* _signal;

		virtual DBusConnection* const getDBusConnection(BusConnection wantedConnection) const = 0;
};

} // namespace NSGKDBus

#endif

