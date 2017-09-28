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

#ifndef __GLOGIKD_CLIENT_H__
#define __GLOGIKD_CLIENT_H__

#include <string>
#include <map>

#include "lib/shared/deviceProperties.h"

namespace GLogiK
{

class Client
{
	public:
		Client(void);
		~Client(void);

		void updateSessionState(const std::string & new_state);

	protected:

	private:

		std::string session_state_;
		std::map<const std::string, DeviceProperties> devices_;
};

} // namespace GLogiK

#endif
