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

#include "int32.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

void TypeInt32::appendInt32(const int32_t value)
{
	this->appendInt32(&_itMessage, value);
}

void TypeInt32::appendInt32(DBusMessageIter *iter, const int32_t value)
{
	GK_LOG_FUNC

	if( ! dbus_message_iter_append_basic(iter, DBUS_TYPE_INT32, &value) ) {
		_hosedMessage = true;
		throw GKDBusMessageWrongBuild("int32_t append failure, not enough memory");
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "int32_t appended")
#endif
}

const int32_t ArgInt32::getNextInt32Argument(void) {
	if( ArgBase::int32Arguments.empty() )
		throw EmptyContainer("missing argument : int32");
	const int32_t ret = ArgBase::int32Arguments.back();
	ArgBase::int32Arguments.pop_back();
	return ret;
}

} // namespace NSGKDBus

