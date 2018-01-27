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

#ifndef __GLOGIK_GKDBUS_EVENT_TEMPLATES_H__
#define __GLOGIK_GKDBUS_EVENT_TEMPLATES_H__

#include <cstdint>

#include <string>
#include <vector>
#include <functional>

#include <dbus/dbus.h>

#include "lib/utils/utils.h"
#include "lib/shared/keyEvent.h"

/* -- */
#include "GKDBusEvent.h"

#include "lib/dbus/GKDBusConnection.h"
#include "lib/dbus/arguments/GKDBusArgString.h"
#include "lib/dbus/arguments/GKDBusArgByte.h"
#include "lib/dbus/arguments/GKDBusArgMacrosBank.h"

#include "lib/dbus/messages/GKDBusReply.h"
#include "lib/dbus/messages/GKDBusErrorReply.h"



/* -- -- -- -- -- -- -- -- -- -- -- -- */
/* -- -- -- --  typedefs   -- -- -- -- */
/* -- -- -- -- -- -- -- -- -- -- -- -- */

typedef std::function<
			void(
				void
			) > VoidToVoid;

typedef std::function<
			const bool(
				const std::string&
			) > StringToBool;

typedef std::function<
			const bool(
				const std::string&,
				const std::string&
			) > TwoStringsToBool;

typedef std::function<
			const std::string(
				const std::string&
			) > StringToString;

typedef std::function<
			const std::string(
				const std::string&,
				const std::string&
			) > TwoStringsToString;

typedef std::function<
			const std::vector<std::string>(
				const std::string&
			) > StringToStringsArray;

typedef std::function<
			const std::vector<std::string>(
				const std::string&,
				const std::string&
			) > TwoStringsToStringsArray;

typedef std::function<
			const bool(
				const std::string&,
				const std::string&,
				const uint8_t
			) > TwoStringsOneByteToBool;

typedef std::function<
			const bool(
				const std::string&,
				const std::string&,
				const uint8_t,
				const uint8_t,
				const uint8_t
			) > TwoStringsThreeBytesToBool;

typedef std::function<
			void(
				const std::vector<std::string> &
			) > StringsArrayToVoid;

typedef std::function<
			const GLogiK::macro_t &(
				const std::string&,
				const std::string&,
				const std::string&,
				const uint8_t
			) > ThreeStringsOneByteToMacro;

typedef std::function<
			const bool(
				const std::string&,
				const std::string&,
				const uint8_t,
				const GLogiK::macros_bank_t &
			) > TwoStringsOneByteOneMacrosBankToBool;


/* -- -- -- -- -- -- -- -- -- -- -- -- */
/* -- -- -- classes templates -- -- -- */
/* -- -- -- -- -- -- -- -- -- -- -- -- */


namespace NSGKDBus
{

template <typename T>
	class GKDBusCallbackEvent
		:	public GKDBusEvent,
			virtual public GKDBusArgumentString,
			virtual public GKDBusArgumentByte,
			public GKDBusArgumentMacrosBank,
			public GKDBusMessageReply,
			public GKDBusMessageErrorReply
{
	public:
		GKDBusCallbackEvent(
			const char* n,
			const std::vector<DBusMethodArgument> & a,
			T c,
			GKDBusEventType t,
			const bool i
			);
		~GKDBusCallbackEvent() = default;

		void runCallback(DBusConnection* connection, DBusMessage* message);

	private:
		GKDBusCallbackEvent() = delete;

		T callback;
};

template <typename T>
	class EventGKDBusCallback
{
	public:
		void addEvent(
			const BusConnection bus,
			const char* object,
			const char* interface,
			const char* eventName,
			const std::vector<DBusMethodArgument> & args,
			T callback,
			GKDBusEventType t=GKDBusEventType::GKDBUS_EVENT_METHOD,
			const bool introspectable=true
		);

		void addSignal(
			const BusConnection bus,
			const char* object,
			const char* interface,
			const char* eventName,
			const std::vector<DBusMethodArgument> & args,
			T callback
		);

