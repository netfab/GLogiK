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

#ifndef __GLOGIKD_DEVICES_MANAGER_H__
#define __GLOGIKD_DEVICES_MANAGER_H__

#include <poll.h>

#include <cstdint>

#include <string>
#include <map>
#include <vector>
#include <sstream>

#include <config.h>

#include "lib/dbus/GKDBus.h"

#include "keyboardDriver.h"

#include "include/glogik.h"

namespace GLogiK
{

struct DetectedDevice {
	KeyboardDevice device;
	std::string input_dev_node;
	std::string vendor;
	std::string model;
	std::string serial;
	std::string usec;
	uint16_t driver_ID;
	uint8_t device_bus;
	uint8_t device_num;
};

#if DEBUGGING_ON
void udevDeviceProperties(struct udev_device *dev, const std::string &subsystem);
#endif

class DevicesManager
{
	public:
		DevicesManager(void);
		~DevicesManager(void);

		void startMonitoring(GKDBus* pDBus);
		void checkDBusMessages(void);
		void resetDevicesStates(void);

	protected:

	private:
		struct udev *udev = nullptr;
		struct udev_monitor *monitor = nullptr;
		struct pollfd fds[1];
		int fd_ = -1;
		GKDBus* DBus = nullptr;

		const char* DBus_object_	= GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT;
		const char* DBus_interface_	= GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE;

		/* CSMH - Clients System Message Handler */
		const char* DBus_CSMH_object_path_	= GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT_PATH;
		const char* DBus_CSMH_interface_	= GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE;

		std::ostringstream buffer_;
		std::vector<KeyboardDriver*> drivers_;
		std::map<const std::string, DetectedDevice> detected_devices_;
		std::map<const std::string, DetectedDevice> initialized_devices_;
		std::map<const std::string, DetectedDevice> plugged_but_stopped_devices_;

		void searchSupportedDevices(void);
		void initializeDevices(void);
		void stopInitializedDevices(void);

		/* exposed over DBus */
		const bool startDevice(const std::string & devID);
		const bool stopDevice(const std::string & devID);
		const bool restartDevice(const std::string & devID);
		const std::vector<std::string> getStartedDevices(void);
		const std::vector<std::string> getStoppedDevices(void);
		const std::vector<std::string> getDeviceProperties(const std::string & devID);
		/* -- */

		void checkForUnpluggedDevices(void);

		void sendSignalToClients(const std::string & signal);
};

} // namespace GLogiK

#endif
