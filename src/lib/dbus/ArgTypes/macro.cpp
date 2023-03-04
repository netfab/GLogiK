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

#include <cstdint>

#include <dbus/dbus.h>

#include "lib/utils/utils.hpp"

#include "macro.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

void TypeMacro::appendMacro(const GLogiK::macro_type & macro)
{
	this->appendMacro(&_itMessage, macro);
}

void TypeMacro::appendMacro(DBusMessageIter *iter, const GLogiK::macro_type & macro)
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

/*
 * helper function to  rebuild macro_type vector
 * see also TypeMacro::appendMacro
 */
const GLogiK::macro_type ArgMacro::getNextMacroArgument(const unsigned int macroSize)
{
	GK_LOG_FUNC

	GKLog(trace, "rebuilding macro from GKDBus values")

	GLogiK::macro_type macro;
	try {
		bool nextRun = true;
		do {
			GLogiK::KeyEvent e;

			e.code = ArgUInt8::getNextByteArgument();

			const uint8_t value = ArgUInt8::getNextByteArgument();
			if(value > GLogiK::EventValue::EVENT_KEY_UNKNOWN)
				throw GLogiKExcept("wrong event value for enum conversion");

			e.event		 = static_cast<GLogiK::EventValue>(value);
			e.interval	 = ArgUInt16::getNextUInt16Argument();

			macro.push_back(e);

			if( macroSize > 0 ) {
				if( macro.size() == macroSize )
					nextRun = false;
			}
			else {
				if( ArgUInt8::byteArguments.empty() )
					nextRun = false;
			}
		}
		while( nextRun );
	}
	catch ( const EmptyContainer & e ) {
		LOG(warning) << "missing macro argument : " << e.what();
		throw GLogiKExcept("rebuilding macro failed");
	}

	if( ( macroSize == 0 ) and ( ! ArgUInt16::uint16Arguments.empty() ) ) { /* sanity check */
		LOG(warning) << "uint16 container not empty";
	}

	GKLog4(trace, "events array size : ", macro.size(), "expected : ", macroSize)

	return macro;
}

} // namespace NSGKDBus
