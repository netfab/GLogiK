/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2017  Fabrice Delliaux <netbox253@gmail.com>
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

#include <config.h>

#include "lib/utils/utils.h"

#include "deviceConfigurationFile.h"

namespace GLogiK
{

DeviceConfigurationFile::DeviceConfigurationFile() {
}

DeviceConfigurationFile::~DeviceConfigurationFile() {
}

const std::string DeviceConfigurationFile::getNextAvailableNewPath(const std::vector<std::string> & paths_to_skip,
	const fs::path & directory_path, const std::string & device_model, bool must_exist)
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "building next new path";
#endif

	unsigned int c = 0;
	while( c++ < 10 ) { /* bonus point */
		fs::path file(device_model);
		file += "_";
		file += std::to_string(c);
		file += ".cfg";

		fs::path full_path = directory_path / file;

		bool used = false;
		const std::string path( full_path.string() );
		for( const auto & p : paths_to_skip ) {
			if( p == path )
				used = true;
		}
		if( used ) {
#if DEBUGGING_ON
			LOG(DEBUG3) << "already used : " << path;
#endif
			continue;
		}

		if( must_exist ) {
			if( ! fs::is_regular_file(full_path) ) {
#if DEBUGGING_ON
				LOG(DEBUG3) << "does not exist : " << path;
#endif
				continue;
			}
		}

#if DEBUGGING_ON
		LOG(DEBUG2) << "returning new path : " << path;
#endif
		return path;
	}

	throw GLogiKExcept("can't get new path, counter reached max");
}

} // namespace GLogiK

