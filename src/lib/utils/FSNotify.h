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

#ifndef __GLOGIK_FS_NOTIFY_H__
#define __GLOGIK_FS_NOTIFY_H__

#include <map>
#include <string>

typedef std::map<const std::string, const std::string> files_map_t;

namespace NSGKUtils
{

// TODO see std::experimental::erase_if
template< typename ContainerT, typename PredicateT >
void erase_if( ContainerT& items, const PredicateT& predicate ) {
	for( auto it = items.begin(); it != items.end(); ) {
		if( predicate(*it) ) { it = items.erase(it); }
		else { ++it; }
	}
}

struct watchedObject {
	public:
		const int wd;
		unsigned int count;

		watchedObject(void) = delete;
		watchedObject(const int num) : wd(num), count(1) {}
		~watchedObject() = default;

	protected:

	private:
};


class FSNotify
{
	public:
		const int addNotifyDirectoryWatch(
			const std::string & path,
			const bool check_if_already_watched=false
		);
		void removeNotifyWatch(const int wd);

		const int getNotifyQueueDescriptor(void) const;

		void readNotifyEvents(files_map_t & files_map);


	protected:
		FSNotify(void);
		~FSNotify(void);

	private:
		int inotify_queue_fd_;

		std::map<const std::string, watchedObject> watch_descriptors_;

		const int addNotifyWatch(
			const std::string & path,
			const uint32_t & mask
		);
};

} // namespace NSGKUtils

#endif
