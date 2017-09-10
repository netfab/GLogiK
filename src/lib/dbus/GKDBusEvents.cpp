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

#include "GKDBusEvents.h"
#include "include/log.h"

namespace GLogiK
{

GKDBusEvents::GKDBusEvents() {
	LOG(DEBUG2) << __func__ << " construction";
	this->addEventVoidToStringCallback("org.freedesktop.DBus.Introspectable", "Introspect", std::bind(&GKDBusEvents::introspect, this));
}

GKDBusEvents::~GKDBusEvents() {
	LOG(DEBUG2) << __func__ << " destruction";
}

void GKDBusEvents::addEventStringToBoolCallback(const char* interface, const char* method, std::function<const bool(const std::string&)> callback) {
	GKDBusEventStringToBoolCallback e(method, callback);
	this->events_string_to_bool_[interface].push_back(e);
}

void GKDBusEvents::addEventVoidToStringCallback(const char* interface, const char* method, std::function<const std::string(void)> callback) {
	GKDBusEventVoidToStringCallback e(method, callback);
	this->events_void_to_string_[interface].push_back(e);
}

const std::string GKDBusEvents::introspect(void) {
	LOG(DEBUG3) << __func__ << " call !";
	const std::string ok("ok");
	return ok;
}

} // namespace GLogiK

