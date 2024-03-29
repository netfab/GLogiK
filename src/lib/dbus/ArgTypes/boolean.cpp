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

#include "boolean.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

void TypeBoolean::appendBoolean(const bool value)
{
	GK_LOG_FUNC

	dbus_bool_t v = value;
	if( ! dbus_message_iter_append_basic(&_itMessage, DBUS_TYPE_BOOLEAN, &v) ) {
		_hosedMessage = true;
		LOG(error) << "boolean append_basic failure, not enough memory";
		throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
	}
#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "boolean appended")
#endif
}

const bool ArgBoolean::getNextBooleanArgument(void)
{
	if( ArgBase::booleanArguments.empty() )
		throw EmptyContainer("missing argument : boolean");
	const bool ret = ArgBase::booleanArguments.back();
	ArgBase::booleanArguments.pop_back();
	return ret;
}

} // namespace NSGKDBus

