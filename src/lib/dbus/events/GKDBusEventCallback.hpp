/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2022  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_LIB_DBUS_EVENTS_GKDBUS_EVENT_CALLBACK_HPP_
#define SRC_LIB_DBUS_EVENTS_GKDBUS_EVENT_CALLBACK_HPP_

#include <vector>

#include <dbus/dbus.h>

#include "lib/utils/utils.hpp"

#include "GKDBusEvent.hpp"

#include "lib/dbus/arguments/GKDBusArgString.hpp"
#include "lib/dbus/arguments/GKDBusArgByte.hpp"
#include "lib/dbus/arguments/GKDBusArgUInt64.hpp"

#include "lib/dbus/arguments/GKDBusArgGKeysID.hpp"
#include "lib/dbus/arguments/GKDBusArgMKeysID.hpp"
#include "lib/dbus/arguments/GKDBusArgMacrosBank.hpp"

namespace NSGKDBus
{

/* -- -- -- -- -- -- -- -- -- -- -- */
/* -- -- -- class template -- -- -- */
/* -- -- -- -- -- -- -- -- -- -- -- */

template <typename T>
	class GKDBusEventCallback
		:	public GKDBusEvent,
			virtual private GKDBusArgumentString,
			virtual private GKDBusArgumentByte,
			virtual private GKDBusArgumentUInt64,
			private GKDBusArgumentGKeysID,
			private GKDBusArgumentMKeysID,
			private GKDBusArgumentMacrosBank
{
	public:
		GKDBusEventCallback(
			const char* n,
			const std::vector<DBusMethodArgument> & a,
			T c,
			GKDBusEventType t,
			const bool i
			);
		~GKDBusEventCallback() = default;

		void runCallback(
			DBusConnection* const connection,
			DBusMessage* message
		);

	private:
		GKDBusEventCallback() = delete;

		T callback;
};

/* -- -- -- -- -- -- -- -- -- -- -- -- */
/* -- -- --  implementations  -- -- -- */
/* -- -- -- -- -- -- -- -- -- -- -- -- */

template <typename T>
	GKDBusEventCallback<T>::GKDBusEventCallback(
		const char* n,
		const std::vector<DBusMethodArgument> & a,
		T c,
		GKDBusEventType t,
		const bool i
	)		: GKDBusEvent(n, a, t, i), callback(c)
{
}

template <typename T>
	void GKDBusEventCallback<T>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message)
{
	GK_LOG_FUNC

	using namespace NSGKUtils;
	const char* errorString = "runCallback not implemented";
	LOG(error) << errorString;
	if(this->eventType != GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		this->buildAndSendErrorReply(connection, message, errorString);
}

} // namespace NSGKDBus

#endif
