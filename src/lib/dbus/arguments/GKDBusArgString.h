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

#ifndef __GLOGIK_GKDBUS_ARG_STRING_H__
#define __GLOGIK_GKDBUS_ARG_STRING_H__

#include "GKDBusArgument.h"

#include <string>
#include <vector>

namespace GLogiK
{

class GKDBusArgumentString : public GKDBusArgument
{
	public:
		static const std::string & getNextStringArgument(void);
		static const std::vector<std::string> & getStringsArray(void);

	protected:
		GKDBusArgumentString(void) = default;
		~GKDBusArgumentString(void) = default;

	private:
		thread_local static std::string current_string_;

};

} // namespace GLogiK

#endif
