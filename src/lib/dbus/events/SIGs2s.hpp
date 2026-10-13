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

#include <string>
#include <functional>

#include <dbus/dbus.h>

#include "callbackEvent.hpp"


typedef std::function<std::string(const std::string &)> SIGs2s; /* string to string */

namespace GKDBUS::Events
{

template <>
	void callbackEvent<SIGs2s>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message,
		DBusMessage* asyncContainer
	);

} // namespace GKDBUS::Events
