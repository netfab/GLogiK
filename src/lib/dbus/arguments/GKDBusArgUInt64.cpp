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

#include "lib/utils/utils.hpp"

#include "GKDBusArgUInt64.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

const uint64_t GKDBusArgumentUInt64::getNextUInt64Argument(void) {
	if( GKDBusArgument::uint64Arguments.empty() )
		throw EmptyContainer("missing argument : uint64");
	const uint64_t ret = GKDBusArgument::uint64Arguments.back();
	GKDBusArgument::uint64Arguments.pop_back();
	return ret;
}

} // namespace NSGKDBus

