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

#include "lib/utils/utils.hpp"

#include "uint8.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

void TypeUInt8::appendUInt8(const uint8_t value)
{
	this->appendUInt8(&_itMessage, value);
}

void TypeUInt8::appendUInt8(DBusMessageIter *iter, const uint8_t value)
{
	GK_LOG_FUNC

	if( ! dbus_message_iter_append_basic(iter, DBUS_TYPE_BYTE, &value) ) {
		_hosedMessage = true;
		throw GKDBusMessageWrongBuild("uint8_t append failure, not enough memory");
	}

#if DEBUG_GKDBUS
	GKLog(trace, "uint8_t appended")
#endif
}

const uint8_t ArgUInt8::getNextByteArgument(void) {
	if( ArgBase::byteArguments.empty() )
		throw EmptyContainer("missing argument : byte");
	const uint8_t ret = ArgBase::byteArguments.back();
	ArgBase::byteArguments.pop_back();
	return ret;
}

} // namespace NSGKDBus

