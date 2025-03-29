/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2025  Fabrice Delliaux <netbox253@gmail.com>
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

#include "uint64.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

void TypeUInt64::appendUInt64(const uint64_t value)
{
	this->appendUInt64(&_itMessage, value);
}

void TypeUInt64::appendUInt64(DBusMessageIter *iter, const uint64_t value)
{
	GK_LOG_FUNC

	if( ! dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT64, &value) ) {
		_hosedMessage = true;
		throw GKDBusMessageWrongBuild("uint64_t append failure, not enough memory");
	}

#if DEBUG_GKDBUS
	GKLog(trace, "uint64_t appended")
#endif
}

const uint64_t ArgUInt64::getNextUInt64Argument(void) {
	if( ArgBase::uint64Arguments.empty() )
		throw EmptyContainer("missing argument : uint64");
	const uint64_t ret = ArgBase::uint64Arguments.back();
	ArgBase::uint64Arguments.pop_back();
	return ret;
}

} // namespace NSGKDBus

