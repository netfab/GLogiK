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

#include "lib/utils/utils.hpp"

#include "LCDPPArray.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

void TypeLCDPPArray::appendLCDPPArray(const GLogiK::LCDPPArray_type & pluginsArray)
{
	this->appendLCDPPArray(&_itMessage, pluginsArray);
}

void TypeLCDPPArray::appendLCDPPArray(DBusMessageIter *iter, const GLogiK::LCDPPArray_type & pluginsArray)
{
	GK_LOG_FUNC

	DBusMessageIter itArray;

	this->appendUInt64(iter, pluginsArray.size());

	const char array_sig[] = \
							DBUS_STRUCT_BEGIN_CHAR_AS_STRING\
							DBUS_TYPE_UINT64_AS_STRING\
							DBUS_TYPE_STRING_AS_STRING\
							DBUS_TYPE_STRING_AS_STRING\
							DBUS_STRUCT_END_CHAR_AS_STRING;

	if( ! dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, array_sig, &itArray) ) {
		_hosedMessage = true;
		LOG(error) << "LCDPluginsProperties array open_container failure, not enough memory";
		throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
	}


	try {
		for(const auto & plugin : pluginsArray)
		{
			DBusMessageIter itStruct;

			/*
			 * From DBus dbus_message_iter_open_container documentation :
			 *		For structs and dict entries, contained_signature should be NULL;
			 */
			if( ! dbus_message_iter_open_container(&itArray, DBUS_TYPE_STRUCT, nullptr, &itStruct) ) {
				LOG(error) << "DBus struct open_container failure, not enough memory";
				throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
			}

			try {
				this->appendUInt64(&itStruct, plugin.getID());
				this->appendString(&itStruct, plugin.getName());
				this->appendString(&itStruct, plugin.getDesc());
			}
			catch (const GKDBusMessageWrongBuild & e) {
				dbus_message_iter_abandon_container(&itArray, &itStruct);
				throw;
			}

			if( ! dbus_message_iter_close_container(&itArray, &itStruct) ) {
				LOG(error) << "DBus struct close_container failure, not enough memory";
				throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
			}
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_hosedMessage = true;
		dbus_message_iter_abandon_container(iter, &itArray);
		throw;
	}
	/* -- */

	if( ! dbus_message_iter_close_container(iter, &itArray) ) {
		LOG(error) << "LCDPluginsProperties array close_container failure, not enough memory";
		_hosedMessage = true;
		throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "LCDPluginsProperties array appended")
#endif
}

/*
 * helper function to rebuild LCDPPArray_type array
 */
const GLogiK::LCDPPArray_type ArgLCDPPArray::getNextLCDPPArrayArgument(void)
{
	GK_LOG_FUNC

	GKLog(trace, "rebuilding LCDPluginsProperties vector from GKDBus values")

	const std::string rebuild_failed("rebuilding LCDPluginsProperties vector failed");

	const uint64_t size = ArgUInt64::getNextUInt64Argument();

	using Size = GLogiK::LCDPPArray_type::size_type;
	Size i = 0;

	GLogiK::LCDPPArray_type pluginsArray;

	try {
		pluginsArray.reserve(size);

		while(i < size) {
			const uint64_t id = ArgUInt64::getNextUInt64Argument();
			const std::string name = ArgString::getNextStringArgument();
			const std::string desc = ArgString::getNextStringArgument();
			pluginsArray.push_back({id, name, desc});
			++i;
		}
	}
	catch ( const EmptyContainer & e ) {
		LOG(warning) << "missing argument : " << e.what();
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

	GKLog4(trace, "LCDPluginsProperties vector size : ", pluginsArray.size(), "expected: ", size)

	return pluginsArray;
}

} // namespace NSGKDBus

