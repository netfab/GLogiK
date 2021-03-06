/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2020  Fabrice Delliaux <netbox253@gmail.com>
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
#include <vector>
#include <map>

#include <boost/filesystem.hpp>

#include "lib/utils/utils.hpp"
#include "lib/dbus/GKDBus.hpp"
#include "lib/shared/glogik.hpp"
#include "lib/shared/deviceProperties.hpp"

#include "include/enums.hpp"
#include "include/LCDPluginProperties.hpp"

#define LogRemoteCallFailure \
	FATALERROR << remoteMethod.c_str() << CONST_STRING_METHOD_CALL_FAILURE << e.what();
#define LogRemoteCallGetReplyFailure \
	LOG(ERROR) << remoteMethod.c_str() << CONST_STRING_METHOD_REPLY_FAILURE << e.what();

typedef std::map<const std::string, const std::string> devices_files_map_t;

namespace fs = boost::filesystem;

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

		void doDeviceFakeKeyEvent(
			const std::string & devID,
			const std::string & mediaKeyEvent
		);

		const devices_files_map_t getDevicesMap(void);
		const std::vector<std::string> getDevicesList(void);
		const LCDPluginsPropertiesArray_type & getDeviceLCDPluginsProperties(
			const std::string & devID
		);

		void reloadDeviceConfigurationFile(const std::string & devID);

	protected:

	private:
		fs::path _configurationRootDirectory;
		std::string _clientID;
		NSGKDBus::GKDBus* _pDBus;
		NSGKUtils::FileSystem* _pGKfs;
		const NSGKDBus::BusConnection _systemBus;

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
		void sendDeviceConfigurationSavedSignal(const std::string & devID);

		void unrefDevice(const std::string & devID);

		const bool checkDeviceCapability(const DeviceProperties & device, Caps toCheck);
};

} // namespace GLogiK

#endif
