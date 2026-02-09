/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2025  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_LIB_DBUS_EVENTS_CALLBACK_EVENT_HPP_
#define SRC_LIB_DBUS_EVENTS_CALLBACK_EVENT_HPP_

#include <string>
#include <vector>

#include <dbus/dbus.h>

#include "lib/utils/utils.hpp"

#include "GKDBusEvent.hpp"

#include "lib/dbus/ArgTypes/string.hpp"
#include "lib/dbus/ArgTypes/uint64.hpp"
#include "lib/dbus/messages/GKDBusReply.hpp"
#include "lib/dbus/messages/GKDBusErrorReply.hpp"

namespace NSGKDBus
{

/* -- -- -- -- -- -- -- -- -- -- -- */
/* -- -- -- class template -- -- -- */
/* -- -- -- -- -- -- -- -- -- -- -- */

template <typename T>
	class callbackEvent
		:	public GKDBusEvent,
			private GKDBusMessageReply,
			private GKDBusMessageErrorReply,
			virtual private ArgString,
			virtual private ArgUInt64
{
	public:
		callbackEvent(
			const std::string & name,
			const std::string & sender,
			const std::vector<DBusEventArgument> & args,
			T cb,
			DBusEventType type,
			const bool intr
		);
		~callbackEvent() = default;

	private:
		callbackEvent() = delete;

		T callback;

		void sendReplyError(
			DBusConnection* const connection,
			DBusMessage* message,
			const std::string & errorMessage
		);

		void sendCallbackError(
			DBusConnection* const connection,
			DBusMessage* message,
			const std::string & errorMessage
		);

		void runCallback(
			DBusConnection* const connection,
			DBusMessage* message,
			DBusMessage* asyncContainer
		);
};

/* -- -- -- -- -- -- -- -- -- -- -- -- */
/* -- -- --  implementations  -- -- -- */
/* -- -- -- -- -- -- -- -- -- -- -- -- */

template <typename T>
	callbackEvent<T>::callbackEvent(
		const std::string & name,
		const std::string & sender,
		const std::vector<DBusEventArgument> & args,
		T cb,
		DBusEventType type,
		const bool intr
	)		:	GKDBusEvent(name, sender, args, type, intr),
				callback(cb)
{
}

/*
 * exception was thrown while building reply
 */
template <typename T>
	void callbackEvent<T>::sendReplyError(
		DBusConnection* const connection,
		DBusMessage* message,
		const std::string & errorMessage)
{
	GK_LOG_FUNC

	using namespace NSGKUtils;
	LOG(error) << "DBus reply failure : " << errorMessage;
	this->abandonReply();	/* delete reply object if allocated */
	this->buildAndSendErrorReply(connection, message, errorMessage.c_str());
}

/*
 * exception was thrown before or while running callback
 */
template <typename T>
	void callbackEvent<T>::sendCallbackError(
		DBusConnection* const connection,
		DBusMessage* message,
		const std::string & errorMessage)
{
	GK_LOG_FUNC

	using namespace NSGKUtils;
	LOG(error) << errorMessage;

	if(this->eventType != DBusEventType::DBUS_SIGNAL_EVENT)
	{ /* send error if something was wrong when running callback */
		this->buildAndSendErrorReply(connection, message, errorMessage.c_str());
	}
}

template <typename T>
	void callbackEvent<T>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message,
		DBusMessage* asyncContainer)
{
	GK_LOG_FUNC

	using namespace NSGKUtils;
	const std::string & errorMessage = "runCallback not implemented";
	LOG(error) << errorMessage;
	if(this->eventType != DBusEventType::DBUS_SIGNAL_EVENT)
		this->buildAndSendErrorReply(connection, message, errorMessage.c_str());
}

} // namespace NSGKDBus

#endif

