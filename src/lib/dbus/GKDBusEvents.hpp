/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2026  Fabrice Delliaux <netbox253@gmail.com>
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

#pragma once

#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <dbus/dbus.h> // DBusConnection

// "s" - string
// "a" - array of
// "v" - void
// "b" - bool
// "y" - byte
// "t" - uint64_t
// "m" - M-KeyID
// "G" - G-KeyID
// "M" - Macro
// "B" - Bank (mBank_type)
// "P" - LCD Plugins Properties
// "D" - GKDepsMap_type

#include "events/callback.hpp"
#include "events/GKDBusEvent.hpp"

#include "events/SIGas2v.hpp"    //         array of string to void
#include "events/SIGb2v.hpp"     //                    bool to void
#include "events/SIGq2v.hpp"     //                uint16_t to void
#include "events/SIGs2as.hpp"    //       string to array of string
#include "events/SIGs2b.hpp"     //                  string to bool
#include "events/SIGs2D.hpp"     //        string to GKDepsMap_type
#include "events/SIGs2s.hpp"     //                string to string
#include "events/SIGs2v.hpp"     //                  string to void
#include "events/SIGsG2v.hpp"    // one string one G-KeyID to void
#include "events/SIGsGM2v.hpp"   // one string one G-KeyID one macro to void
#include "events/SIGsm2v.hpp"    // one string one M-KeyID to void
#include "events/SIGss2aG.hpp"   //  two strings to array of G-KeyID
#include "events/SIGss2am.hpp"   //  two strings to array of M-KeyID
#include "events/SIGss2aP.hpp"   //  two strings to array of LCD Plugins Properties
#include "events/SIGss2b.hpp"    //             two strings to bool
#include "events/SIGss2s.hpp"    //           two strings to string
#include "events/SIGss2v.hpp"    //             two strings to void
#include "events/SIGssyt2b.hpp"  // two strings one byte one uint64_t to bool
#include "events/SIGssyyy2b.hpp" // two strings three bytes to bool
#include "events/SIGv2v.hpp"     //                    void to void

#include "GKDBusConnection.hpp"

namespace GKDBUS::Events
{

class GKDBusEvents
	:	public Callback<SIGas2v>,
		public Callback<SIGb2v>,
		public Callback<SIGq2v>,
		public Callback<SIGs2as>,
		public Callback<SIGs2b>,
		public Callback<SIGs2D>,
		public Callback<SIGs2s>,
		public Callback<SIGs2v>,
		public Callback<SIGsG2v>,
		public Callback<SIGsGM2v>,
		public Callback<SIGsm2v>,
		public Callback<SIGss2aG>,
		public Callback<SIGss2am>,
		public Callback<SIGss2aP>,
		public Callback<SIGss2b>,
		public Callback<SIGss2s>,
		public Callback<SIGss2v>,
		public Callback<SIGssyt2b>,
		public Callback<SIGssyyy2b>,
		public Callback<SIGv2v>
{
	private:
		using BusConnection = GKDBUS::BusConnection;

		typedef std::map<BusConnection,
					std::map<std::string, /* object path */
						std::map<std::string, /* interface */
							std::vector<DBusEvent*> > > > DBusEventsContainer;

	public:
		void declareIntrospectableSignal(
			BusConnection eventBus,
			const std::string & eventObjectPath,
			const std::string & eventInterface,
			const std::string & eventName,
			const std::vector<DBusEventArgument> & args
		);

/*
		void removeMethod(
			BusConnection eventBus,
			const std::string & eventObjectPath,
			const std::string & eventInterface,
			const std::string & eventName
		);
*/

		void removeInterface(
			BusConnection bus,
			const std::string & objectPath,
			const std::string & interface
		) noexcept;

		void removeIntrospectableSignalsInterface(
			BusConnection bus,
			const std::string & objectPath,
			const std::string & interface
		) noexcept;

	protected:
		GKDBusEvents() = default;
		~GKDBusEvents() = default;

		void clearDBusEvents() noexcept;
		std::string getRootNodeIntrospection() noexcept;

		DBusEventsContainer _DBusEvents;

	private:
		DBusEventsContainer _DBusIntrospectableSignals;

		std::set<std::string> _DBusInterfaces;

	protected:
		thread_local static BusConnection currentBus;

	private:
		const std::string _FREEDESKTOP_DBUS_INTROSPECTABLE_STANDARD_INTERFACE =
			"org.freedesktop.DBus.Introspectable";
		virtual DBusConnection* const getDBusConnection(BusConnection wantedConnection) const = 0;

		void openXMLInterface(
			std::ostringstream & xml,
			bool & interfaceOpened,
			const std::string & interface
		);
		void closeXMLInterface(
			std::ostringstream & xml,
			bool & interfaceOpened
		);
		void eventToXML(
			std::ostringstream & xml,
			const DBusEvent* event
		);
		std::string introspect(const std::string & askedObjectPath);

		/*
		void removeEvent(
			BusConnection eventBus,
			const std::string & eventObjectPath,
			const std::string & eventInterface,
			const std::string & eventName
		);
		*/

		void exposeIntrospectMethod(
			BusConnection eventBus,
			const std::string & eventObjectPath
		);

		void addEvent(
			BusConnection eventBus,
			const std::string & eventObjectPath,
			const std::string & eventInterface,
			DBusEvent* event
		);

		std::string buildSignalRuleMatch(
			const std::string & sender,
			const std::string & interface,
			const std::string & eventName
		) noexcept;

		void addSignalRuleMatch(
			BusConnection eventBus,
			const std::string & sender,
			const std::string & interface,
			const std::string & eventName
		) noexcept;

		void removeSignalRuleMatch(
			BusConnection eventBus,
			const std::string & sender,
			const std::string & interface,
			const std::string & eventName
		) noexcept;

		bool findInterface(
			DBusEventsContainer & DBusEvents,
			BusConnection bus,
			const std::string & objectPath,
			const std::string & interface
		) noexcept;

		void removeDBusEventsInterface(
			BusConnection bus,
			const std::string & objectPath,
			const std::string & interface
		) noexcept;
};

} // namespace GKDBUS::Events
