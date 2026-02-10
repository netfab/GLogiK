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

#ifndef SRC_LIB_DBUS_GKDBUS_EVENTS_HPP_
#define SRC_LIB_DBUS_GKDBUS_EVENTS_HPP_

#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>

#include "lib/dbus/GKDBusConnection.hpp"

#include "events/callback.hpp"

namespace NSGKDBus
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
		typedef std::map<BusConnection,
					std::map<std::string, /* object path */
						std::map<std::string, /* interface */
							std::vector<DBusEvent*> > > > DBusEventsContainer;

	public:
		void declareIntrospectableSignal(
			const BusConnection eventBus,
			const std::string & eventObjectPath,
			const std::string & eventInterface,
			const std::string & eventName,
			const std::vector<DBusEventArgument> & args
		);

/*
		void removeMethod(
			const BusConnection eventBus,
			const std::string & eventObjectPath,
			const std::string & eventInterface,
			const std::string & eventName
		);
*/

		void removeInterface(
			const BusConnection bus,
			const std::string & objectPath,
			const std::string & interface
		) noexcept;

		void removeIntrospectableSignalsInterface(
			const BusConnection bus,
			const std::string & objectPath,
			const std::string & interface
		) noexcept;

	protected:
		GKDBusEvents(void) = default;
		~GKDBusEvents(void) = default;

		void clearDBusEvents(void) noexcept;
		const std::string getRootNodeIntrospection(void) noexcept;

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
		const std::string introspect(const std::string & askedObjectPath);

		/*
		void removeEvent(
			const BusConnection eventBus,
			const std::string & eventObjectPath,
			const std::string & eventInterface,
			const std::string & eventName
		);
		*/

		void exposeIntrospectMethod(
			const BusConnection eventBus,
			const std::string & eventObjectPath
		);

		void addEvent(
			const BusConnection eventBus,
			const std::string & eventObjectPath,
			const std::string & eventInterface,
			DBusEvent* event
		);

		const std::string buildSignalRuleMatch(
			const std::string & sender,
			const std::string & interface,
			const std::string & eventName
		) noexcept;

		void addSignalRuleMatch(
			const BusConnection eventBus,
			const std::string & sender,
			const std::string & interface,
			const std::string & eventName
		) noexcept;

		void removeSignalRuleMatch(
			const BusConnection eventBus,
			const std::string & sender,
			const std::string & interface,
			const std::string & eventName
		) noexcept;

		const bool findInterface(
			DBusEventsContainer & DBusEvents,
			const BusConnection bus,
			const std::string & objectPath,
			const std::string & interface
		) noexcept;

		void removeDBusEventsInterface(
			const BusConnection bus,
			const std::string & objectPath,
			const std::string & interface
		) noexcept;
};

} // namespace NSGKDBus

#endif
