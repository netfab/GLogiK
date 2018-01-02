/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2017  Fabrice Delliaux <netbox253@gmail.com>
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

#include "lib/utils/utils.h"

#include "GKDBusArgument.h"

namespace GLogiK
{

thread_local std::vector<std::string> GKDBusArgument::string_arguments_ = {};
thread_local std::vector<uint8_t> GKDBusArgument::byte_arguments_ = {};
thread_local std::vector<uint16_t> GKDBusArgument::uint16_arguments_ = {};
thread_local std::vector<bool> GKDBusArgument::boolean_arguments_ = {};

void GKDBusArgument::decodeArgumentFromIterator(
	DBusMessageIter* iter,
	const char* signature,
	const unsigned int num,
	const bool logoff
) {
	int current_type = dbus_message_iter_get_arg_type(iter);

	DBusSignatureIter sig_it;

	// sometimes current_type could not be recognized and
	// is setted up as INVALID (for example when decoding arrays)
	// in this cases, we are trying to use the expected signature
	if(current_type == DBUS_TYPE_INVALID) {
		dbus_signature_iter_init(&sig_it, signature);
		current_type = dbus_signature_iter_get_current_type(&sig_it);
	}

#if DEBUGGING_ON
	if( ! logoff ) {
		LOG(DEBUG3) << "decoding argument: " << num << " type: "
					<< static_cast<char>(current_type) << " sig: " << signature;
	}
#endif

	switch(current_type) {
		case DBUS_TYPE_STRING:
		case DBUS_TYPE_OBJECT_PATH:
			{
				const char* value = nullptr;
				dbus_message_iter_get_basic(iter, &value);
				//LOG(DEBUG4) << "string arg value : " << value;
				GKDBusArgument::string_arguments_.push_back(value);
			}
			break;
		case DBUS_TYPE_BOOLEAN:
			{
				bool value = false;
				dbus_message_iter_get_basic(iter, &value);
				//LOG(DEBUG4) << "bool arg value : " << value;
				GKDBusArgument::boolean_arguments_.push_back(value);
			}
			break;
		case DBUS_TYPE_ARRAY:
			{
				unsigned int c = 0;
				DBusMessageIter array_it;
				dbus_message_iter_recurse(iter, &array_it);
				/* checking that we really have an array here, else
				 * with recursion this could lead to bad things */
				if(dbus_message_iter_get_arg_type(&array_it) == DBUS_TYPE_INVALID)
					break;
				do {
					c++; /* bonus point */
					char* sig = dbus_message_iter_get_signature(&array_it);
					GKDBusArgument::decodeArgumentFromIterator(&array_it, sig, c, logoff);
					dbus_free(sig);
				}
				while( dbus_message_iter_next(&array_it) );
			}
			break;
		case DBUS_TYPE_BYTE:
			{
				uint8_t byte = 0;
				dbus_message_iter_get_basic(iter, &byte);
				GKDBusArgument::byte_arguments_.push_back(byte);
				//LOG(DEBUG4) << "byte arg value : " << byte;
			}
			break;
		case DBUS_TYPE_UINT16:
			{
				uint16_t value = 0;
				dbus_message_iter_get_basic(iter, &value);
				GKDBusArgument::uint16_arguments_.push_back(value);
				//LOG(DEBUG4) << "uint16 arg value : " << value;
			}
			break;
		case DBUS_TYPE_STRUCT:
			{
				do {
					unsigned int c = 0;
					DBusMessageIter struct_it;
					dbus_message_iter_recurse(iter, &struct_it);
					do {
						c++; /* bonus point */
						char* sig = dbus_message_iter_get_signature(&struct_it);
						GKDBusArgument::decodeArgumentFromIterator(&struct_it, sig, c, logoff);
						dbus_free(sig);
					}
					while( dbus_message_iter_next(&struct_it) );
				}
				while( dbus_message_iter_next(iter) );
			}
			break;
		default: // other dbus type
			LOG(ERROR) << "unhandled argument type: " << static_cast<char>(current_type) << " sig: " << signature;
			break;
	}
}

void GKDBusArgument::fillInArguments(DBusMessage* message, bool logoff) {
	GKDBusArgument::string_arguments_.clear();
	GKDBusArgument::boolean_arguments_.clear();
	GKDBusArgument::byte_arguments_.clear();
	GKDBusArgument::uint16_arguments_.clear();

	if(message == nullptr) {
		LOG(WARNING) << __func__ << " : message is NULL";
		return;
	}

	unsigned int c = 0;
	DBusMessageIter arg_it;

	if( ! dbus_message_iter_init(message, &arg_it) )
		return; /* no arguments */

	do {
		c++; /* bonus point */
		char* signature = dbus_message_iter_get_signature(&arg_it);
		GKDBusArgument::decodeArgumentFromIterator(&arg_it, signature, c, logoff);
		dbus_free(signature);
	}
	while( dbus_message_iter_next(&arg_it) );

	if( ! GKDBusArgument::string_arguments_.empty() )
		std::reverse(GKDBusArgument::string_arguments_.begin(), GKDBusArgument::string_arguments_.end());
	if( ! GKDBusArgument::boolean_arguments_.empty() )
		std::reverse(GKDBusArgument::boolean_arguments_.begin(), GKDBusArgument::boolean_arguments_.end());
	if( ! GKDBusArgument::byte_arguments_.empty() )
		std::reverse(GKDBusArgument::byte_arguments_.begin(), GKDBusArgument::byte_arguments_.end());
	if( ! GKDBusArgument::uint16_arguments_.empty() )
		std::reverse(GKDBusArgument::uint16_arguments_.begin(), GKDBusArgument::uint16_arguments_.end());
}

} // namespace GLogiK

