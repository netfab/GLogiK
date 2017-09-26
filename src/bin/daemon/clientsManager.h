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

#ifndef __GLOGIKD_CLIENTS_MANAGER_H__
#define __GLOGIKD_CLIENTS_MANAGER_H__

#include <string>
#include <map>

#include "lib/dbus/GKDBus.h"

#include "devicesManager.h"

#include "include/glogik.h"

namespace GLogiK
{

class ClientsManager
{
	public:
		ClientsManager(GKDBus* pDBus);
		~ClientsManager(void);

		void runLoop(void);

	protected:

	private:
		const char* DBus_object_	= GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT;
		const char* DBus_interface_	= GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE;

		GKDBus* DBus;

		DevicesManager devicesManager;

		std::map<const std::string, bool> clients_;
		
		const bool registerClient(const std::string & uniqueString);
		const bool unregisterClient(const std::string & uniqueString);
};

} // namespace GLogiK

#endif
