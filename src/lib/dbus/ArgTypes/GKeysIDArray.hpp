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

#include "TypeBase.hpp"
#include "uint8.hpp"
#include "GKeysID.hpp"

#include "include/base.hpp"

namespace GKDBUS::Types
{

class TypeGKeysIDArray
	:	virtual private TypeBase,
		virtual private TypeUInt8
{
	public:
		void appendGKeysIDArray(const GLogiK::GKeysIDArray_type & keysID);

	protected:
		TypeGKeysIDArray() = default;
		~TypeGKeysIDArray() = default;

	private:
};

class ArgGKeysIDArray
	:	virtual private ArgUInt8,
		virtual protected ArgGKeysID
{
	public:
		static const GLogiK::GKeysIDArray_type getNextGKeysIDArrayArgument();

	protected:
		ArgGKeysIDArray() = default;
		~ArgGKeysIDArray() = default;

	private:
};

} // namespace GKDBUS::Types
