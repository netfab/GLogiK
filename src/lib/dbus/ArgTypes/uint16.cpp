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

#include "uint16.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

void TypeUInt16::appendUInt16(const uint16_t value)
{
	this->appendUInt16(&_itMessage, value);
}

void TypeUInt16::appendUInt16(DBusMessageIter *iter, const uint16_t value)
{
	GK_LOG_FUNC

	if( ! dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT16, &value) ) {
		_hosedMessage = true;
		throw GKDBusMessageWrongBuild("uint16_t append failure, not enough memory");
	}

#if DEBUG_GKDBUS
	GKLog(trace, "uint16_t appended")
#endif
}

const uint16_t ArgUInt16::getNextUInt16Argument(void) {
	if( ArgBase::uint16Arguments.empty() )
		throw EmptyContainer("missing argument : uint16");
	const uint16_t ret = ArgBase::uint16Arguments.back();
	ArgBase::uint16Arguments.pop_back();
	return ret;
}

} // namespace NSGKDBus

