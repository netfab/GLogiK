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

#include "lib/utils/utils.h"

#include "GKDBusMessage.h"

namespace GLogiK
{

GKDBusMessage::GKDBusMessage(DBusConnection* connection, const bool logoff)
	: connection_(connection), message_(nullptr), hosed_message_(false), log_off_(logoff)
{
	/* sanity check */
	if(connection == nullptr)
		throw GLogiKExcept("current connection is NULL");
}

GKDBusMessage::~GKDBusMessage() {
}

void GKDBusMessage::appendBoolean(const bool value) {
	dbus_bool_t v = value;
	if( ! dbus_message_iter_append_basic(&this->args_it_, DBUS_TYPE_BOOLEAN, &v) ) {
		this->hosed_message_ = true;
		LOG(ERROR) << "boolean append_basic failure, not enough memory";
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}
#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ ) {
		LOG(DEBUG2) << "DBus boolean value appended";
	}
#endif
}

void GKDBusMessage::appendString(const std::string & value) {
	const char* p = value.c_str();
	if( ! dbus_message_iter_append_basic(&this->args_it_, DBUS_TYPE_STRING, &p) ) {
		this->hosed_message_ = true;
		LOG(ERROR) << "string append_basic failure, not enough memory";
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}
#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ ) {
		LOG(DEBUG2) << "DBus string value appended";
	}
#endif
}

void GKDBusMessage::appendStringVector(const std::vector<std::string> & list) {
	DBusMessageIter container_it;

	if( ! dbus_message_iter_open_container(&this->args_it_, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &container_it) ) {
		this->hosed_message_ = true;
		LOG(ERROR) << "string array open_container failure, not enough memory";
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}

	for (const std::string & s : list) {
		const char* p = s.c_str();
		if( ! dbus_message_iter_append_basic(&container_it, DBUS_TYPE_STRING, &p) ) {
			LOG(ERROR) << "string array append_basic failure, not enough memory";
			this->hosed_message_ = true;
			dbus_message_iter_abandon_container(&this->args_it_, &container_it);
			throw GKDBusOOMWrongBuild(this->append_failure_);
		}
	}

	if( ! dbus_message_iter_close_container(&this->args_it_, &container_it) ) {
		LOG(ERROR) << "string array close_container failure, not enough memory";
		this->hosed_message_ = true;
		dbus_message_iter_abandon_container(&this->args_it_, &container_it);
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ ) {
		LOG(DEBUG2) << "DBus string array appended";
	}
#endif
}

void GKDBusMessage::appendUInt8(const uint8_t value) {
	this->appendUInt8(&this->args_it_, value);
}

void GKDBusMessage::appendUInt16(const uint16_t value) {
	this->appendUInt16(&this->args_it_, value);
}

void GKDBusMessage::appendUInt32(const uint32_t value) {
	dbus_uint32_t v = value;
	if( ! dbus_message_iter_append_basic(&this->args_it_, DBUS_TYPE_UINT32, &v) ) {
		this->hosed_message_ = true;
		throw GKDBusOOMWrongBuild("uint32_t append failure, not enough memory");
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ ) {
		LOG(DEBUG2) << "DBus uint32_t value appended";
	}
#endif
}

