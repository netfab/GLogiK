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

#include <errno.h>

#include <cstring>
#include <cstdint>

#include <sstream>

#include <sys/inotify.h>

#include <config.h>

#define UTILS_COMPILATION 1

#include "log.h"
#include "exception.h"
#include "filesystem.h"

#undef UTILS_COMPILATION

namespace NSGKUtils
{

FileSystem::FileSystem() :
	inotify_queue_fd_(0)
{
	this->inotify_queue_fd_ = inotify_init();
	if( this->inotify_queue_fd_ == -1 ) {
		std::ostringstream buffer("inotify init failure : ", std::ios_base::app);
		buffer << strerror(errno);
		throw GLogiKExcept( buffer.str() );
	}
}

FileSystem::~FileSystem() {
	/* sanity check */
	if( ! this->watch_descriptors_.empty() ) {
		LOG(WARNING) << "some watch descriptors were not removed";
		for(const int &wd : this->watch_descriptors_) {
			this->notifyRemoveFile(wd);
		}
		this->watch_descriptors_.clear();
	}

	/* closing queue */
	if(this->inotify_queue_fd_ > -1) {
		if( close(this->inotify_queue_fd_) == -1 ) {
			LOG(ERROR) << "inotify queue closing failure : " << strerror(errno);
		}
	}
}

const int FileSystem::notifyWatchFile(const std::string & filePath) {
	uint32_t mask = IN_DELETE_SELF | IN_MODIFY;
	int ret = inotify_add_watch(this->inotify_queue_fd_, filePath.c_str(), mask);
	if(ret < 0) {
		std::ostringstream buffer("inotify watch file failure : ", std::ios_base::app);
		buffer << strerror(errno);
		throw GLogiKExcept( buffer.str() );
	}
	this->watch_descriptors_.insert(ret);
#if DEBUGGING_ON
	LOG(DEBUG2) << "added file notify - " << filePath;
#endif
	return ret;
}

void FileSystem::notifyRemoveFile(const int wd) {
	if(wd > -1) {
		if( inotify_rm_watch(this->inotify_queue_fd_, wd) == 0 ) {
			this->watch_descriptors_.erase(wd);
#if DEBUGGING_ON
			LOG(DEBUG2) << "removed file notify";
#endif
			return;
		}
		LOG(ERROR) << "removed file notify failure : " << strerror(errno);
		return;
	}
	LOG(ERROR) << "negative watch descriptor";
}

void FileSystem::createOwnerDirectory(const fs::path & directory) {
#if DEBUGGING_ON
	bool success = false;
	LOG(DEBUG2) << "trying to create directory : " << directory.string();
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

