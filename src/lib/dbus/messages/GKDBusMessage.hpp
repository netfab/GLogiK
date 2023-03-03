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

#include "include/base.hpp"
#include "include/MBank.hpp"
#include "include/LCDPluginProperties.hpp"

#include "lib/dbus/ArgTypes/TypeBase.hpp"
#include "lib/dbus/ArgTypes/boolean.hpp"
#include "lib/dbus/ArgTypes/string.hpp"
#include "lib/dbus/ArgTypes/stringArray.hpp"
#include "lib/dbus/ArgTypes/uint16.hpp"
#include "lib/dbus/ArgTypes/uint32.hpp"
#include "lib/dbus/ArgTypes/uint64.hpp"
#include "lib/dbus/ArgTypes/uint8.hpp"
#include "lib/dbus/ArgTypes/GKeysID.hpp"
#include "lib/dbus/ArgTypes/GKeysIDArray.hpp"

namespace NSGKDBus
{

class GKDBusMessage
	:	virtual public TypeBase,
		public TypeBoolean,
		public TypeString,
		public TypeStringArray,
		virtual public TypeUInt64,
		virtual public TypeUInt8,
		public TypeUInt16,
		public TypeUInt32,
		public TypeGKeysID,
		public TypeGKeysIDArray
{
	public:
		void appendMKeysID(const GLogiK::MKeysID keyID);
		void appendMKeysIDArray(const GLogiK::MKeysIDArray_type & keysID);
		void appendMacro(const GLogiK::macro_type & macro);
		void appendMacrosBank(const GLogiK::mBank_type & bank);
		void appendLCDPluginsPropertiesArray(
			const GLogiK::LCDPluginsPropertiesArray_type & pluginsArray
		);

	protected:
		GKDBusMessage(
			DBusConnection* const connection,
			const bool check=false
		);
		~GKDBusMessage(void);

		DBusConnection* _connection;
		DBusMessage* _message;

	private:
		void appendMacro(DBusMessageIter *iter, const GLogiK::macro_type & macro);
		void appendLCDPluginsPropertiesArray(
			DBusMessageIter *iter,
			const GLogiK::LCDPluginsPropertiesArray_type & pluginsArray
		);
};

} // namespace NSGKDBus

#endif
