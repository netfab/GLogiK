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

#include "include/base.hpp"

#include <dbus/dbus.h>

#include "ArgBase.hpp"
#include "TypeBase.hpp"

#include "uint16.hpp"
#include "uint8.hpp"

namespace GKDBUS::Types
{

class TypeMacro
	:	virtual private TypeBase,
		virtual private TypeUInt16,
		virtual private TypeUInt8
{
	public:
		void appendMacro(const GLogiK::macro_type & macro);

	protected:
		TypeMacro() = default;
		~TypeMacro() = default;

		void appendMacro(
			DBusMessageIter *iter,
			const GLogiK::macro_type & macro
		);

	private:
};


class ArgMacro
	:	virtual protected ArgBase,
		virtual private ArgUInt8,
		virtual private ArgUInt16
{
	public:
		static GLogiK::macro_type getNextMacroArgument(unsigned int macroSize = 0);

	protected:
		ArgMacro() = default;
		~ArgMacro() = default;

	private:

};

} // namespace GKDBUS::Types
