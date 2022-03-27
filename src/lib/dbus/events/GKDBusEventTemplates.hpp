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

#ifndef SRC_LIB_DBUS_EVENTS_GKDBUS_EVENT_TEMPLATES_HPP_
#define SRC_LIB_DBUS_EVENTS_GKDBUS_EVENT_TEMPLATES_HPP_

#include <cstdint>

#include <new>
#include <string>
#include <vector>
#include <functional>

#include <dbus/dbus.h>

#include "lib/utils/utils.hpp"

#include "include/keyEvent.hpp"
#include "include/LCDPluginProperties.hpp"

/* -- */
#include "GKDBusEvent.hpp"
#include "GKDBusEventCallback.hpp"

#include "lib/dbus/GKDBusConnection.hpp"

// "s" - string
// "a" - array of
// "v" - void
// "b" - bool
// "y" - byte
// "m" - M-KeyID
// "G" - G-KeyID
// "M" - Macro
// "B" - Bank

#include "SIGas2v.hpp"    //         array of string to void
#include "SIGs2as.hpp"    //       string to array of string
#include "SIGs2b.hpp"     //                  string to bool
#include "SIGs2s.hpp"     //                string to string
#include "SIGs2v.hpp"     //                  string to void
#include "SIGsmG2b.hpp"   // one string one M-KeyID one G-KeyID to bool
#include "SIGss2aG.hpp"   //  two strings to array of G-KeyID
#include "SIGss2as.hpp"   //  two strings to array of string
#include "SIGss2b.hpp"    //             two strings to bool
#include "SIGss2s.hpp"    //           two strings to string
#include "SIGss2v.hpp"    //             two strings to void
#include "SIGssBG2M.hpp"  // two strings one M-KeyID one G-KeyID to macro
#include "SIGssm2b.hpp"   // two strings one M-KeyID to bool
#include "SIGssyyy2b.hpp" // two strings three bytes to bool
#include "SIGv2v.hpp"     //                    void to void

/* -- -- -- -- -- -- -- -- -- -- -- -- */
/* -- -- -- --  typedefs   -- -- -- -- */
/* -- -- -- -- -- -- -- -- -- -- -- -- */

/* TODO currently same signature as ResetDeviceMacrosBank
typedef std::function<
			const bool(
				const std::string&,
				const std::string&,
				const GLogiK::MKeysID
			) > DeviceMacroChanged;
*/

typedef std::function<
			const GLogiK::LCDPluginsPropertiesArray_type &(
				const std::string&,
				const std::string&
			) > TwoStringsToLCDPluginsPropertiesArray;

typedef std::function<
			const bool(
				const std::string&,
				const std::string&,
				const GLogiK::MKeysID,
				const GLogiK::mBank_type &
			) > SetDeviceMacrosBank;

typedef std::function<
			const bool(
				const std::string&,
				const std::string&,
				const uint8_t,
				const uint64_t
			) > TwoStringsOneByteOneUInt64ToBool;


/* -- -- -- -- -- -- -- -- -- -- -- -- */
/* -- -- -- classes templates -- -- -- */
/* -- -- -- -- -- -- -- -- -- -- -- -- */

namespace NSGKDBus
{



template <typename T>
	class EventGKDBusCallback
{
	public:
		void exposeMethod(
			const BusConnection bus,
			const char* object,
			const char* interface,
			const char* eventName,
			const std::vector<DBusMethodArgument> & args,
			T callback
		);

		void exposeSignal(
			const BusConnection bus,
			const char* sender,
			const char* object,
			const char* interface,
			const char* eventName,
			const std::vector<DBusMethodArgument> & args,
			T callback
		);

	protected:
		EventGKDBusCallback() = default;
		virtual ~EventGKDBusCallback() = default;

		void exposeEvent(
			const BusConnection bus,
			const char* sender,
			const char* object,
			const char* interface,
			const char* eventName,
			const std::vector<DBusMethodArgument> & args,
			T callback,
			GKDBusEventType t,
			const bool introspectable
		);

	private:
		virtual void addEvent(
			const BusConnection bus,
			const char* sender,
			const char* object,
			const char* interface,
			GKDBusEvent* event
		) = 0;
};

/* -- -- -- -- -- -- -- -- -- -- -- -- */
/* -- -- --  implementations  -- -- -- */
/* -- -- -- -- -- -- -- -- -- -- -- -- */

template <typename T>
	void EventGKDBusCallback<T>::exposeMethod(
		const BusConnection bus,
		const char* object,
		const char* interface,
		const char* eventName,
		const std::vector<DBusMethodArgument> & args,
		T callback
	)
{
	this->exposeEvent(bus, nullptr, object, interface, eventName, args, callback, GKDBusEventType::GKDBUS_EVENT_METHOD, true);
}

template <typename T>
	void EventGKDBusCallback<T>::exposeSignal(
		const BusConnection bus,
		const char* sender,
		const char* object,
		const char* interface,
		const char* eventName,
		const std::vector<DBusMethodArgument> & args,
		T callback
	)
{
	/* signals declared as events with callback functions are not introspectable */
	this->exposeEvent(bus, sender, object, interface, eventName, args, callback, GKDBusEventType::GKDBUS_EVENT_SIGNAL, false);
}

template <typename T>
	void EventGKDBusCallback<T>::exposeEvent(
		const BusConnection bus,
		const char* sender,
		const char* object,
		const char* interface,
		const char* eventName,
		const std::vector<DBusMethodArgument> & args,
		T callback,
		GKDBusEventType eventType,
		const bool introspectable
	)
{
	GKDBusEvent* event = nullptr;
	try {
		event = new GKDBusEventCallback<T>(eventName, args, callback, eventType, introspectable);
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		throw NSGKUtils::GLogiKBadAlloc("DBus event bad allocation");
	}

	this->addEvent(bus, sender, object, interface, event);
}

/* -- -- -- -- -- -- -- -- -- -- -- -- */
/* --   template member functions   -- */
/* --		specialization          -- */
/* -- -- -- -- -- -- -- -- -- -- -- -- */

/* TODO currently same signature as ResetDeviceMacrosBank
template <>
	void GKDBusEventCallback<DeviceMacroChanged>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	);
*/

template <>
	void GKDBusEventCallback<TwoStringsToLCDPluginsPropertiesArray>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	);

template <>
	void GKDBusEventCallback<SetDeviceMacrosBank>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	);

template <>
	void GKDBusEventCallback<TwoStringsOneByteOneUInt64ToBool>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	);

} // namespace NSGKDBus

#endif
