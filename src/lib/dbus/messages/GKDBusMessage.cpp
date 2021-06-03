/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
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
			_message(nullptr),
			_hosedMessage(false)
{
	/* sanity check */
	if( (connection == nullptr) and (! check) )
		throw GKDBusMessageWrongBuild("current connection is NULL");
}

GKDBusMessage::~GKDBusMessage()
{
}

void GKDBusMessage::abandon(void)
{
	GK_LOG_FUNC

	LOG(info) << "intentionally abandon message";
	_hosedMessage = true;
}

void GKDBusMessage::appendBoolean(const bool value)
{
	GK_LOG_FUNC

	dbus_bool_t v = value;
	if( ! dbus_message_iter_append_basic(&_itMessage, DBUS_TYPE_BOOLEAN, &v) ) {
		_hosedMessage = true;
		LOG(error) << "boolean append_basic failure, not enough memory";
		throw GKDBusMessageWrongBuild(_appendFailure);
	}
#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "boolean appended")
#endif
}

void GKDBusMessage::appendString(const std::string & value)
{
	this->appendString(&_itMessage, value);
}

void GKDBusMessage::appendStringVector(const std::vector<std::string> & list)
{
	GK_LOG_FUNC

	DBusMessageIter itContainer;

	if( ! dbus_message_iter_open_container(&_itMessage, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &itContainer) ) {
		_hosedMessage = true;
		LOG(error) << "string array open_container failure, not enough memory";
		throw GKDBusMessageWrongBuild(_appendFailure);
	}

	for (const std::string & s : list) {
		const char* p = s.c_str();
		if( ! dbus_message_iter_append_basic(&itContainer, DBUS_TYPE_STRING, &p) ) {
			LOG(error) << "string array append_basic failure, not enough memory";
			_hosedMessage = true;
			dbus_message_iter_abandon_container(&_itMessage, &itContainer);
			throw GKDBusMessageWrongBuild(_appendFailure);
		}
	}

	if( ! dbus_message_iter_close_container(&_itMessage, &itContainer) ) {
		LOG(error) << "string array close_container failure, not enough memory";
		_hosedMessage = true;
		dbus_message_iter_abandon_container(&_itMessage, &itContainer);
		throw GKDBusMessageWrongBuild(_appendFailure);
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "string array appended")
#endif
}

void GKDBusMessage::appendUInt8(const uint8_t value)
{
	this->appendUInt8(&_itMessage, value);
}

void GKDBusMessage::appendUInt16(const uint16_t value)
{
	this->appendUInt16(&_itMessage, value);
}

void GKDBusMessage::appendUInt32(const uint32_t value)
{
	GK_LOG_FUNC

	dbus_uint32_t v = value;
	if( ! dbus_message_iter_append_basic(&_itMessage, DBUS_TYPE_UINT32, &v) ) {
		_hosedMessage = true;
		throw GKDBusMessageWrongBuild("uint32_t append failure, not enough memory");
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "uint32_t appended")
#endif
}

void GKDBusMessage::appendUInt64(const uint64_t value)
{
	this->appendUInt64(&_itMessage, value);
}

void GKDBusMessage::appendMacro(const GLogiK::macro_type & macro)
{
	this->appendMacro(&_itMessage, macro);
}

void GKDBusMessage::appendMacrosBank(const GLogiK::mBank_type & bank)
{
	GK_LOG_FUNC

	DBusMessageIter itArray;

	const char array_sig[] = \
		DBUS_STRUCT_BEGIN_CHAR_AS_STRING\
		DBUS_TYPE_STRING_AS_STRING\
		DBUS_TYPE_BYTE_AS_STRING\
		DBUS_TYPE_ARRAY_AS_STRING\
		DBUS_STRUCT_BEGIN_CHAR_AS_STRING\
		DBUS_TYPE_BYTE_AS_STRING\
		DBUS_TYPE_BYTE_AS_STRING\
		DBUS_TYPE_UINT16_AS_STRING\
		DBUS_STRUCT_END_CHAR_AS_STRING\
		DBUS_STRUCT_END_CHAR_AS_STRING;

	if( ! dbus_message_iter_open_container(&_itMessage, DBUS_TYPE_ARRAY, array_sig, &itArray) ) {
		_hosedMessage = true;
		LOG(error) << "MacrosBank open_container failure, not enough memory";
		throw GKDBusMessageWrongBuild(_appendFailure);
	}

	/*
	 * From DBus dbus_message_iter_open_container documentation :
	 *		For structs and dict entries, contained_signature should be NULL;
	 */
	/*
	const char struct_sig[] = \
		DBUS_TYPE_STRING_AS_STRING\
		DBUS_TYPE_BYTE_AS_STRING\
		DBUS_TYPE_ARRAY_AS_STRING\
		DBUS_STRUCT_BEGIN_CHAR_AS_STRING\
		DBUS_TYPE_BYTE_AS_STRING\
		DBUS_TYPE_BYTE_AS_STRING\
		DBUS_TYPE_UINT16_AS_STRING\
		DBUS_STRUCT_END_CHAR_AS_STRING;
	*/

	try {
		for(const auto & keyMacroPair : bank) {
			const std::string & key = keyMacroPair.first;
			const GLogiK::macro_type & macro = keyMacroPair.second;

			bool append_macro = false;
			if( ( ! macro.empty() ) ) {
				if( macro.size() < MACRO_T_MAX_SIZE ) {
					append_macro = true;
				}
				else {
					LOG(warning) << "skipping " << key << " macro - size >= MACRO_T_MAX_SIZE";
				}
			}
			/* skip empty macro slots */
			if( ! append_macro )
				continue;

			DBusMessageIter itStruct;

			if( ! dbus_message_iter_open_container(&itArray, DBUS_TYPE_STRUCT, nullptr, &itStruct) ) {
				LOG(error) << "DBus struct open_container failure, not enough memory";
				throw GKDBusMessageWrongBuild(_appendFailure);
			}

			try {
				const uint8_t size = macro.size();

				this->appendString(&itStruct, key);
				this->appendUInt8(&itStruct, size);
				this->appendMacro(&itStruct, macro);
			}
			catch (const GKDBusMessageWrongBuild & e) {
				dbus_message_iter_abandon_container(&itArray, &itStruct);
				throw;
			}

			if( ! dbus_message_iter_close_container(&itArray, &itStruct) ) {
				LOG(error) << "DBus struct close_container failure, not enough memory";
				dbus_message_iter_abandon_container(&itArray, &itStruct);
				throw GKDBusMessageWrongBuild(_appendFailure);
			}
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_hosedMessage = true;
		dbus_message_iter_abandon_container(&_itMessage, &itArray);
		throw;
	}

	if( ! dbus_message_iter_close_container(&_itMessage, &itArray) ) {
		LOG(error) << "MacrosBank close_container failure, not enough memory";
		_hosedMessage = true;
		dbus_message_iter_abandon_container(&_itMessage, &itArray);
		throw GKDBusMessageWrongBuild(_appendFailure);
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "MacrosBank appended")
#endif
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
		throw GKDBusMessageWrongBuild(_appendFailure);
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
				throw GKDBusMessageWrongBuild(_appendFailure);
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
				throw GKDBusMessageWrongBuild(_appendFailure);
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
		throw GKDBusMessageWrongBuild(_appendFailure);
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "LCDPluginsProperties array appended")
#endif
}

