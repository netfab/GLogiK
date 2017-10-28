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

#include <sstream>
#include <boost/filesystem.hpp>

#include <config.h>

#define UTILS_COMPILATION 1

#include "log.h"
#include "exception.h"
#include "filesystem.h"

#undef UTILS_COMPILATION

namespace fs = boost::filesystem;

namespace GLogiK
{

FileSystem::FileSystem() {
}

FileSystem::~FileSystem() {
}

void FileSystem::createOwnerDirectory(const std::string & directory) {
#if DEBUGGING_ON
	bool success = false;
#endif
	std::ostringstream buffer;

#if DEBUGGING_ON
	LOG(DEBUG2) << "trying to create directory : " << directory;
#endif

	try {
		fs::path path(directory);
#if DEBUGGING_ON
		success = fs::create_directory(path);
#else
		fs::create_directory(path);
#endif
		fs::permissions(path, fs::owner_all);
	}
	catch (const fs::filesystem_error & e) {
		buffer.str("directory creation or set permissions failure : ");
		buffer << directory << " : " << e.what();
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

} // namespace GLogiK

