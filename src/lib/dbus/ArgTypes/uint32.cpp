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

#include <dbus/dbus.h>

#include "lib/utils/utils.hpp"

#include "uint32.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

void TypeUInt32::appendUInt32(const uint32_t value)
{
	GK_LOG_FUNC

	dbus_uint32_t v = value;
	if( ! dbus_message_iter_append_basic(&_itMessage, DBUS_TYPE_UINT32, &v) ) {
		_hosedMessage = true;
		throw GKDBusMessageWrongBuild("uint32_t append failure, not enough memory");
	}

#if DEBUG_GKDBUS
	GKLog(trace, "uint32_t appended")
#endif
}

} // namespace NSGKDBus