void GKDBusMessage::appendMacro(DBusMessageIter *iter, const GLogiK::macro_type & macro)
{
	GK_LOG_FUNC

	DBusMessageIter itArray;

	const char array_sig[] = \
							DBUS_STRUCT_BEGIN_CHAR_AS_STRING\
							DBUS_TYPE_BYTE_AS_STRING\
							DBUS_TYPE_BYTE_AS_STRING\
							DBUS_TYPE_UINT16_AS_STRING\
							DBUS_STRUCT_END_CHAR_AS_STRING;

	if( ! dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, array_sig, &itArray) ) {
		_hosedMessage = true;
		LOG(error) << "macro array open_container failure, not enough memory";
		throw GKDBusMessageWrongBuild(_appendFailure);
	}

	/*
	 * From DBus dbus_message_iter_open_container documentation :
	 *		For structs and dict entries, contained_signature should be NULL;
	 */
	/*
	const char struct_sig[] = \
							DBUS_TYPE_BYTE_AS_STRING\
							DBUS_TYPE_BYTE_AS_STRING\
							DBUS_TYPE_UINT16_AS_STRING;
	*/

	try {
		for(const auto & keyEvent : macro) {
			DBusMessageIter itStruct;

			if( ! dbus_message_iter_open_container(&itArray, DBUS_TYPE_STRUCT, nullptr, &itStruct) ) {
				LOG(error) << "DBus struct open_container failure, not enough memory";
				throw GKDBusMessageWrongBuild(_appendFailure);
			}

			try {
				this->appendUInt8(&itStruct, keyEvent.code);
				this->appendUInt8(&itStruct, toEnumType(keyEvent.event));
				this->appendUInt16(&itStruct, keyEvent.interval);
			}
			catch (const GKDBusMessageWrongBuild & e) {
				dbus_message_iter_abandon_container(&itArray, &itStruct);
				throw;
			}

			if( ! dbus_message_iter_close_container(&itArray, &itStruct) ) {
				LOG(error) << "DBus struct close_container failure, not enough memory";
				dbus_message_iter_abandon_container(&itArray, &itStruct);
				throw GKDBusMessageWrongBuild(_appendFailure);
			}
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_hosedMessage = true;
		dbus_message_iter_abandon_container(iter, &itArray);
		throw;
	}

	if( ! dbus_message_iter_close_container(iter, &itArray) ) {
		LOG(error) << "macro array close_container failure, not enough memory";
		_hosedMessage = true;
		dbus_message_iter_abandon_container(iter, &itArray);
		throw GKDBusMessageWrongBuild(_appendFailure);
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "macro appended")
#endif
}

void GKDBusMessage::appendString(DBusMessageIter *iter, const std::string & value)
{
	GK_LOG_FUNC

	const char* p = value.c_str();
	if( ! dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &p) ) {
		_hosedMessage = true;
		LOG(error) << "string append_basic failure, not enough memory";
		throw GKDBusMessageWrongBuild(_appendFailure);
	}
#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "string appended")
#endif
}

void GKDBusMessage::appendUInt8(DBusMessageIter *iter, const uint8_t value)
{
	GK_LOG_FUNC

	if( ! dbus_message_iter_append_basic(iter, DBUS_TYPE_BYTE, &value) ) {
		_hosedMessage = true;
		throw GKDBusMessageWrongBuild("uint8_t append failure, not enough memory");
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "uint8_t appended")
#endif
}

void GKDBusMessage::appendUInt16(DBusMessageIter *iter, const uint16_t value)
{
	GK_LOG_FUNC

	if( ! dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT16, &value) ) {
		_hosedMessage = true;
		throw GKDBusMessageWrongBuild("uint16_t append failure, not enough memory");
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "uint16_t appended")
#endif
}

void GKDBusMessage::appendUInt64(DBusMessageIter *iter, const uint64_t value)
{
	GK_LOG_FUNC

	if( ! dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT64, &value) ) {
		_hosedMessage = true;
		throw GKDBusMessageWrongBuild("uint64_t append failure, not enough memory");
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "uint64_t appended")
#endif
}

} // namespace NSGKDBus

