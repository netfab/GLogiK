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

#ifndef SRC_LIB_DBUS_MSG_GKDBUS_MESSAGE_HPP_
#define SRC_LIB_DBUS_MSG_GKDBUS_MESSAGE_HPP_

#include <string>
#include <vector>

#include <dbus/dbus.h>

#include "include/keyEvent.hpp"
#include "include/LCDPluginProperties.hpp"

namespace NSGKDBus
{

class GKDBusMessage
{
	public:
		void appendBoolean(const bool value);
		void appendString(const std::string & value);
		void appendStringVector(const std::vector<std::string> & list);

		void appendUInt8(const uint8_t value);
		void appendUInt16(const uint16_t value);
		void appendUInt32(const uint32_t value);
		void appendUInt64(const uint64_t value);

		void appendMKeysID(const GLogiK::MKeysID keyID);
		void appendMacro(const GLogiK::macro_type & macro);
		void appendMacrosBank(const GLogiK::mBank_type & bank);
		void appendLCDPluginsPropertiesArray(
			const GLogiK::LCDPluginsPropertiesArray_type & pluginsArray
		);

		void abandon(void);

	protected:
		GKDBusMessage(
			DBusConnection* const connection,
			const bool check=false
		);
		~GKDBusMessage(void);

		DBusConnection* _connection;
		DBusMessage* _message;
		DBusMessageIter _itMessage;
		bool _hosedMessage;

		const std::string _appendFailure = "message append failure";

	private:
		void appendString(DBusMessageIter *iter, const std::string & value);
		void appendUInt8(DBusMessageIter *iter, const uint8_t value);
		void appendUInt16(DBusMessageIter *iter, const uint16_t value);
		void appendUInt64(DBusMessageIter *iter, const uint64_t value);
		void appendMacro(DBusMessageIter *iter, const GLogiK::macro_type & macro);
		void appendLCDPluginsPropertiesArray(
			DBusMessageIter *iter,
			const GLogiK::LCDPluginsPropertiesArray_type & pluginsArray
		);

};

} // namespace NSGKDBus

#endif
