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

#include <dbus/dbus.h>

#include "TypeBase.hpp"
#include "ArgBase.hpp"

#include "uint64.hpp"
#include "string.hpp"

#include "include/LCDPP.hpp"

namespace GKDBUS::Types
{

class TypeLCDPPArray
	:	virtual private TypeBase,
		virtual private TypeUInt64,
		virtual private TypeString
{
	public:
		void appendLCDPPArray(const GLogiK::LCDPPArray_type & pluginsArray);

	protected:
		TypeLCDPPArray() = default;
		~TypeLCDPPArray() = default;

		void appendLCDPPArray(
			DBusMessageIter *iter,
			const GLogiK::LCDPPArray_type & pluginsArray
		);

	private:
};


class ArgLCDPPArray
	:	virtual protected ArgBase,
		virtual private ArgUInt64,
		virtual private ArgString
{
	public:
		static const GLogiK::LCDPPArray_type getNextLCDPPArrayArgument();

	protected:
		ArgLCDPPArray() = default;
		~ArgLCDPPArray() = default;

	private:

};

} // namespace GKDBUS::Types
