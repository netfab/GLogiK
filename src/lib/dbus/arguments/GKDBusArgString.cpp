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

#include "lib/utils/utils.h"

#include "GKDBusArgString.h"

namespace GLogiK
{

std::string CBStringArgument::current_string_("");

const std::string & CBStringArgument::getNextStringArgument(void) {
	if( CBArgument::string_arguments_.empty() )
		throw EmptyContainer("no string argument");
	CBStringArgument::current_string_ = CBArgument::string_arguments_.back();
	CBArgument::string_arguments_.pop_back();
	return CBStringArgument::current_string_;
}

const std::vector<std::string> & CBStringArgument::getAllStringArguments(void) {
	return CBArgument::string_arguments_;
}

} // namespace GLogiK

