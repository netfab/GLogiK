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

#ifndef SRC_LIB_DBUS_MSG_GKDBUS_ARGTYPES_STRING_HPP_
#define SRC_LIB_DBUS_MSG_GKDBUS_ARGTYPES_STRING_HPP_

#include <string>
#include <vector>

#include <dbus/dbus.h>

#include "TypeBase.hpp"
#include "ArgBase.hpp"
#include "uint64.hpp"

namespace NSGKDBus
{

class TypeString
	:	virtual private TypeBase,
		virtual private TypeUInt64
{
	public:
		void appendString(const std::string & value);

	protected:
		TypeString(void) = default;
		~TypeString(void) = default;

		void appendString(DBusMessageIter *iter, const std::string & value);

	private:
};

class ArgString
	:	virtual protected ArgBase,
		virtual private ArgUInt64
{
	public:
		static const std::string & getNextStringArgument(void);

	protected:
		ArgString(void) = default;
		~ArgString(void) = default;

	private:
		thread_local static std::string currentString;

};

} // namespace NSGKDBus

#endif
