/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_LIB_DBUS_EVENTS_GKDBUS_EVENT_TYPE_SIG_SSYT2B_HPP_
#define SRC_LIB_DBUS_EVENTS_GKDBUS_EVENT_TYPE_SIG_SSYT2B_HPP_

#include <cstdint>

#include <string>
#include <functional>

#include <dbus/dbus.h>

#include "callbackEvent.hpp"


/* two strings one byte one uint64_t to bool */
typedef std::function<
	const bool(
		const std::string&,
		const std::string&,
		const uint8_t,
		const uint64_t
	) > SIGssyt2b;



namespace NSGKDBus
{

template <>
	void callbackEvent<SIGssyt2b>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message,
		DBusMessage* asyncContainer
	);

} // namespace NSGKDBus

#endif
