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
		public EventGKDBusCallback<TwoStringsToVoid>,
		public EventGKDBusCallback<TwoStringsToBool>,
		public EventGKDBusCallback<TwoStringsOneByteToBool>,
		public EventGKDBusCallback<TwoStringsThreeBytesToBool>,
		public EventGKDBusCallback<StringToString>,
		public EventGKDBusCallback<TwoStringsToString>,
		public EventGKDBusCallback<StringToStringsArray>,
		public EventGKDBusCallback<TwoStringsToStringsArray>,
		public EventGKDBusCallback<ThreeStringsOneByteToMacro>,
		public EventGKDBusCallback<TwoStringsOneByteOneMacrosBankToBool>
{
	public:

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

		void addIntrospectableEvent(
			const BusConnection eventBus,
			const char* eventObject,
			const char* eventInterface,
			GKDBusEvent* event
		);
};

} // namespace NSGKDBus

#endif
