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

#include <vector>
#include <string>
#include <map>
#include <sstream>

#include "lib/dbus/GKDBus.h"

#include "devicesManager.h"
#include "client.h"

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

		/* CSMH - Clients System Message Handler */
		const char* DBus_CSMH_object_path_	= GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT_PATH;
		const char* DBus_CSMH_interface_	= GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE;

		std::ostringstream buffer_;
		GKDBus* DBus;
		DevicesManager* devicesManager;

		std::map<const std::string, Client*> clients_;

		const std::string generateRandomClientID(void);
		const bool registerClient(const std::string & clientSessionObjectPath);
		const bool unregisterClient(const std::string & clientID);
		const bool updateClientState(const std::string & clientID, const std::string & state);
};

} // namespace GLogiK

#endif
