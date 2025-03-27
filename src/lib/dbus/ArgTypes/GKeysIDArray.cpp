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

#include <stdexcept>
#include <new>
#include <string>

#include <dbus/dbus.h>

#include "lib/utils/utils.hpp"

#include "GKeysIDArray.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;


void TypeGKeysIDArray::appendGKeysIDArray(const GLogiK::GKeysIDArray_type & keysID)
{
	GK_LOG_FUNC

	DBusMessageIter itContainer;

	const uint8_t size = keysID.size();
	this->appendUInt8(size);

	if( ! dbus_message_iter_open_container(&_itMessage, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &itContainer) ) {
		_hosedMessage = true;
		LOG(error) << "GKeysID array open_container failure, not enough memory";
		throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
	}

	for(const GLogiK::GKeysID keyID : keysID) {
		const uint8_t value = toEnumType(keyID);
		if( ! dbus_message_iter_append_basic(&itContainer, DBUS_TYPE_BYTE, &value) ) {
			LOG(error) << "GKeysID array append_basic failure, not enough memory";
			_hosedMessage = true;
			dbus_message_iter_abandon_container(&_itMessage, &itContainer);
			throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
		}
	}

	if( ! dbus_message_iter_close_container(&_itMessage, &itContainer) ) {
		LOG(error) << "GKeysID array close_container failure, not enough memory";
		_hosedMessage = true;
		throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
	}

#if DEBUG_GKDBUS
	GKLog(trace, "GKeysID array appended")
#endif
}

/*
 * helper function to rebuild GKeysIDArray
 * see also TypeGKeysIDArray::appendGKeysIDArray
 */
const GLogiK::GKeysIDArray_type ArgGKeysIDArray::getNextGKeysIDArrayArgument(void)
{
	GK_LOG_FUNC

	GKLog(trace, "rebuilding GKeysIDArray from GKDBus values")

	const std::string rebuild_failed("rebuilding GKeysIDArray failed");
	GLogiK::GKeysIDArray_type ret;

	try {
		using Size = GLogiK::GKeysIDArray_type::size_type;

		const Size size = ArgUInt8::getNextByteArgument();
		Size i = 0;

		ret.reserve(size);

		while(i < size) {
			ret.push_back(ArgGKeysID::getNextGKeysIDArgument());
			++i;
		}
	}
	catch( const EmptyContainer & e ) {
		LOG(warning) << "missing GKeysID argument : " << e.what();
		throw GLogiKExcept(rebuild_failed);
	}
	catch( const std::length_error & e ) {
		LOG(warning) << "reserve length_error failure : " << e.what();
		throw GLogiKExcept(rebuild_failed);
	}
	catch( const std::bad_alloc & e ) {
		LOG(warning) << "reserve bad_alloc failure : " << e.what();
		throw GLogiKExcept(rebuild_failed);
	}

	GKLog2(trace, "GKeysIDArray size: ", ret.size())

	return ret;
}

} // namespace NSGKDBus

