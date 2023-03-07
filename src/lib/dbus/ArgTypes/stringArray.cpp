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

#include <algorithm>

#include <dbus/dbus.h>

#include "lib/utils/utils.hpp"

#include "stringArray.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

void TypeStringArray::appendStringArray(const std::vector<std::string> & stringArray)
{
	GK_LOG_FUNC

	this->appendUInt64(&_itMessage, stringArray.size());

	DBusMessageIter itContainer;

	if( ! dbus_message_iter_open_container(&_itMessage, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &itContainer) ) {
		_hosedMessage = true;
		LOG(error) << "string array open_container failure, not enough memory";
		throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
	}

	try {
		for (const std::string & s : stringArray) {
			this->appendString(&itContainer, s);
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_hosedMessage = true;
		dbus_message_iter_abandon_container(&_itMessage, &itContainer);
		throw;
	}

	if( ! dbus_message_iter_close_container(&_itMessage, &itContainer) ) {
		LOG(error) << "string array close_container failure, not enough memory";
		_hosedMessage = true;
		dbus_message_iter_abandon_container(&_itMessage, &itContainer);
		throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "string array appended")
#endif
}

const std::vector<std::string> ArgStringArray::getNextStringArray(void)
{
	GK_LOG_FUNC

	const uint64_t size = ArgUInt64::getNextUInt64Argument();
	std::vector<std::string> ret;

	if(size > 0) {
		auto & x = ArgBase::stringArguments;
		if(x.size() < size) {
			LOG(error) << "vector size: " << x.size() << " - wanted size: " << size;
			throw GLogiKExcept("wrong string array size");
		}

		ret.assign(x.end() - size, x.end());
		x.resize(x.size() - size);

		if( ! ret.empty() )
			std::reverse(ret.begin(), ret.end());
	}

	GKLog4(trace, "returning string array size: ", ret.size(), "expected: ", size)

	return ret;
}

} // namespace NSGKDBus

