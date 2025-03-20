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

#ifndef SRC_LIB_DBUS_MSG_GKDBUS_ARGTYPES_ARGBASE_HPP_
#define SRC_LIB_DBUS_MSG_GKDBUS_ARGTYPES_ARGBASE_HPP_

#include <cstdint>

#include <algorithm>
#include <string>
#include <vector>

#include <dbus/dbus.h>

namespace NSGKDBus
{

class ArgBase
{
	public:

	protected:
		ArgBase(void) = default;
		~ArgBase(void) = default;

		static void fillInArguments(DBusMessage* message);
		static const int decodeNextArgument(DBusMessageIter* itArgument);

		thread_local static std::vector<std::string> stringArguments;
		thread_local static std::vector<int32_t> int32Arguments;
		thread_local static std::vector<uint8_t> byteArguments;
		thread_local static std::vector<uint16_t> uint16Arguments;
		thread_local static std::vector<uint64_t> uint64Arguments;
		thread_local static std::vector<bool> booleanArguments;

		template<typename T, typename A>
		static void reverse(std::vector<T, A> & ctn)
		{
			if( ! ctn.empty() )
				std::reverse(ctn.begin(), ctn.end());
		}

	private:
		static void decodeArgumentFromIterator(
			DBusMessageIter* iter,
			const char* signature,
			const uint16_t num
		);

};

} // namespace NSGKDBus

#endif
