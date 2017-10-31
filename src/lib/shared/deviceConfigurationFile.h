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

#ifndef __GLOGIK_DEVICE_CONFIGURATION_FILE_H__
#define __GLOGIK_DEVICE_CONFIGURATION_FILE_H__

#include <string>
#include <vector>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace GLogiK
{

class DeviceConfigurationFile
{
	public:
		static const std::string getNextAvailableNewPath(const std::vector<std::string> & paths_to_skip,
			const fs::path & directory_path, const std::string & device_model, bool must_exist=false);

	protected:
		DeviceConfigurationFile(void);
		~DeviceConfigurationFile(void);

	private:
};

} // namespace GLogiK

#endif
