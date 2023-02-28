/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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

#include "GKDBusArgDevicesMap.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

/*
 * helper function rebuilding DevicesMap_type map
 */
const GLogiK::DevicesMap_type GKDBusArgumentDevicesMap::getNextDevicesMapArgument(void)
{
	GK_LOG_FUNC

	GKLog(trace, "rebuilding DevicesMap_type map from GKDBus values")

	GLogiK::DevicesMap_type devicesMap;

	std::string devID;

	/* handle case where DevicesMap is empty */
	try {
		devID = GKDBusArgumentString::getNextStringArgument();

		try {
			do {
				GLogiK::DeviceID device;

				/* don't get string on first iteration */
				if( devID.empty() )
					devID = GKDBusArgumentString::getNextStringArgument();

				device.setStatus( GKDBusArgumentString::getNextStringArgument() );
				device.setVendor( GKDBusArgumentString::getNextStringArgument() );
				device.setProduct( GKDBusArgumentString::getNextStringArgument() );
				device.setName( GKDBusArgumentString::getNextStringArgument() );
				device.setConfigFilePath( GKDBusArgumentString::getNextStringArgument() );

				devicesMap[devID] = device;

				/* for next loop iteration */
				devID.clear();
			}
			while( ! GKDBusArgumentString::stringArguments.empty() );
		}
		catch ( const EmptyContainer & e ) {
			LOG(warning) << "missing argument : " << e.what();
			throw GLogiKExcept("rebuilding DevicesMap_type map failed");
		}
	}
	catch ( const EmptyContainer & e ) {
		GKLog(trace, "no devID, empty devicesMap ?")
	}

	GKLog2(trace, "DevicesMap_type map size : ", devicesMap.size())

	return devicesMap;
}

} // namespace NSGKDBus

