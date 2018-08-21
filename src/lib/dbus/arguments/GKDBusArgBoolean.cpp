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

#include "lib/utils/utils.h"

#include "GKDBusArgBoolean.h"

namespace NSGKDBus
{

using namespace NSGKUtils;

const bool GKDBusArgumentBoolean::getNextBooleanArgument(void) {
	if( GKDBusArgument::booleanArguments.empty() )
		throw EmptyContainer("missing argument : boolean");
	const bool ret = GKDBusArgument::booleanArguments.back();
	GKDBusArgument::booleanArguments.pop_back();
	return ret;
}

} // namespace NSGKDBus

