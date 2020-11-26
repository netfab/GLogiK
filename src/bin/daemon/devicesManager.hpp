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

#ifndef SRC_BIN_DAEMON_DEVICES_MANAGER_HPP_
#define SRC_BIN_DAEMON_DEVICES_MANAGER_HPP_

#include <cstdint>

#include <string>
#include <map>
#include <vector>

#include <config.h>

#if GKDBUS
#include "lib/dbus/GKDBus.hpp"

#include "clientsSignals.hpp"
#endif

#include "keyboardDriver.hpp"

#include "USBDeviceID.hpp"

#include "include/keyEvent.hpp"
#include "include/LCDPluginProperties.hpp"

namespace GLogiK
{

#if DEBUGGING_ON
void udevDeviceProperties(struct udev_device * pDevice, const std::string & subSystem);
#endif

class DevicesManager
#if GKDBUS
	:	public ClientsSignals
#endif
{
	public:
		DevicesManager(void);
		~DevicesManager(void);

		void startMonitoring(void);

#if GKDBUS
		void setDBus(NSGKDBus::GKDBus* pDBus);
		void setNumClients(uint8_t num);

		void checkDBusMessages(void) noexcept;
#endif

		void resetDevicesStates(void);
		const bool startDevice(const std::string & devID);
		const bool stopDevice(const std::string & devID) noexcept;
		const std::vector<std::string> getStartedDevices(void) const;
		const std::vector<std::string> getStoppedDevices(void) const;

		const std::string & getDeviceVendor(const std::string & devID) const;
		const std::string & getDeviceModel(const std::string & devID) const;
		const uint64_t getDeviceCapabilities(const std::string & devID) const;
		const LCDPluginsPropertiesArray_type & getDeviceLCDPluginsProperties(
			const std::string & devID
		) const;

		void setDeviceActiveConfiguration(
			const std::string & devID,
			const banksMap_type & macrosBanks,
			const uint8_t r,
			const uint8_t g,
			const uint8_t b,
			const uint64_t LCDPluginsMask1
		);

		const banksMap_type & getDeviceMacrosBanks(const std::string & devID) const;
		const std::vector<std::string> & getDeviceMacroKeysNames(const std::string & devID) const;
		const std::string getDeviceStatus(const std::string & devID) const;

	protected:

	private:
		const std::string _unknown;
		std::vector<KeyboardDriver*> _drivers;
		std::map<const std::string, USBDeviceID> _detectedDevices;
		std::map<const std::string, USBDeviceID> _startedDevices;
		std::map<const std::string, USBDeviceID> _stoppedDevices;
		std::map<const std::string, USBDeviceID> _unpluggedDevices;

#if GKDBUS
		NSGKDBus::GKDBus* _pDBus;
		uint8_t _numClients;
#endif

		void searchSupportedDevices(struct udev * pUdev);
		void initializeDevices(void) noexcept;
		void stopInitializedDevices(void);
		void checkInitializedDevicesThreadsStatus(void) noexcept;

		void checkForUnpluggedDevices(void) noexcept;
};

} // namespace GLogiK

#endif