	protected:
		EventGKDBusCallback() = default;
		virtual ~EventGKDBusCallback() = default;

		virtual void addIntrospectableEvent(
			const BusConnection bus,
			const char* object,
			const char* interface,
			GKDBusEvent* event
		) = 0;

	private:
		static void addSignalRuleMatch(
			DBusConnection* connection,
			const char* interface,
			const char* eventName
		);

		virtual DBusConnection* getConnection(BusConnection wanted_connection) = 0;
};

/* -- -- -- -- -- -- -- -- -- -- -- -- */
/* -- -- --  implementations  -- -- -- */
/* -- -- -- -- -- -- -- -- -- -- -- -- */

template <typename T>
	GKDBusCallbackEvent<T>::GKDBusCallbackEvent(
		const char* n,
		const std::vector<DBusMethodArgument> & a,
		T c,
		GKDBusEventType t,
		const bool i
	)		: GKDBusEvent(n, a, t, i), callback(c)
{
}

template <typename T>
	void GKDBusCallbackEvent<T>::runCallback(
		DBusConnection* connection,
		DBusMessage* message)
{
	// FIXME
}

template <typename T>
	void EventGKDBusCallback<T>::addEvent(
		const BusConnection bus,
		const char* object,
		const char* interface,
		const char* eventName,
		const std::vector<DBusMethodArgument> & args,
		T callback,
		GKDBusEventType eventType,
		const bool introspectable
	)	{
			GKDBusEvent* e = new GKDBusCallbackEvent<T>(eventName, args, callback, eventType, introspectable);
			this->addIntrospectableEvent(bus, object, interface, e);
		}

template <typename T>
	void EventGKDBusCallback<T>::addSignal(
		const BusConnection bus,
		const char* object,
		const char* interface,
		const char* eventName,
		const std::vector<DBusMethodArgument> & args,
		T callback
	)	{
	this->addEvent(bus, object, interface, eventName, args, callback, GKDBusEventType::GKDBUS_EVENT_SIGNAL, true);
	DBusConnection* connection = this->getConnection(bus);
	this->addSignalRuleMatch(connection, interface, eventName);
}

template <typename T>
void EventGKDBusCallback<T>::addSignalRuleMatch(
	DBusConnection* connection,
	const char* interface,
	const char* eventName
) {
	std::string rule = "type='signal',interface='";
	rule += interface;
	rule += "',member='";
	rule += eventName;
	rule += "'";
	dbus_bus_add_match(connection, rule.c_str(), nullptr);
	dbus_connection_flush(connection);
	// FIXME check error
	//this->checkDBusError("DBus Session match error");
#if DEBUGGING_ON
	using namespace NSGKUtils;
	LOG(DEBUG1) << "DBus Signal match rule sent : " << rule;
#endif
}

/* -- -- -- -- -- -- -- -- -- -- -- -- */
/* --   template member functions   -- */
/* --		specialization          -- */
/* -- -- -- -- -- -- -- -- -- -- -- -- */

template <>
	void GKDBusCallbackEvent<VoidToVoid>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	);

template <>
	void GKDBusCallbackEvent<StringToBool>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	);

template <>
	void GKDBusCallbackEvent<TwoStringsToBool>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	);

template <>
	void GKDBusCallbackEvent<StringToString>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	);

template <>
	void GKDBusCallbackEvent<TwoStringsToString>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	);

template <>
	void GKDBusCallbackEvent<StringToStringsArray>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	);

template <>
	void GKDBusCallbackEvent<TwoStringsToStringsArray>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	);

template <>
	void GKDBusCallbackEvent<TwoStringsOneByteToBool>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	);

template <>
	void GKDBusCallbackEvent<TwoStringsThreeBytesToBool>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	);

template <>
	void GKDBusCallbackEvent<StringsArrayToVoid>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	);

template <>
	void GKDBusCallbackEvent<ThreeStringsOneByteToMacro>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	);

template <>
	void GKDBusCallbackEvent<TwoStringsOneByteOneMacrosBankToBool>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	);

} // namespace NSGKDBus

#endif
