/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2018  Fabrice Delliaux <netbox253@gmail.com>
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

#include "lib/utils/utils.hpp"

#include "GKDBusArgument.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

thread_local std::vector<std::string> GKDBusArgument::stringArguments = {};
thread_local std::vector<uint8_t> GKDBusArgument::byteArguments = {};
thread_local std::vector<uint16_t> GKDBusArgument::uint16Arguments = {};
thread_local std::vector<uint64_t> GKDBusArgument::uint64Arguments = {};
thread_local std::vector<bool> GKDBusArgument::booleanArguments = {};

void GKDBusArgument::decodeArgumentFromIterator(
	DBusMessageIter* iter,
	const char* signature,
	const unsigned int num,
	const bool disabledDebugOutput
) {
	int currentType = dbus_message_iter_get_arg_type(iter);

	DBusSignatureIter itSignature;

	// sometimes currentType could not be recognized and
	// is setted up as INVALID (for example when decoding arrays)
	// in this cases, we are trying to use the expected signature
	if(currentType == DBUS_TYPE_INVALID) {
		dbus_signature_iter_init(&itSignature, signature);
		currentType = dbus_signature_iter_get_current_type(&itSignature);
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! disabledDebugOutput ) {
		LOG(DEBUG3) << "decoding argument: " << num << " type: "
					<< static_cast<char>(currentType) << " sig: " << signature;
	}
#endif

	switch(currentType) {
		case DBUS_TYPE_STRING:
		case DBUS_TYPE_OBJECT_PATH:
			{
				const char* value = nullptr;
				dbus_message_iter_get_basic(iter, &value);
				//LOG(DEBUG4) << "string arg value : " << value;
				GKDBusArgument::stringArguments.push_back(value);
			}
			break;
		case DBUS_TYPE_BOOLEAN:
			{
				bool value = false;
				dbus_message_iter_get_basic(iter, &value);
				//LOG(DEBUG4) << "bool arg value : " << value;
				GKDBusArgument::booleanArguments.push_back(value);
			}
			break;
		case DBUS_TYPE_ARRAY:
			{
				unsigned int c = 0;
				DBusMessageIter itArray;
				dbus_message_iter_recurse(iter, &itArray);
				/* checking that we really have an array here, else
				 * with recursion this could lead to bad things */
				if(dbus_message_iter_get_arg_type(&itArray) == DBUS_TYPE_INVALID)
					break;
				do {
					c++; /* bonus point */
					char* sig = dbus_message_iter_get_signature(&itArray);
					GKDBusArgument::decodeArgumentFromIterator(&itArray, sig, c, disabledDebugOutput);
					dbus_free(sig);
				}
				while( dbus_message_iter_next(&itArray) );
			}
			break;
		case DBUS_TYPE_BYTE:
			{
				uint8_t byte = 0;
				dbus_message_iter_get_basic(iter, &byte);
				GKDBusArgument::byteArguments.push_back(byte);
				//LOG(DEBUG4) << "byte arg value : " << byte;
			}
			break;
		case DBUS_TYPE_UINT16:
			{
				uint16_t value = 0;
				dbus_message_iter_get_basic(iter, &value);
				GKDBusArgument::uint16Arguments.push_back(value);
				//LOG(DEBUG4) << "uint16 arg value : " << value;
			}
			break;
		case DBUS_TYPE_UINT64:
			{
				uint64_t value = 0;
				dbus_message_iter_get_basic(iter, &value);
				GKDBusArgument::uint64Arguments.push_back(value);
				//LOG(DEBUG4) << "uint64 arg value : " << value;
			}
			break;
		case DBUS_TYPE_STRUCT:
			{
				do {
					unsigned int c = 0;
					DBusMessageIter itStruct;
					dbus_message_iter_recurse(iter, &itStruct);
					do {
						c++; /* bonus point */
						char* sig = dbus_message_iter_get_signature(&itStruct);
						GKDBusArgument::decodeArgumentFromIterator(&itStruct, sig, c, disabledDebugOutput);
						dbus_free(sig);
					}
					while( dbus_message_iter_next(&itStruct) );
				}
				while( dbus_message_iter_next(iter) );
			}
			break;
		case DBUS_TYPE_VARIANT:
			{
				unsigned int c = 0;
				DBusMessageIter itSub;
				//LOG(DEBUG4) << "parsing variant";
				dbus_message_iter_recurse(iter, &itSub);
				char* sig = dbus_message_iter_get_signature(&itSub);
				GKDBusArgument::decodeArgumentFromIterator(&itSub, sig, c, disabledDebugOutput);
				dbus_free(sig);
			}
			break;
		default: // other dbus type
			LOG(ERROR) << "unhandled argument type: " << static_cast<char>(currentType) << " sig: " << signature;
			break;
	}
}

void GKDBusArgument::fillInArguments(DBusMessage* message, const bool disabledDebugOutput) {
	GKDBusArgument::stringArguments.clear();
	GKDBusArgument::booleanArguments.clear();
	GKDBusArgument::byteArguments.clear();
	GKDBusArgument::uint16Arguments.clear();
	GKDBusArgument::uint64Arguments.clear();

	if(message == nullptr) {
		LOG(WARNING) << __func__ << " : message is NULL";
		return;
	}

	unsigned int c = 0;
	DBusMessageIter itArgument;

	if( ! dbus_message_iter_init(message, &itArgument) )
		return; /* no arguments */

	do {
		c++; /* bonus point */
		char* signature = dbus_message_iter_get_signature(&itArgument);
		GKDBusArgument::decodeArgumentFromIterator(&itArgument, signature, c, disabledDebugOutput);
		dbus_free(signature);
	}
	while( dbus_message_iter_next(&itArgument) );

	if( ! GKDBusArgument::stringArguments.empty() )
		std::reverse(GKDBusArgument::stringArguments.begin(), GKDBusArgument::stringArguments.end());
	if( ! GKDBusArgument::booleanArguments.empty() )
		std::reverse(GKDBusArgument::booleanArguments.begin(), GKDBusArgument::booleanArguments.end());
	if( ! GKDBusArgument::byteArguments.empty() )
		std::reverse(GKDBusArgument::byteArguments.begin(), GKDBusArgument::byteArguments.end());
	if( ! GKDBusArgument::uint16Arguments.empty() )
		std::reverse(GKDBusArgument::uint16Arguments.begin(), GKDBusArgument::uint16Arguments.end());
	if( ! GKDBusArgument::uint64Arguments.empty() )
		std::reverse(GKDBusArgument::uint64Arguments.begin(), GKDBusArgument::uint64Arguments.end());
}

const int GKDBusArgument::decodeNextArgument(DBusMessageIter* itArgument) {
	GKDBusArgument::stringArguments.clear();
	GKDBusArgument::booleanArguments.clear();
	GKDBusArgument::byteArguments.clear();
	GKDBusArgument::uint16Arguments.clear();
	GKDBusArgument::uint64Arguments.clear();

	int currentType = dbus_message_iter_get_arg_type(itArgument);
	if(currentType == DBUS_TYPE_INVALID) /* no more arguments, or struct or array */
		return currentType;

	unsigned int c = 0;
	char* signature = dbus_message_iter_get_signature(itArgument);
	GKDBusArgument::decodeArgumentFromIterator(itArgument, signature, c, true);
	dbus_free(signature);

	return currentType;
}

} // namespace NSGKDBus

