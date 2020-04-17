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

#include <errno.h>

#include <syslog.h>

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

void FileSystem::createDirectory(
	const fs::path & directory,
	const fs::perms prms)
{
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

		if( prms != fs::no_perms ) {
			fs::permissions(directory, prms);
		}
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

void FileSystem::openFile(
	const fs::path & filePath,
	FILE* & pFile,
	const fs::perms prms)
{
	errno = 0;
	const std::string file = filePath.string();
	pFile = std::fopen(file.c_str(), "w");

	if(pFile == nullptr) {
		std::ostringstream buffer(std::ios_base::app);
		buffer << "failed to open file : " << file;
		if(errno != 0) {
			buffer << " : " << strerror(errno);
		}

		syslog(LOG_ERR, "%s", buffer.str().c_str());
	}
	else {
		if( prms != fs::no_perms ) {
			boost::system::error_code ec;
			fs::permissions(filePath, prms, ec);
			if( ec.value() != 0 ) {
				std::ostringstream buffer(std::ios_base::app);
				buffer << "failed to set file permissions : " << file
						<< " : " << ec.message();
				GKSysLog(LOG_ERR, ERROR, buffer.str());
			}
		}
	}
}

} // namespace NSGKUtils

