/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2020  Fabrice Delliaux <netbox253@gmail.com>
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

#include "events/GKDBusEventTemplates.hpp"

namespace NSGKDBus
{


class GKDBusEvents
	:	public EventGKDBusCallback<VoidToVoid>,
		public EventGKDBusCallback<StringsArrayToVoid>,
		public EventGKDBusCallback<StringToBool>,
		public EventGKDBusCallback<StringToVoid>,
		public EventGKDBusCallback<TwoStringsToVoid>,
		public EventGKDBusCallback<TwoStringsToBool>,
		public EventGKDBusCallback<TwoStringsOneByteToBool>,
		public EventGKDBusCallback<TwoStringsThreeBytesToBool>,
		public EventGKDBusCallback<StringToString>,
		public EventGKDBusCallback<TwoStringsToString>,
		public EventGKDBusCallback<StringToStringsArray>,
		public EventGKDBusCallback<TwoStringsToStringsArray>,
		public EventGKDBusCallback<ThreeStringsOneByteToMacro>,
		public EventGKDBusCallback<TwoStringsToLCDPluginsPropertiesArray>,
		public EventGKDBusCallback<TwoStringsOneByteOneMacrosBankToBool>,
		public EventGKDBusCallback<TwoStringsOneByteOneUInt64ToBool>
{
	public:
		void declareIntrospectableSignal(
			const BusConnection bus,
			const char* object,
			const char* interface,
			const char* name,
			const std::vector<DBusMethodArgument> & args
		);

/*
		void removeMethod(
			const BusConnection eventBus,
			const char* eventObject,
			const char* eventInterface,
			const char* eventName
		);
*/

		void removeMethodsInterface(
			const BusConnection eventBus,
			const char* eventObject,
			const char* eventInterface
		) noexcept;

		void removeSignalsInterface(
			const BusConnection eventBus,
			const char* eventSender,
			const char* eventObject,
			const char* eventInterface
		) noexcept;

	protected:
		GKDBusEvents(
			const std::string & rootNode,
			const std::string & rootNodePath
		);
		~GKDBusEvents(void);

		thread_local static BusConnection currentBus;

		std::map< const BusConnection,
			std::map< const std::string, /* object */
				std::map< const std::string, /* interface */
					std::vector<GKDBusEvent*> > > > _DBusEvents;

		const std::string & getRootNode(void) const;

	private:
		static const std::string _rootNodeObject;
		std::string _rootNode;
		std::string _rootNodePath;
		std::set<std::string> _DBusInterfaces;

		std::map< const BusConnection,
			std::map< const std::string, /* object */
				std::map< const std::string, /* interface */
					std::vector<GKDBusIntrospectableSignal> > > > _DBusIntrospectableSignals;

		virtual DBusConnection* getConnection(BusConnection wantedConnection) = 0;

		void openXMLInterface(
			std::ostringstream & xml,
			bool & interfaceOpened,
			const std::string & interface
		);
		void eventToXMLMethod(
			std::ostringstream & xml,
			const GKDBusEvent* DBusEvent
		);
		const std::string introspect(const std::string & askedObjectPath);
		const std::string introspectRootNode(void);

		/*
		void removeEvent(
			const BusConnection eventBus,
			const char* eventObject,
			const char* eventInterface,
			const char* eventName
		);
		*/

		void addEvent(
			const BusConnection eventBus,
			const char* eventSender,
			const char* eventObject,
			const char* eventInterface,
			GKDBusEvent* event
		);

		const std::string buildSignalRuleMatch(
			const char* sender,
			const char* interface,
			const char* eventName
		) noexcept;

		void addSignalRuleMatch(
			const BusConnection eventBus,
			const char* sender,
			const char* interface,
			const char* eventName
		) noexcept;

		void removeSignalRuleMatch(
			const BusConnection eventBus,
			const char* sender,
			const char* interface,
			const char* eventName
		) noexcept;

		void removeInterface(
			const BusConnection eventBus,
			const char* sender,
			const char* eventObject,
			const char* eventInterface
		) noexcept;
};

} // namespace NSGKDBus

#endif
