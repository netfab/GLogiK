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


#include <cerrno>
#include <cstring>
#include <cstdint>
#include <unistd.h>

#include <set>
#include <sstream>

#include <sys/inotify.h>

#include <config.h>


#define UTILS_COMPILATION 1

#include "GKLogging.hpp"
#include "exception.hpp"
#include "FSNotify.hpp"

#undef UTILS_COMPILATION


namespace NSGKUtils
{

FSNotify::FSNotify()
	:	_inotifyQueueFD(0)
{
	_inotifyQueueFD = inotify_init1(IN_NONBLOCK);
	if( _inotifyQueueFD == -1 ) {
		std::ostringstream buffer("inotify init failure : ", std::ios_base::app);
		buffer << strerror(errno);
		throw GLogiKExcept( buffer.str() );
	}
}

FSNotify::~FSNotify()
{
	GK_LOG_FUNC

	/* sanity check */
	if( ! _watchedDescriptorsMap.empty() ) {
		LOG(warning) << "some watch descriptors were not removed";
		for(const auto & watchedPair : _watchedDescriptorsMap) {
			this->removeNotifyWatch(watchedPair.second.wd);
		}
		_watchedDescriptorsMap.clear();
	}

	/* closing queue */
	if(_inotifyQueueFD > -1) {
		if( close(_inotifyQueueFD) == -1 ) {
			LOG(error) << "inotify queue closing failure : " << strerror(errno);
		}
	}
}

/* make sure path is readable before calling this one */
const int FSNotify::addNotifyDirectoryWatch(
	const std::string & path,
	const bool checkIfAlreadyWatched)
{
	GK_LOG_FUNC

	/* check first if this path is already watched */
	/* returns its descriptor in this case */
	if( checkIfAlreadyWatched ) {
		auto it = _watchedDescriptorsMap.find(path);
		if ( it != _watchedDescriptorsMap.end() ) {
			GKLog2(trace, "path already watched : ", path)
			return (*it).second.wd;
		}
	}

	uint32_t mask = IN_CLOSE_WRITE | IN_DELETE_SELF | IN_MOVE_SELF | IN_ONLYDIR;
	return this->addNotifyWatch(path, mask);
}

void FSNotify::removeNotifyWatch(const int wd)
{
	GK_LOG_FUNC

	if(wd == -1) {
		LOG(error) << "[" << wd << "] - negative watch descriptor";
		return;
	}

	const int & fd = _inotifyQueueFD;
	auto findDescriptor = [&fd, &wd] (auto & item) -> const bool {
		WatchedObject & watched = item.second;
		if(watched.wd != wd)
			return false;
		--watched.count;
		if(watched.count == 0) {
			if( inotify_rm_watch(fd, wd) == -1 ) {
				LOG(error) << "inotify rm watch failure : " << strerror(errno);
			}
			else {
				GKLog3(trace, wd, " | removed path notify watch", item.first)
			}
			return true;
		}
		GKLog3(trace, wd, " | decremented reference to path notify watch", item.first)
		return false;
	};

	erase_if(_watchedDescriptorsMap, findDescriptor);
}

const int FSNotify::getNotifyQueueDescriptor(void) const
{
	return _inotifyQueueFD;
}

void FSNotify::readNotifyEvents(DevicesFilesMap_type & filesMap)
{
	GK_LOG_FUNC

	char *ptr;
	ssize_t len;
	char buf[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
	const struct inotify_event *event;

	std::set<std::string> toRemove;

	for (;;) {
		/* Read some events. */
		len = read(_inotifyQueueFD, buf, sizeof buf);
		if (len == -1 && errno != EAGAIN) {
			throw GLogiKExcept("read error");
		}

		if (len <= 0)
			break;

		for (ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
			event = (const struct inotify_event *) ptr;

			std::string wd("[");
			wd += std::to_string(event->wd);
			wd += "]";

			/* file inside watched directory */
			if (event->len) {
				const std::string name(event->name);
				GKLog3(trace, wd, " | name : ", name)
				/* map size will be 0 or 1 after that */
				erase_if(filesMap, [&name]( auto & item ) -> const bool { return item.second != name; } );
			}
			else { /* watched object event */
				filesMap.clear();

				std::string path;
				for(const auto & itemPair : _watchedDescriptorsMap) {
					if(itemPair.second.wd == event->wd) {
						path = itemPair.first;
						break;
					}
				}
				if( path.empty() ) {
					/*
					 * on next readNotifyEvents call IN_IGNORED can be generated by
					 * current IN_MOVE_SELF since we force remove the watch below
					 */
					if( event->mask & IN_IGNORED ) {
						GKLog2(trace, wd, " | [IN_IGNORED] watch was removed")
					}
					else {
						GKLog2(trace, wd, " | skip event : path not found")
					}
					continue; /* next event */
				}

				if( event->mask & IN_MOVE_SELF ) {
					GKLog3(trace, wd, " | [IN_MOVE_SELF] watched object renamed or moved : ", path)
					toRemove.insert(path);
				}
				if( event->mask & IN_DELETE_SELF ) {
					GKLog3(trace, wd, " | [IN_DELETE_SELF] watched object deleted : ", path)
				}
				if( event->mask & IN_IGNORED ) {
					GKLog3(trace, wd, " | [IN_IGNORED] watch was removed : ", path)
					_watchedDescriptorsMap.erase(path);
				}
			}
		}

		for(const auto & path : toRemove) {
			auto it = _watchedDescriptorsMap.find(path);
			if(it != _watchedDescriptorsMap.end()) {
				(*it).second.count = 1; /* force removing watch */
				this->removeNotifyWatch((*it).second.wd);
			}
		}
		toRemove.clear();
	}
}

/*
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 *
 * === private === private === private === private === private ===
 *
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 */

const int FSNotify::addNotifyWatch(const std::string & path, const uint32_t & mask)
{
	GK_LOG_FUNC

	auto it = _watchedDescriptorsMap.find(path);

	if(it == _watchedDescriptorsMap.end()) {
		const int ret = inotify_add_watch(_inotifyQueueFD, path.c_str(), mask);
		if(ret < 0) {
			std::ostringstream buffer("inotify path watch failure : ", std::ios_base::app);
			buffer << path << " - " << strerror(errno);
			throw GLogiKExcept( buffer.str() );
		}

		_watchedDescriptorsMap.insert( std::pair<const std::string, WatchedObject>(path, WatchedObject(ret)) );
		it = _watchedDescriptorsMap.find(path);

		GKLog3(trace, (*it).second.wd, " | added path notify watch : ", path)
	}
	else {
		(*it).second.count++;

		GKLog3(trace, (*it).second.wd, " | incremented reference to path notify watch : ", path)
	}

	return (*it).second.wd;
}


} // namespace NSGKUtils

