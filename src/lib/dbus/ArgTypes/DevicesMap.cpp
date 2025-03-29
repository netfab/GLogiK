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

#include <utility>

#include "lib/utils/utils.hpp"

#include "DevicesMap.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

/*
 * helper function to rebuild DevicesMap_type map from GKDBus stringArray
 */
const GLogiK::DevicesMap_type ArgDevicesMap::getNextDevicesMapArgument(void)
{
	GK_LOG_FUNC

	GKLog(trace, "rebuilding DevicesMap_type map from GKDBus values")

	GLogiK::DevicesMap_type devicesMap;

	const std::vector<std::string> stringArray( ArgStringArray::getNextStringArray() );

	using Size = std::vector<std::string>::size_type;

	Size s = stringArray.size();
	GKLog2(trace, "DeviceID stringArray size : ", s)
	if( (s % (DEVICE_ID_NUM_PROPERTIES + 1)) != 0 ) {
		LOG(error) << "vector size: " << s
			<< " - wanted modulo: " << (DEVICE_ID_NUM_PROPERTIES + 1);
		throw GLogiKExcept("wrong string array size");
	}

	Size i = 0;
	while(i < s) {
		/* see DevicesHandler::getDevicesList() in service */
		auto & devID	= stringArray[0 + i];
		auto & status	= stringArray[1 + i];
		auto & vendor	= stringArray[2 + i];
		auto & product	= stringArray[3 + i];
		auto & name		= stringArray[4 + i];
		auto & filePath	= stringArray[5 + i];

		//GKLog6(trace, devID, vendor, product, name, status, filePath)

		devicesMap.insert(
			std::pair<std::string, GLogiK::DeviceID>(
				devID, {vendor, product, name, status, filePath})
		);
		i += (DEVICE_ID_NUM_PROPERTIES + 1);
	}

	GKLog4(trace, "returning DevicesMap_type map size: ", devicesMap.size(),
		"expected: ", (s / (DEVICE_ID_NUM_PROPERTIES + 1)))
	return devicesMap;
}

} // namespace NSGKDBus
