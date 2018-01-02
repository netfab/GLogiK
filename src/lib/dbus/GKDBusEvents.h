/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2017  Fabrice Delliaux <netbox253@gmail.com>
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

#include "lib/shared/keyEvent.h"

#include "events/GKDBusEventStringToBool.h"
#include "events/GKDBusEventTwoStringsToBool.h"
#include "events/GKDBusEventStringToString.h"
#include "events/GKDBusEventStringToStringsArray.h"
#include "events/GKDBusEventTwoStringsToStringsArray.h"
#include "events/GKDBusEventTwoStringsThreeBytesToBool.h"

namespace GLogiK
{


class GKDBusEvents
	:	public EventStringToBool,
		public EventTwoStringsToBool,
		public EventStringToString,
		public EventStringToStringsArray,
		public EventTwoStringsToStringsArray,
		public EventTwoStringsThreeBytesToBool
{
	public:


	protected:
		GKDBusEvents(void);
		~GKDBusEvents(void);

		std::map< const std::string, /* object path */
			std::map< const std::string, /* interface */
				std::vector<GKDBusEvent*>>> DBusEvents_;

		std::map<const std::string, bool> DBusObjects_;
		std::map<const std::string, bool> DBusInterfaces_;

		void defineRootNode(const std::string& rootnode);
		const std::string getNode(const std::string & object);

	private:
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

		void addIntrospectableEvent(const char* object, const char* interface, GKDBusEvent* event);
};

} // namespace GLogiK

#endif
