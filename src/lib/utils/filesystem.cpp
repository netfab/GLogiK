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

#include <cstring>
#include <cstdlib>

#include <algorithm>
#include <sstream>
#include <chrono>

#include <config.h>

#define UTILS_COMPILATION 1

#include "log.hpp"
#include "exception.hpp"
#include "filesystem.hpp"

#undef UTILS_COMPILATION

namespace chr = std::chrono;

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
#if DEBUGGING_ON
			LOG(DEBUG3) << "already used : " << fileName;
#endif
			continue;
		}

		if( mustExist ) {
			fs::path fullPath = directory / file;
			try {
				if( ! fs::is_regular_file(fullPath) ) {
#if DEBUGGING_ON
					LOG(DEBUG3) << "does not exist : " << fullPath.string();
#endif
					continue;
				}
			}
			catch (const fs::filesystem_error & e) {
				LOG(WARNING) << "boost::filesystem::is_regular_file() error : " << e.what();
				continue;
			}
			catch (const std::exception & e) {
				LOG(WARNING) << "boost::filesystem::is_regular_file() (allocation) error : " << e.what();
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

	auto throwError = [] (
		const std::string & error,
		const char* what
			) -> void {
		std::ostringstream buffer(error, std::ios_base::app);
		buffer << " : " << what;
		throw GLogiKExcept( buffer.str() );
	};

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
		throwError("directory creation or set permissions failure", e.what());
	}
	catch (const std::exception & e) {
		throwError("directory creation or set permissions (allocation) failure", e.what());
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

#if DEBUGGING_ON
void FileSystem::openDebugFile(
	const std::string & baseName,
	FILE* & pFile,
	const fs::perms prms,
	const bool createDir
	)
{
	if( LOG_TO_FILE_AND_CONSOLE::FileReportingLevel() == NONE ) {
		if( pFile == nullptr) {
			syslog(LOG_INFO, "debug file not opened");
			return;
		}
	}

	fs::path debugFile(DEBUG_DIR);

	if( createDir )
		FileSystem::createDirectory(debugFile, fs::owner_all | fs::group_all);

	{
		chr::seconds s = chr::duration_cast< chr::seconds >(
			chr::system_clock::now().time_since_epoch()
		);

		debugFile /= baseName;
		debugFile += "-";
		debugFile += std::to_string(s.count());
	}

	debugFile += "-XXXXXX.log";

	int fd = -1;
	std::string filePath;

	{
		const std::string f( debugFile.string() );
		char char_array[f.size() + 1];

		std::strcpy(char_array, f.c_str());

		fd = mkstemps(char_array, 4);
		filePath = char_array;
	}

	auto openError = [&filePath] (const std::string & error) -> void {
		std::ostringstream buffer(std::ios_base::app);
		buffer << error << " : " << filePath;
		if(errno != 0) {
			buffer << " : " << strerror(errno);
		}

		syslog(LOG_ERR, "%s", buffer.str().c_str());
	};

	if( fd != -1 ) {
		pFile = fdopen(fd, "w");
		if(pFile != nullptr) {
			if( prms != fs::no_perms ) {
				boost::system::error_code ec;
				fs::permissions(filePath, prms, ec);
				if( ec.value() != 0 ) {
					std::ostringstream buffer(std::ios_base::app);
					buffer << "failed to set file permissions : " << filePath
							<< " : " << ec.message();
					GKSysLog(LOG_ERR, ERROR, buffer.str());
				}
			}

			LOG_TO_FILE_AND_CONSOLE::FileStream() = pFile;
		}
		else openError("fdopen error");
	}
	else openError("mkstemps error");

	// tried to open debug file but something goes wrong
	if( pFile == nullptr)
		syslog(LOG_INFO, "debug file not opened");
}
#endif

} // namespace NSGKUtils

