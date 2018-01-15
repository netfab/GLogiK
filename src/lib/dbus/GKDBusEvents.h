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

#ifndef __GLOGIK_GKDBUS_EVENTS_H__
#define __GLOGIK_GKDBUS_EVENTS_H__

#include <string>
#include <vector>
#include <map>
#include <sstream>

#include "lib/dbus/GKDBusConnection.h"
#include "lib/shared/keyEvent.h"

#include "events/GKDBusEventVoidToVoid.h"
//#include "events/GKDBusEventStringToVoid.h"
#include "events/GKDBusEventStringToBool.h"
#include "events/GKDBusEventTwoStringsToBool.h"
#include "events/GKDBusEventStringToString.h"
#include "events/GKDBusEventTwoStringsToString.h"
#include "events/GKDBusEventStringToStringsArray.h"
#include "events/GKDBusEventTwoStringsToStringsArray.h"
#include "events/GKDBusEventTwoStringsOneByteToBool.h"
#include "events/GKDBusEventTwoStringsThreeBytesToBool.h"
#include "events/GKDBusEventThreeStringsOneByteToMacro.h"
#include "events/GKDBusEventStringsArrayToVoid.h"

namespace NSGKDBus
{


class GKDBusEvents
	:	public EventVoidToVoid,
		public EventStringToBool,
		public EventTwoStringsToBool,
		public EventStringToString,
		public EventTwoStringsToString,
		public EventStringToStringsArray,
		public EventTwoStringsToStringsArray,
		public EventTwoStringsOneByteToBool,
		public EventTwoStringsThreeBytesToBool,
		public EventThreeStringsOneByteToMacro,
		public EventStringsArrayToVoid
{
	public:


	protected:
		GKDBusEvents(const std::string & rootnode);
		~GKDBusEvents(void);

		thread_local static BusConnection current_bus_;

		std::map< const BusConnection,
			std::map< const std::string, /* object path */
				std::map< const std::string, /* interface */
					std::vector<GKDBusEvent*> > > > DBusEvents_;

		std::map<const std::string, bool> DBusObjects_;
		std::map<const std::string, bool> DBusInterfaces_;

		const std::string getNode(const std::string & object) const;
		const std::string & getRootNode(void) const;

	private:
		static const std::string rootNodeObject_;
		std::string root_node_;

		void openXMLInterface(
			std::ostringstream & xml,
			bool & interface_opened,
			const std::string & interface
		);
		void eventToXMLMethod(
			std::ostringstream & xml,
			const GKDBusEvent* DBusEvent
		);
		const std::string introspect(const std::string & asked_object_path);
		const std::string introspectRootNode(void);

		void addIntrospectableEvent(
			const BusConnection bus,
			const char* object,
			const char* interface,
			GKDBusEvent* event
		);
};

} // namespace NSGKDBus

#endif