void GKDBusMessage::appendMacro(const macro_t & macro_array) {
	DBusMessageIter array_it;

	const char array_sig[] = \
							DBUS_STRUCT_BEGIN_CHAR_AS_STRING\
							DBUS_TYPE_BYTE_AS_STRING\
							DBUS_TYPE_BYTE_AS_STRING\
							DBUS_TYPE_UINT16_AS_STRING\
							DBUS_STRUCT_END_CHAR_AS_STRING;

	if( ! dbus_message_iter_open_container(&this->args_it_, DBUS_TYPE_ARRAY, array_sig, &array_it) ) {
		this->hosed_message_ = true;
		LOG(ERROR) << "macro array open_container failure, not enough memory";
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}

	const char struct_sig[] = \
							DBUS_TYPE_BYTE_AS_STRING\
							DBUS_TYPE_BYTE_AS_STRING\
							DBUS_TYPE_UINT16_AS_STRING;

	try {
		for(const auto & keyEvent : macro_array) {
			DBusMessageIter struct_it;

			if( ! dbus_message_iter_open_container(&array_it, DBUS_TYPE_STRUCT, struct_sig, &struct_it) ) {
				LOG(ERROR) << "DBus struct open_container failure, not enough memory";
				throw GKDBusOOMWrongBuild(this->append_failure_);
			}

			try {
				this->appendUInt8(&struct_it, keyEvent.event_code);
				this->appendUInt8(&struct_it, to_type(keyEvent.event));
				this->appendUInt16(&struct_it, keyEvent.interval);
			}
			catch (const GKDBusOOMWrongBuild & e) {
				dbus_message_iter_abandon_container(&array_it, &struct_it);
				throw;
			}

			if( ! dbus_message_iter_close_container(&array_it, &struct_it) ) {
				LOG(ERROR) << "DBus struct close_container failure, not enough memory";
				throw GKDBusOOMWrongBuild(this->append_failure_);
			}
		}
	}
	catch (const GKDBusOOMWrongBuild & e) {
		this->hosed_message_ = true;
		dbus_message_iter_abandon_container(&this->args_it_, &array_it);
		throw;
	}

	if( ! dbus_message_iter_close_container(&this->args_it_, &array_it) ) {
		LOG(ERROR) << "macro array close_container failure, not enough memory";
		this->hosed_message_ = true;
		dbus_message_iter_abandon_container(&this->args_it_, &array_it);
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ ) {
		LOG(DEBUG2) << "DBus macro appended";
	}
#endif
}

/*
void GKDBusMessage::appendVariantToMessage(const std::string & value) {
	DBusMessageIter container_it;

	if( ! dbus_message_iter_open_container(&this->args_it_, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &container_it) ) {
		this->hosed_message_ = true;
		LOG(ERROR) << "string variant open_container failure, not enough memory";
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}

	const char* p = value.c_str();

	if( ! dbus_message_iter_append_basic(&container_it, DBUS_TYPE_STRING, &p) ) {
		LOG(ERROR) << "string variant append_basic failure, not enough memory";
		this->hosed_message_ = true;
		dbus_message_iter_abandon_container(&this->args_it_, &container_it);
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}

	if( ! dbus_message_iter_close_container(&this->args_it_, &container_it) ) {
		LOG(ERROR) << "string variant close_container failure, not enough memory";
		this->hosed_message_ = true;
		dbus_message_iter_abandon_container(&this->args_it_, &container_it);
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ ) {
		LOG(DEBUG2) << "DBus string variant appended";
	}
#endif
}

void GKDBusMessage::appendVariantToMessage(const unsigned char value) {
	DBusMessageIter container_it;

	if( ! dbus_message_iter_open_container(&this->args_it_, DBUS_TYPE_VARIANT, DBUS_TYPE_BYTE_AS_STRING, &container_it) ) {
		this->hosed_message_ = true;
		LOG(ERROR) << "BYTE variant open_container failure, not enough memory";
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}

	if( ! dbus_message_iter_append_basic(&container_it, DBUS_TYPE_BYTE, &value) ) {
		LOG(ERROR) << "BYTE variant append_basic failure, not enough memory";
		this->hosed_message_ = true;
		dbus_message_iter_abandon_container(&this->args_it_, &container_it);
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}

	if( ! dbus_message_iter_close_container(&this->args_it_, &container_it) ) {
		LOG(ERROR) << "BYTE variant close_container failure, not enough memory";
		this->hosed_message_ = true;
		dbus_message_iter_abandon_container(&this->args_it_, &container_it);
		throw GKDBusOOMWrongBuild(this->append_failure_);
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ ) {
		LOG(DEBUG2) << "DBus BYTE variant appended";
	}
#endif
}
*/

void GKDBusMessage::appendUInt8(DBusMessageIter *iter, const uint8_t value) {
	if( ! dbus_message_iter_append_basic(iter, DBUS_TYPE_BYTE, &value) ) {
		this->hosed_message_ = true;
		throw GKDBusOOMWrongBuild("uint8_t append failure, not enough memory");
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ ) {
		LOG(DEBUG2) << "DBus uint8_t value appended";
	}
#endif
}

void GKDBusMessage::appendUInt16(DBusMessageIter *iter, const uint16_t value) {
	if( ! dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT16, &value) ) {
		this->hosed_message_ = true;
		throw GKDBusOOMWrongBuild("uint16_t append failure, not enough memory");
	}

#if DEBUG_GKDBUS_SUBOBJECTS
	if( ! this->log_off_ ) {
		LOG(DEBUG2) << "DBus uint16_t value appended";
	}
#endif
}

} // namespace GLogiK

