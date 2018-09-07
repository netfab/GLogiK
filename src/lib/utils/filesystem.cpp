/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2018  Fabrice Delliaux <netbox253@gmail.com>
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

#include <sstream>

#include <config.h>

#define UTILS_COMPILATION 1

#include "log.hpp"
#include "exception.hpp"
#include "filesystem.hpp"

#undef UTILS_COMPILATION

namespace NSGKUtils
{

FileSystem::FileSystem()
{
}

FileSystem::~FileSystem() {
}

const std::string FileSystem::getNextAvailableFileName(
	const std::set<std::string> & toSkip,
	const fs::path & directory,
	const std::string & baseName,
	const std::string & extension,
	bool mustExist)
{
	unsigned int c = 0;
	while( c++ < 10 ) { /* bonus point */
		fs::path file(baseName);
		file += "_";
		file += std::to_string(c);
		file += ".";
		file += extension;

		const std::string fileName( file.string() );

		if( toSkip.count(fileName) == 1 ) {
#if DEBUGGING_ON
			LOG(DEBUG3) << "already used : " << fileName;
#endif
			continue;
		}

		if( mustExist ) {
			fs::path fullPath = directory / file;
			if( ! fs::is_regular_file(fullPath) ) {
#if DEBUGGING_ON
				LOG(DEBUG3) << "does not exist : " << fullPath.string();
#endif
				continue;
			}
		}

#if DEBUGGING_ON
		LOG(DEBUG2) << "returning new filename : " << fileName;
#endif
		return fileName;
	}

	throw GLogiKExcept("can't get new filename, counter reached max");
}

void FileSystem::createOwnerDirectory(const fs::path & directory) {
#if DEBUGGING_ON
	bool success = false;
	LOG(DEBUG2) << "creating directory : " << directory.string();
#endif

	try {
#if DEBUGGING_ON
		success = fs::create_directory( directory );
#else
		fs::create_directory( directory );
#endif
		fs::permissions(directory, fs::owner_all);
	}
	catch (const fs::filesystem_error & e) {
		std::ostringstream buffer;
		buffer.str("directory creation or set permissions failure : ");
		buffer << e.what();
		throw GLogiKExcept( buffer.str() );
	}

#if DEBUGGING_ON
	if( ! success ) {
		LOG(DEBUG3) << "directory not created because it seems already exists";
	}
	else {
		LOG(DEBUG3) << "directory successfully created";
	}
#endif
}

} // namespace NSGKUtils

