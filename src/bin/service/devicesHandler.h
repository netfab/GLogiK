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

#ifndef __GLOGIK_DESKTOP_SERVICE_DEVICES_HANDLER_H__
#define __GLOGIK_DESKTOP_SERVICE_DEVICES_HANDLER_H__

#include <string>
#include <vector>
#include <map>
#include <sstream>

#include "lib/dbus/GKDBus.h"
#include "lib/shared/deviceProperties.h"

#include "include/glogik.h"

namespace GLogiK
{

class DevicesHandler
{
	public:
		DevicesHandler(void);
		~DevicesHandler(void);

		void setDBus(GKDBus* pDBus);

		void saveDevicesProperties(void);

		void checkStartedDevice(const std::string & device);
		void checkStoppedDevice(const std::string & device);
		void clearLoadedDevices(void);

	protected:

	private:
		/* DDM - Daemon DevicesManager - to contact the DevicesManager */
		const char* DBus_DDM_object_path_	= GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH;
		const char* DBus_DDM_interface_		= GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE;

		GKDBus* DBus;
		std::ostringstream buffer_;
		std::string config_root_directory_;

		std::vector<std::string> used_conf_files_;
		std::map<const std::string, DeviceProperties> devices_;

		void setDeviceProperties(const std::string & devID, DeviceProperties & device);
};

} // namespace GLogiK

#endif
