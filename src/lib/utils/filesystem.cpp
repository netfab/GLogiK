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

#include <syslog.h>

#include <cstdint>

#include <algorithm>
#include <sstream>

#include <config.h>

#define UTILS_COMPILATION 1

#include "GKLogging.hpp"
#include "exception.hpp"
#include "filesystem.hpp"

#undef UTILS_COMPILATION

namespace NSGKUtils
{

FileSystem::FileSystem()
{
}

FileSystem::~FileSystem()
{
}

bool FileSystem::createDirectorySuccess = false;
std::string FileSystem::lastDirectoryCreation;

const std::string FileSystem::getNextAvailableFileName(
	const std::set<std::string> & toSkip,
	const fs::path & directory,
	const std::string & baseName,
	const std::string & extension,
	bool mustExist)
{
	GK_LOG_FUNC

	uint16_t c = 0;

	std::string base(baseName);
	std::replace( base.begin(), base.end(), '/', '_');
	std::replace( base.begin(), base.end(), ' ', '_');

	while( c++ < 10 ) { /* bonus point */
		fs::path file(base);
		file += "_";
		file += std::to_string(c);
		file += ".";
		file += extension;

		const std::string fileName( file.string() );

		if( toSkip.count(fileName) == 1 ) {
			GKLog2(trace, "already used : ", fileName)
			continue;
		}

		if( mustExist ) {
			fs::path fullPath = directory / file;
			try {
				if( ! fs::is_regular_file(fullPath) ) {
					GKLog2(trace, "does not exist : ", fullPath.string())
					continue;
				}
			}
			catch (const fs::filesystem_error & e) {
				LOG(warning) << "boost::filesystem::is_regular_file() error : " << e.what();
				continue;
			}
			catch (const std::exception & e) {
				LOG(warning) << "boost::filesystem::is_regular_file() (allocation) error : " << e.what();
				continue;
			}
		}

		GKLog2(trace, "returning new filename : ", fileName)
		return fileName;
	}

	throw GLogiKExcept("can't get new filename, counter reached max");
}

void FileSystem::createDirectory(
	const fs::path & directory,
	const fs::perms prms)
{
	FileSystem::createDirectorySuccess = false;
	lastDirectoryCreation = directory.string();

	auto throwError = [] (
		const std::string & error,
		const char* what
			) -> void {
		std::ostringstream buffer(error, std::ios_base::app);
		buffer << " : " << what;
		throw GLogiKExcept( buffer.str() );
	};

	try {
		FileSystem::createDirectorySuccess = fs::create_directory( directory );
		if( prms != fs::no_perms ) {
			fs::permissions(directory, prms);
		}
	}
	catch (const fs::filesystem_error & e) {
		throwError("directory creation or set permissions failure", e.what());
	}
	catch (const std::exception & e) {
		throwError("directory creation or set permissions (allocation) failure", e.what());
	}
}

#if DEBUGGING_ON
void FileSystem::traceLastDirectoryCreation(void)
{
	GK_LOG_FUNC

	if(GKLogging::GKDebug) {
		if(FileSystem::createDirectorySuccess) {
			LOG(trace) << "created directory : " << lastDirectoryCreation;
		}
		else {
			LOG(trace) << "directory not created because it seems already exists : " << lastDirectoryCreation;
		}
	}
}
#endif

} // namespace NSGKUtils

