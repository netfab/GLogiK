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

#ifndef SRC_LIB_DBUS_EVENTS_CALLBACK_HPP_
#define SRC_LIB_DBUS_EVENTS_CALLBACK_HPP_

#include <new>
#include <vector>

#include "lib/utils/utils.hpp"

#include "GKDBusEvent.hpp"
#include "callbackEvent.hpp"

#include "lib/dbus/GKDBusConnection.hpp"

namespace NSGKDBus
{

template <typename T>
	class Callback
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
		Callback() = default;
		virtual ~Callback() = default;

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
	void Callback<T>::exposeMethod(
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
	void Callback<T>::exposeSignal(
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
	void Callback<T>::exposeEvent(
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
		event = new callbackEvent<T>(eventName, args, callback, eventType, introspectable);
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		throw NSGKUtils::GLogiKBadAlloc("DBus event bad allocation");
	}

	this->addEvent(bus, sender, object, interface, event);
}

} // namespace NSGKDBus

#endif

