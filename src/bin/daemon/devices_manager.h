/*
 *
 *	This file is part of GLogiK project.
 *	GLogiKd, daemon to handle special features on gaming keyboards
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

#include "keyboard_driver.h"
#include "lib/dbus/GKDBus.h"

#include "globals.h"

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
void deviceProperties(struct udev_device *dev, const std::string &subsystem);
#endif

class DevicesManager
{
	public:
		DevicesManager(void);
		~DevicesManager(void);

		void startMonitoring(GKDBus* DBus);

	protected:

	private:
		struct udev *udev = nullptr;
		struct udev_monitor *monitor = nullptr;
		struct pollfd fds[1];
		int fd_ = -1;

		std::ostringstream buffer_;
		std::vector<KeyboardDriver*> drivers_;
		std::map<const std::string, DetectedDevice> detected_devices_;
		std::map<const std::string, DetectedDevice> initialized_devices_;
		std::map<const std::string, DetectedDevice> plugged_but_closed_devices_;

		void searchSupportedDevices(void);
		void initializeDevices(void);
		void closeInitializedDevices(void);
		const bool closeDevice(const std::string & devID);
		void cleanUnpluggedDevices(void);
		void checkDBusMessages(GKDBus* DBus);
};

} // namespace GLogiK

#endif
