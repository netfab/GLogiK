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

#ifndef SRC_BIN_SERVICE_DEVICES_HANDLER_HPP_
#define SRC_BIN_SERVICE_DEVICES_HANDLER_HPP_

#include <cstdint>

#include <string>
#include <map>

#include "lib/utils/utils.h"
#include "lib/dbus/GKDBus.h"
#include "lib/shared/glogik.h"
#include "lib/shared/deviceProperties.h"

#include "include/enums.h"

#if DESKTOP_NOTIFICATIONS
#include "volumeNotification.h"
#endif

typedef std::map<const std::string, const std::string> devices_files_map_t;

namespace GLogiK
{

class DevicesHandler
{
	public:
		DevicesHandler(void);
		~DevicesHandler(void);

		void setGKfs(NSGKUtils::FileSystem* pGKfs);
		void setDBus(NSGKDBus::GKDBus* pDBus);
		void setClientID(const std::string & id);

		void startDevice(const std::string & devID);
		void stopDevice(const std::string & devID);
		void unplugDevice(const std::string & devID);

		void clearDevices(void);

		const bool setDeviceMacro(
			const std::string & devID,
			const std::string & keyName,
			const uint8_t bankID
		);

		const bool clearDeviceMacro(
			const std::string & devID,
			const std::string & keyName,
			const uint8_t bankID
		);

		void runDeviceMediaEvent(
			const std::string & devID,
			const std::string & mediaKeyEvent
		);

		const devices_files_map_t getDevicesMap(void);
		void checkDeviceConfigurationFile(const std::string & devID);

	protected:

	private:
		NSGKDBus::GKDBus* _pDBus;
		NSGKUtils::FileSystem* _pGKfs;
#if DESKTOP_NOTIFICATIONS
		VolumeNotification _notification;
#endif
		const NSGKDBus::BusConnection _systemBus;
		std::string _clientID;
		std::string _configurationRootDirectory;

		std::map<const std::string, DeviceProperties> _startedDevices;
		std::map<const std::string, DeviceProperties> _stoppedDevices;

		void setDeviceProperties(
			const std::string & devID,
			DeviceProperties & device
		);
		void loadDeviceConfigurationFile(
			DeviceProperties & device
		);
		void saveDeviceConfigurationFile(
			const std::string & devID,
			const DeviceProperties & device
		);
		void watchDirectory(
			DeviceProperties & device,
			const bool check=true
		);

		void sendDeviceConfigurationToDaemon(
			const std::string & devID,
			const DeviceProperties & device
		);
		void unrefDevice(const std::string & devID);

		const bool checkDeviceCapability(const DeviceProperties & device, Caps toCheck);

		void runCommand(
			const std::string & mediaKeyEvent,
			const std::string & command
		);

};

} // namespace GLogiK

#endif
