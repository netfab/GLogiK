/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2019  Fabrice Delliaux <netbox253@gmail.com>
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

#include "lib/utils/utils.hpp"

#include "GKDBusArgString.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

thread_local std::string GKDBusArgumentString::currentString("");

const std::string & GKDBusArgumentString::getNextStringArgument(void)
{
	GKDBusArgumentString::currentString.clear();

	const uint64_t size = ArgUInt64::getNextUInt64Argument();

	if( size != 0 ) {
		if( ArgBase::stringArguments.empty() )
			throw EmptyContainer("missing argument : string");

		GKDBusArgumentString::currentString = ArgBase::stringArguments.back();
		ArgBase::stringArguments.pop_back();

		if( GKDBusArgumentString::currentString.size() != size ) {
			LOG(warning) << "wrong string size";
		}
	}

	return GKDBusArgumentString::currentString;
}

const std::vector<std::string> & GKDBusArgumentString::getStringsArray(void)
{
	return ArgBase::stringArguments;
}

} // namespace NSGKDBus

