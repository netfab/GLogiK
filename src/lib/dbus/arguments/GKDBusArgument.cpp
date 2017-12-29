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

std::vector<std::string> CBArgument::string_arguments_ = {};
std::vector<uint8_t> CBArgument::byte_arguments_ = {};
std::vector<uint16_t> CBArgument::uint16_arguments_ = {};
std::vector<bool> CBArgument::boolean_arguments_ = {};

void CBArgument::decodeArgumentFromIterator(DBusMessageIter* iter, const char* signature, const unsigned int num) {
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
	//if(! this->logoff_) { // FIXME
		LOG(DEBUG3) << "decoding argument: " << num << " type: "
					<< static_cast<char>(current_type) << " sig: " << signature;
	//}
#endif

	switch(current_type) {
		case DBUS_TYPE_STRING:
		case DBUS_TYPE_OBJECT_PATH:
			{
				const char* value = nullptr;
				dbus_message_iter_get_basic(iter, &value);
				//LOG(DEBUG4) << "string arg value : " << value;
				CBArgument::string_arguments_.push_back(value);
			}
			break;
		case DBUS_TYPE_BOOLEAN:
			{
				bool value = false;
				dbus_message_iter_get_basic(iter, &value);
				//LOG(DEBUG4) << "bool arg value : " << value;
				CBArgument::boolean_arguments_.push_back(value);
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
					CBArgument::decodeArgumentFromIterator(&array_it, sig, c);
					dbus_free(sig);
				}
				while( dbus_message_iter_next(&array_it) );
			}
			break;
		case DBUS_TYPE_BYTE:
			{
				uint8_t byte = 0;
				dbus_message_iter_get_basic(iter, &byte);
				CBArgument::byte_arguments_.push_back(byte);
				//LOG(DEBUG4) << "byte arg value : " << byte;
			}
			break;
		case DBUS_TYPE_UINT16:
			{
				uint16_t value = 0;
				dbus_message_iter_get_basic(iter, &value);
				CBArgument::uint16_arguments_.push_back(value);
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
						CBArgument::decodeArgumentFromIterator(&struct_it, sig, c);
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

void CBArgument::fillInArguments(DBusMessage* message) {
	CBArgument::string_arguments_.clear();
	CBArgument::boolean_arguments_.clear();
	CBArgument::byte_arguments_.clear();
	CBArgument::uint16_arguments_.clear();

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
		CBArgument::decodeArgumentFromIterator(&arg_it, signature, c);
		dbus_free(signature);
	}
	while( dbus_message_iter_next(&arg_it) );

	if( ! CBArgument::string_arguments_.empty() )
		std::reverse(CBArgument::string_arguments_.begin(), CBArgument::string_arguments_.end());
	if( ! CBArgument::boolean_arguments_.empty() )
		std::reverse(CBArgument::boolean_arguments_.begin(), CBArgument::boolean_arguments_.end());
	if( ! CBArgument::byte_arguments_.empty() )
		std::reverse(CBArgument::byte_arguments_.begin(), CBArgument::byte_arguments_.end());
	if( ! CBArgument::uint16_arguments_.empty() )
		std::reverse(CBArgument::uint16_arguments_.begin(), CBArgument::uint16_arguments_.end());
}

} // namespace GLogiK

