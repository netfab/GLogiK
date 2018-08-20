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

#ifndef SRC_LIB_UTILS_FS_NOTIFY_HPP_
#define SRC_LIB_UTILS_FS_NOTIFY_HPP_

#include <map>
#include <string>

typedef std::map<const std::string, const std::string> files_map_type;

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

struct WatchedObject {
	public:
		const int wd;
		unsigned int count;

		WatchedObject(void) = delete;
		WatchedObject(const int num) : wd(num), count(1) {}
		~WatchedObject() = default;

	protected:

	private:
};


class FSNotify
{
	public:
		const int addNotifyDirectoryWatch(
			const std::string & path,
			const bool checkIfAlreadyWatched=false
		);
		void removeNotifyWatch(const int wd);

		const int getNotifyQueueDescriptor(void) const;

		void readNotifyEvents(files_map_type & filesMap);


	protected:
		FSNotify(void);
		~FSNotify(void);

	private:
		int _inotifyQueueFD;

		std::map<const std::string, WatchedObject> _watchedDescriptorsMap;

		const int addNotifyWatch(
			const std::string & path,
			const uint32_t & mask
		);
};

} // namespace NSGKUtils

#endif
