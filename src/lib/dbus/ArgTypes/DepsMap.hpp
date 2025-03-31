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

#ifndef SRC_LIB_DBUS_ARG_GKDBUS_ARGTYPES_GKDEPS_MAP_HPP_
#define SRC_LIB_DBUS_ARG_GKDBUS_ARGTYPES_GKDEPS_MAP_HPP_

#include <dbus/dbus.h>

#include "TypeBase.hpp"
#include "ArgBase.hpp"
#include "uint8.hpp"
#include "uint64.hpp"
#include "string.hpp"

#include "include/DepsMap.hpp"

namespace NSGKDBus
{

class TypeGKDepsMap
	:	virtual private TypeBase,
		virtual private TypeUInt8,
		virtual private TypeUInt64,
		virtual private TypeString
{
	public:
		void appendGKDepsMap(const GLogiK::GKDepsMap_type & depsMap);

	protected:
		TypeGKDepsMap(void) = default;
		~TypeGKDepsMap(void) = default;

		void appendGKDepsMap(DBusMessageIter *iter, const GLogiK::GKDepsMap_type & depsMap);

	private:
};

class ArgGKBinary
	:	virtual private ArgUInt8
{
	public:
		static const GLogiK::GKBinary getNextGKBinaryArgument(void);

	protected:
		ArgGKBinary(void) = default;
		~ArgGKBinary(void) = default;

	private:

};

class ArgGKDepsMap
	:	virtual protected ArgBase,
		virtual private ArgUInt8,
		virtual private ArgUInt64,
		virtual private ArgString
{
	public:
		static const GLogiK::GKDepsMap_type getNextGKDepsMapArgument(void);

	protected:
		ArgGKDepsMap(void) = default;
		~ArgGKDepsMap(void) = default;

	private:
};

} // namespace NSGKDBus

#endif
