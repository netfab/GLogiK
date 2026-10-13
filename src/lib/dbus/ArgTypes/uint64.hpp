/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2026  Fabrice Delliaux <netbox253@gmail.com>
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

#pragma once

#include <cstdint>

#include <dbus/dbus.h>

#include "TypeBase.hpp"
#include "ArgBase.hpp"

namespace GKDBUS::Types
{

class TypeUInt64
	:	virtual private TypeBase
{
	public:
		void appendUInt64(std::uint64_t value);

	protected:
		TypeUInt64() = default;
		~TypeUInt64() = default;

		void appendUInt64(
			DBusMessageIter *iter,
			std::uint64_t value
		);

	private:
};

class ArgUInt64
	:	virtual protected ArgBase
{
	public:
		static std::uint64_t getNextUInt64Argument();

	protected:
		ArgUInt64() = default;
		~ArgUInt64() = default;

	private:

};

} // namespace GKDBUS::Types
