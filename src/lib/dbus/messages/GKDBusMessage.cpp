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

void GKDBusMessage::appendMKeysID(const GLogiK::MKeysID keyID)
{
	const uint8_t value = toEnumType(keyID);
	this->appendUInt8(value);
}

void GKDBusMessage::appendMKeysIDArray(const GLogiK::MKeysIDArray_type & keysID)
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

	for(const GLogiK::MKeysID keyID : keysID) {
		const uint8_t value = toEnumType(keyID);
		if( ! dbus_message_iter_append_basic(&itContainer, DBUS_TYPE_BYTE, &value) ) {
			LOG(error) << "MKeysID array append_basic failure, not enough memory";
			_hosedMessage = true;
			dbus_message_iter_abandon_container(&_itMessage, &itContainer);
			throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
		}
	}

	if( ! dbus_message_iter_close_container(&_itMessage, &itContainer) ) {
		LOG(error) << "MKeysID array close_container failure, not enough memory";
		_hosedMessage = true;
		dbus_message_iter_abandon_container(&_itMessage, &itContainer);
		throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "MKeysID array appended")
#endif
}

void GKDBusMessage::appendMacro(const GLogiK::macro_type & macro)
{
	this->appendMacro(&_itMessage, macro);
}

void GKDBusMessage::appendMacrosBank(const GLogiK::mBank_type & bank)
{
	GK_LOG_FUNC

	DBusMessageIter itArray;

	// signature = (yya(yyq))
	const char array_sig[] = \
		DBUS_STRUCT_BEGIN_CHAR_AS_STRING\
		DBUS_TYPE_BYTE_AS_STRING\
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
		throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
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
		for(const auto & keyEventPair : bank) {
			const uint8_t key = toEnumType(keyEventPair.first);
			const GLogiK::macro_type & macro = keyEventPair.second.getMacro();

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
				throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
			}

			try {
				const uint8_t size = macro.size();

				this->appendUInt8(&itStruct, key);
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
				throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
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
		throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
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

void GKDBusMessage::appendMacro(DBusMessageIter *iter, const GLogiK::macro_type & macro)
{
	GK_LOG_FUNC

	DBusMessageIter itArray;

	// signature = (yyq)
	const char array_sig[] = \
							DBUS_STRUCT_BEGIN_CHAR_AS_STRING\
							DBUS_TYPE_BYTE_AS_STRING\
							DBUS_TYPE_BYTE_AS_STRING\
							DBUS_TYPE_UINT16_AS_STRING\
							DBUS_STRUCT_END_CHAR_AS_STRING;

	if( ! dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, array_sig, &itArray) ) {
		_hosedMessage = true;
		LOG(error) << "macro array open_container failure, not enough memory";
		throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
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
				throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
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
				throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
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
		throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "macro appended")
#endif
}

} // namespace NSGKDBus

