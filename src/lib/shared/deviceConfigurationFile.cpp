/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2019  Fabrice Delliaux <netbox253@gmail.com>
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

#include <exception>
#include <sstream>
#include <fstream>

#include <boost/archive/archive_exception.hpp>
#include <boost/archive/xml_archive_exception.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "lib/utils/utils.hpp"

#include "deviceConfigurationFile.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

DeviceConfigurationFile::DeviceConfigurationFile()
{
}

DeviceConfigurationFile::~DeviceConfigurationFile()
{
}

void DeviceConfigurationFile::load(
	const std::string & filePath,
	DeviceProperties & device) noexcept
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "loading device configuration file " << device.getConfigFilePath();
#endif
	try {
		std::ifstream ifs;
		ifs.exceptions(std::ifstream::badbit);
		ifs.open(filePath);
#if DEBUGGING_ON
		LOG(DEBUG2) << "configuration file successfully opened for reading";
#endif

		{
			DeviceProperties newDevice;
			boost::archive::text_iarchive inputArchive(ifs);
			inputArchive >> newDevice;

			device.setProperties( newDevice );
		}

#if DEBUGGING_ON
		LOG(DEBUG3) << "success, closing";
#endif
		ifs.close();
	}
	catch (const std::ifstream::failure & e) {
		std::ostringstream buffer("fail to open configuration file : ", std::ios_base::app);
		buffer << e.what();
		LOG(ERROR) << buffer.str();
	}
	catch(const boost::archive::archive_exception & e) {
		std::ostringstream buffer("boost::archive exception : ", std::ios_base::app);
		buffer << e.what();
		LOG(ERROR) << buffer.str();
		// TODO throw GLogiKExcept to create new configuration
		// file and avoid overwriting on close ?
	}
}

void DeviceConfigurationFile::save(
	const std::string & filePath,
	const DeviceProperties & device) noexcept
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "opening configuration file for writing : " << filePath;
#endif
	try {
		std::ofstream ofs;
		ofs.exceptions(std::ofstream::failbit|std::ofstream::badbit);
		ofs.open(filePath, std::ofstream::out|std::ofstream::trunc);
#if DEBUGGING_ON
		LOG(DEBUG3) << "opened";
#endif

		{
			boost::archive::text_oarchive outputArchive(ofs);
			outputArchive << device;
		}

		LOG(INFO) << "successfully saved configuration file, closing";
		ofs.close();
	}
	catch (const std::ofstream::failure & e) {
		std::ostringstream buffer("fail to open configuration file : ", std::ios_base::app);
		buffer << e.what();
		LOG(ERROR) << buffer.str();
	}
	catch(const boost::archive::archive_exception & e) {
		std::ostringstream buffer("boost::archive exception : ", std::ios_base::app);
		buffer << e.what();
		LOG(ERROR) << buffer.str();
	}
}

} // namespace GLogiK

