/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2020  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_LIB_DBUS_ARG_GKDBUS_ARG_DEVICES_MAP_HPP_
#define SRC_LIB_DBUS_ARG_GKDBUS_ARG_DEVICES_MAP_HPP_

#include "GKDBusArgString.hpp"

#include "include/device.hpp"

namespace NSGKDBus
{

class GKDBusArgumentDevicesMap
	:	virtual private GKDBusArgumentString
{
	public:
		static const GLogiK::devices_map_type getNextDevicesMapArgument(void);

	protected:
		GKDBusArgumentDevicesMap(void) = default;
		~GKDBusArgumentDevicesMap(void) = default;

	private:
};

} // namespace NSGKDBus

#endif
