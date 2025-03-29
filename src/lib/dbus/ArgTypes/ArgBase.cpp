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

#include "ArgBase.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

thread_local std::vector<std::string> ArgBase::stringArguments = {};
thread_local std::vector<int32_t> ArgBase::int32Arguments = {};
thread_local std::vector<uint8_t> ArgBase::byteArguments = {};
thread_local std::vector<uint16_t> ArgBase::uint16Arguments = {};
thread_local std::vector<uint64_t> ArgBase::uint64Arguments = {};
thread_local std::vector<bool> ArgBase::booleanArguments = {};

void ArgBase::decodeArgumentFromIterator(
	DBusMessageIter* iter,
	const char* signature,
	const uint16_t num)
{
	GK_LOG_FUNC

	int currentType = dbus_message_iter_get_arg_type(iter);

	DBusSignatureIter itSignature;

	// sometimes currentType could not be recognized and
	// is setted up as INVALID (for example when decoding arrays)
	// in this cases, we are trying to use the expected signature
	if(currentType == DBUS_TYPE_INVALID) {
		dbus_signature_iter_init(&itSignature, signature);
		currentType = dbus_signature_iter_get_current_type(&itSignature);
	}

#if DEBUG_GKDBUS
	LOG(trace)	<< "decoding argument: " << num << " type: "
				<< static_cast<char>(currentType) << " sig: " << signature;
#endif

	switch(currentType) {
		case DBUS_TYPE_STRING:
		case DBUS_TYPE_OBJECT_PATH:
			{
				const char* value = nullptr;
				dbus_message_iter_get_basic(iter, &value);
				//GKLog2(trace, "string arg value : ", value)

				/* -
				 * ability to send empty strings, see
				 * ArgString::getNextStringArgument() and TypeString::appendString()
				 * in ArgTypes/string.cpp
				 *
				 * for non-empty strings, uint64_t argument must be added here, else
				 * ::decodeArgumentFromIterator() will fail to decode arguments from
				 * DBusMessage(s) that were NOT sent using libGKDBus
				 *
				 * typical error (with appendUInt64() implemented into appendString()) when
				 * trying to start desktop service :
				 * -
				 * error    - (...) GetSessionByPID method reply failure : missing argument : uint64
				 * error    - (...) failure to get session ID from logind
				 * (...)
				 * info     - (~DesktopService) GLogiKs desktop service process exiting, bye !
				 * error    - () unable to contact a session manager
				 * -
				 */
				const std::string arg(value);
				ArgBase::uint64Arguments.push_back(arg.size());

				ArgBase::stringArguments.push_back(arg);
			}
			break;
		case DBUS_TYPE_BOOLEAN:
			{
				bool value = false;
				dbus_message_iter_get_basic(iter, &value);
				//GKLog2(trace, "bool arg value : ", value)
				ArgBase::booleanArguments.push_back(value);
			}
			break;
		case DBUS_TYPE_ARRAY:
			{
				uint16_t c = 0;
				DBusMessageIter itArray;
				dbus_message_iter_recurse(iter, &itArray);
				/* checking that we really have an array here, else
				 * with recursion this could lead to bad things */
				if(dbus_message_iter_get_arg_type(&itArray) == DBUS_TYPE_INVALID)
					break;
				do {
					c++; /* bonus point */
					char* sig = dbus_message_iter_get_signature(&itArray);
					ArgBase::decodeArgumentFromIterator(&itArray, sig, c);
					dbus_free(sig);
				}
				while( dbus_message_iter_next(&itArray) );
			}
			break;
		case DBUS_TYPE_BYTE:
			{
				uint8_t byte = 0;
				dbus_message_iter_get_basic(iter, &byte);
				ArgBase::byteArguments.push_back(byte);
				//GKLog2(trace, "uint8_t arg value : ", value)
			}
			break;
		case DBUS_TYPE_UINT16:
			{
				uint16_t value = 0;
				dbus_message_iter_get_basic(iter, &value);
				ArgBase::uint16Arguments.push_back(value);
				//GKLog2(trace, "uint16_t arg value : ", value)
			}
			break;
		case DBUS_TYPE_UINT64:
			{
				uint64_t value = 0;
				dbus_message_iter_get_basic(iter, &value);
				ArgBase::uint64Arguments.push_back(value);
				//GKLog2(trace, "uint64_t arg value : ", value)
			}
			break;
		case DBUS_TYPE_UNIX_FD:
			{
#if SIZEOF_INT != 4
#error "wrong int size, please report bug"
#endif
				int32_t value = -1;
				dbus_message_iter_get_basic(iter, &value);
				ArgBase::int32Arguments.push_back(value);
				//GKLog2(trace, "int32_t arg value : ", value)
			}
			break;
		case DBUS_TYPE_STRUCT:
			{
				do {
					uint16_t c = 0;
					DBusMessageIter itStruct;
					dbus_message_iter_recurse(iter, &itStruct);
					do {
						c++; /* bonus point */
						char* sig = dbus_message_iter_get_signature(&itStruct);
						ArgBase::decodeArgumentFromIterator(&itStruct, sig, c);
						dbus_free(sig);
					}
					while( dbus_message_iter_next(&itStruct) );
				}
				while( dbus_message_iter_next(iter) );
			}
			break;
		case DBUS_TYPE_VARIANT:
			{
				uint16_t c = 0;
				DBusMessageIter itSub;
				//GKLog(trace, "parsing variant")
				dbus_message_iter_recurse(iter, &itSub);
				char* sig = dbus_message_iter_get_signature(&itSub);
				ArgBase::decodeArgumentFromIterator(&itSub, sig, c);
				dbus_free(sig);
			}
			break;
		default: // other dbus type
			LOG(error) << "unhandled argument type: " << static_cast<char>(currentType) << " sig: " << signature;
			break;
	}
}

