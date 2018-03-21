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

#ifndef __GLOGIK_FILESYSTEM_H__
#define __GLOGIK_FILESYSTEM_H__

#if !defined (UTILS_INSIDE_UTILS_H) && !defined (UTILS_COMPILATION)
#error "Only "utils/utils.h" can be included directly, this file may disappear or change contents."
#endif

#include <set>
#include <map>
#include <string>
#include <boost/filesystem.hpp>

#include "FSNotify.h"

namespace fs = boost::filesystem;

namespace NSGKUtils
{

class FileSystem
	:	public FSNotify
{
	public:
		FileSystem(void);
		~FileSystem(void);

		static void createOwnerDirectory(const fs::path & directory);
		const std::string getNextAvailableFileName(
			const std::set<std::string> & to_skip,
			const fs::path & directory,
			const std::string & basename,
			const std::string & extension,
			bool must_exist=false
		);

	protected:

	private:
};

} // namespace NSGKUtils

#endif
