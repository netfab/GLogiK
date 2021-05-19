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
 * helper function rebuilding LCDPluginsPropertiesArray_type array
 */
const GLogiK::LCDPluginsPropertiesArray_type GKDBusArgumentLCDPluginsArray::getNextLCDPluginsArrayArgument(void)
{
	GK_LOG_FUNC

#if DEBUGGING_ON
	LOG(DEBUG2) << "rebuilding LCDPluginsProperties vector from GKDBus values";
#endif
	GLogiK::LCDPluginsPropertiesArray_type pluginsArray;

	try {
		do {
			const uint64_t id = GKDBusArgumentUInt64::getNextUInt64Argument();
			const std::string name = GKDBusArgumentString::getNextStringArgument();
			const std::string desc = GKDBusArgumentString::getNextStringArgument();

			pluginsArray.push_back({id, name, desc});
		}
		while( ! GKDBusArgumentString::stringArguments.empty() );
	}
	catch ( const EmptyContainer & e ) {
		LOG(WARNING) << "missing argument : " << e.what();
		throw GLogiKExcept("rebuilding LCDPluginsProperties vector failed");
	}

	if( ! GKDBusArgumentUInt64::uint64Arguments.empty() ) { /* sanity check */
		LOG(WARNING) << "UInt64 container not empty";
	}

#if DEBUGGING_ON
	LOG(DEBUG3) << "LCDPluginsProperties vector size : " << pluginsArray.size();
#endif
	return pluginsArray;
}

} // namespace NSGKDBus

