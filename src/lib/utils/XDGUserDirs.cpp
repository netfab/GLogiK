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

#include <cstdlib>
#include <string>

#define UTILS_COMPILATION 1

#include "exception.hpp"
#include "functions.hpp"
#include "XDGUserDirs.hpp"

#undef UTILS_COMPILATION

namespace NSGKUtils
{

XDGUserDirs::XDGUserDirs() {
}

XDGUserDirs::~XDGUserDirs() {
}

const fs::path XDGUserDirs::getConfigurationRootDirectory(void) {
	const std::string home = toString( getenv("HOME") );
	if(home == "")
		throw GLogiKExcept("can't get HOME environment variable");
	fs::path XDGConfigHome = toString( getenv("XDG_CONFIG_HOME") );
	if(XDGConfigHome == "") {
		XDGConfigHome = home;
		XDGConfigHome /= ".config";
	}
	return XDGConfigHome;
}

} // namespace NSGKUtils

