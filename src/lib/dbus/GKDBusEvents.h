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
#include <functional>

namespace GLogiK
{

struct GKDBusEventStringToBoolCallback {
	const std::string method;
	std::function<const bool(const std::string&)> callback;

	GKDBusEventStringToBoolCallback(const char* m, std::function<const bool(const std::string&)> c) : method(m), callback(c) {}
};

struct GKDBusEventVoidToStringCallback {
	const std::string method;
	std::function<const std::string(void)> callback;

	GKDBusEventVoidToStringCallback(const char* m, std::function<const std::string(void)> c) : method(m), callback(c) {}
};

class GKDBusEvents
{
	public:
		GKDBusEvents();
		~GKDBusEvents(void);

		void addEventStringToBoolCallback(const char* interface, const char* method, std::function<const bool(const std::string&)> callback);
		void addEventVoidToStringCallback(const char* interface, const char* method, std::function<const std::string(void)> callback);

	protected:
		std::map<const std::string, std::vector<GKDBusEventStringToBoolCallback>> events_string_to_bool_;
		std::map<const std::string, std::vector<GKDBusEventVoidToStringCallback>> events_void_to_string_;

	private:
		const std::string introspect(void);

};

} // namespace GLogiK

#endif
