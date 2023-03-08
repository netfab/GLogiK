/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
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

#include "GKDBusArgLCDPluginsArray.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

/*
 * helper function rebuilding LCDPPArray_type array
 */
const GLogiK::LCDPPArray_type GKDBusArgumentLCDPluginsArray::getNextLCDPluginsArrayArgument(void)
{
	GK_LOG_FUNC

	GKLog(trace, "rebuilding LCDPluginsProperties vector from GKDBus values")

	GLogiK::LCDPPArray_type pluginsArray;

	try {
		do {
			const uint64_t id = ArgUInt64::getNextUInt64Argument();
			const std::string name = ArgString::getNextStringArgument();
			const std::string desc = ArgString::getNextStringArgument();

			pluginsArray.push_back({id, name, desc});
		}
		while( ! ArgString::stringArguments.empty() );
	}
	catch ( const EmptyContainer & e ) {
		LOG(warning) << "missing argument : " << e.what();
		throw GLogiKExcept("rebuilding LCDPluginsProperties vector failed");
	}

	if( ! ArgUInt64::uint64Arguments.empty() ) { /* sanity check */
		LOG(warning) << "UInt64 container not empty";
	}

	GKLog2(trace, "LCDPluginsProperties vector size : ", pluginsArray.size())

	return pluginsArray;
}

} // namespace NSGKDBus

