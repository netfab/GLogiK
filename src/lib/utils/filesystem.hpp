/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_LIB_UTILS_FILESYSTEM_HPP_
#define SRC_LIB_UTILS_FILESYSTEM_HPP_

#if !defined (UTILS_INSIDE_UTILS_H) && !defined (UTILS_COMPILATION)
#error "Only "utils/utils.hpp" can be included directly, this file may disappear or change contents."
#endif

#include <set>
#include <string>
#include <boost/filesystem.hpp>

#include "FSNotify.hpp"

namespace fs = boost::filesystem;

namespace NSGKUtils
{

class FileSystem
	:	public FSNotify
{
	public:
		FileSystem(void);
		~FileSystem(void);

		static void createDirectory(
			const fs::path & directory,
			const fs::perms prms = fs::no_perms
		);

#if DEBUGGING_ON
		static void traceLastDirectoryCreation(void);
#endif

		const std::string getNextAvailableFileName(
			const std::set<std::string> & toSkip,
			const fs::path & directory,
			const std::string & baseName,
			const std::string & extension,
			bool mustExist=false
		);

	protected:

	private:
		static bool createDirectorySuccess;
		static std::string lastDirectoryCreation;
};

} // namespace NSGKUtils

#endif
