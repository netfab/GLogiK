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

#include "lib/utils/utils.hpp"

#include "string.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

void TypeString::appendString(const std::string & value)
{
	this->appendString(&_itMessage, value);
}

void TypeString::appendString(DBusMessageIter *iter, const std::string & value)
{
	GK_LOG_FUNC

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "appending string")
#endif

	if( ! value.empty() ) {
		const char* p = value.c_str();
		if( ! dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &p) ) {
			_hosedMessage = true;
			LOG(error) << "string append_basic failure, not enough memory";
			throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
		}
#if DEBUG_GKDBUS_SUBOBJECTS
		GKLog(trace, "string appended")
#endif
	}
	else {
		this->appendUInt64(iter, 0);
#if DEBUG_GKDBUS_SUBOBJECTS
		GKLog(trace, "empty string appended")
#endif
	}
}

thread_local std::string ArgString::currentString("");

const std::string & ArgString::getNextStringArgument(void)
{
	ArgString::currentString.clear();

	const uint64_t size = ArgUInt64::getNextUInt64Argument();

	if( size != 0 ) {
		if( ArgBase::stringArguments.empty() )
			throw EmptyContainer("missing argument : string");

		ArgString::currentString = ArgBase::stringArguments.back();
		ArgBase::stringArguments.pop_back();

		if( ArgString::currentString.size() != size ) {
			LOG(warning) << "wrong string size";
		}
	}

	return ArgString::currentString;
}

// FIXME
const std::vector<std::string> & ArgString::getStringsArray(void)
{
	return ArgBase::stringArguments;
}

} // namespace NSGKDBus
