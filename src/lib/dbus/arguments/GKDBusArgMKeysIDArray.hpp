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

#ifndef SRC_LIB_DBUS_ARG_GKDBUS_ARG_MKEYS_ID_ARRAY_HPP_
#define SRC_LIB_DBUS_ARG_GKDBUS_ARG_MKEYS_ID_ARRAY_HPP_

#include "include/base.hpp"

#include "GKDBusArgMKeysID.hpp"

namespace NSGKDBus
{

class GKDBusArgumentMKeysIDArray
	:	virtual private GKDBusArgumentByte,
		virtual private GKDBusArgumentMKeysID
{
	public:
		static const GLogiK::MKeysIDArray_type getNextMKeysIDArrayArgument(void);

	protected:
		GKDBusArgumentMKeysIDArray(void) = default;
		~GKDBusArgumentMKeysIDArray(void) = default;

	private:
};

} // namespace NSGKDBus

#endif