void ArgBase::fillInArguments(DBusMessage* message)
{
	GK_LOG_FUNC

	ArgBase::stringArguments.clear();
	ArgBase::booleanArguments.clear();
	ArgBase::byteArguments.clear();
	ArgBase::uint16Arguments.clear();
	ArgBase::uint64Arguments.clear();

	if(message == nullptr) {
		LOG(warning) << "message is NULL";
		return;
	}

	uint16_t c = 0;
	DBusMessageIter itArgument;

	if( ! dbus_message_iter_init(message, &itArgument) )
		return; /* no arguments */

	do {
		c++; /* bonus point */
		char* signature = dbus_message_iter_get_signature(&itArgument);
		ArgBase::decodeArgumentFromIterator(&itArgument, signature, c);
		dbus_free(signature);
	}
	while( dbus_message_iter_next(&itArgument) );

	ArgBase::reverse(ArgBase::stringArguments);
	ArgBase::reverse(ArgBase::booleanArguments);
	ArgBase::reverse(ArgBase::int32Arguments);
	ArgBase::reverse(ArgBase::byteArguments);
	ArgBase::reverse(ArgBase::uint16Arguments);
	ArgBase::reverse(ArgBase::uint64Arguments);
}

const int ArgBase::decodeNextArgument(DBusMessageIter* itArgument)
{
	ArgBase::stringArguments.clear();
	ArgBase::booleanArguments.clear();
	ArgBase::int32Arguments.clear();
	ArgBase::byteArguments.clear();
	ArgBase::uint16Arguments.clear();
	ArgBase::uint64Arguments.clear();

	int currentType = dbus_message_iter_get_arg_type(itArgument);
	if(currentType == DBUS_TYPE_INVALID) /* no more arguments, or struct or array */
		return currentType;

	uint16_t c = 0;
	char* signature = dbus_message_iter_get_signature(itArgument);
	ArgBase::decodeArgumentFromIterator(itArgument, signature, c);
	dbus_free(signature);

	return currentType;
}

} // namespace NSGKDBus

