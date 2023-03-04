/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2022  Fabrice Delliaux <netbox253@gmail.com>
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

#include "GKDBusMessage.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

GKDBusMessage::GKDBusMessage(
	DBusConnection* const connection,
	const bool check)
		:	_connection(connection),
			_message(nullptr)
{
	/* sanity check */
	if( (connection == nullptr) and (! check) )
		throw GKDBusMessageWrongBuild("current connection is NULL");
}

GKDBusMessage::~GKDBusMessage()
{
}

void GKDBusMessage::appendLCDPluginsPropertiesArray(const GLogiK::LCDPluginsPropertiesArray_type & pluginsArray)
{
	this->appendLCDPluginsPropertiesArray(&_itMessage, pluginsArray);
}

/*
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 *
 * === private === private === private === private === private ===
 *
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 */

void GKDBusMessage::appendLCDPluginsPropertiesArray(
	DBusMessageIter *iter,
	const GLogiK::LCDPluginsPropertiesArray_type & pluginsArray)
{
	GK_LOG_FUNC

	DBusMessageIter itArray;

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
		for(const auto & plugin : pluginsArray) {
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
				dbus_message_iter_abandon_container(&itArray, &itStruct);
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
		dbus_message_iter_abandon_container(iter, &itArray);
		throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "LCDPluginsProperties array appended")
#endif
}


} // namespace NSGKDBus

