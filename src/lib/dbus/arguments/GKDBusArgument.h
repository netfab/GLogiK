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

#ifndef __GLOGIK_GKDBUS_CALLBACK_ARGUMENT_H__
#define __GLOGIK_GKDBUS_CALLBACK_ARGUMENT_H__

#include <cstdint>

#include <string>
#include <vector>

#include <dbus/dbus.h>

namespace GLogiK
{

class GKDBusArgument
{
	public:

	protected:
		GKDBusArgument(void) = default;
		~GKDBusArgument(void) = default;

		static void fillInArguments(DBusMessage* message);

		thread_local static std::vector<std::string> string_arguments_;
		thread_local static std::vector<uint8_t> byte_arguments_;
		thread_local static std::vector<uint16_t> uint16_arguments_;
		thread_local static std::vector<bool> boolean_arguments_;

	private:
		static void decodeArgumentFromIterator(DBusMessageIter* iter, const char* signature, const unsigned int num);

};

} // namespace GLogiK

#endif