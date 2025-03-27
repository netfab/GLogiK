/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2024  Fabrice Delliaux <netbox253@gmail.com>
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

#include "include/base.hpp"
#include "include/LCDPP.hpp"

#define LogRemoteCallFailure \
	LOG(critical) << remoteMethod.c_str() << CONST_STRING_METHOD_CALL_FAILURE << e.what();
#define LogRemoteCallGetReplyFailure \
	LOG(error) << remoteMethod.c_str() << CONST_STRING_METHOD_REPLY_FAILURE << e.what();

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

		static const std::string getLibudevVersion(void);

		void initializeDBusRequests(void);
		void cleanDBusRequests(void) noexcept;
		void startMonitoring(void);

#if GKDBUS
		void setDBus(NSGKDBus::GKDBus* pDBus);
		void setNumClients(uint8_t num);

		void checkDBusMessages(void) noexcept;
#endif

		void resetDevicesStates(void);
		const bool startDevice(const std::string & devID);
		const bool stopDevice(
			const std::string & devID,
			const bool skipUSBRequests = false
		) noexcept;
		const std::vector<std::string> getStartedDevices(void) const;
		const std::vector<std::string> getStoppedDevices(void) const;

		const std::string & getDeviceVendor(const std::string & devID) const;
		const std::string & getDeviceProduct(const std::string & devID) const;
		const std::string & getDeviceName(const std::string & devID) const;
		const uint64_t getDeviceCapabilities(const std::string & devID) const;
		const LCDPPArray_type & getDeviceLCDPluginsProperties(
			const std::string & devID
		) const;

		void setDeviceActiveConfiguration(
			const std::string & devID,
			const uint8_t r,
			const uint8_t g,
			const uint8_t b,
			const uint64_t LCDPluginsMask1
		);

		const MKeysIDArray_type getDeviceMKeysIDArray(const std::string & devID) const;
		const GKeysIDArray_type getDeviceGKeysIDArray(const std::string & devID) const;
		const std::string getDeviceStatus(const std::string & devID) const;

	protected:

	private:
		std::map<std::string, USBDeviceID> _detectedDevices;
		std::map<std::string, USBDeviceID> _startedDevices;
		std::map<std::string, USBDeviceID> _stoppedDevices;
		std::map<std::string, USBDeviceID> _unpluggedDevices;
		std::vector<std::string> _sleepingDevices;
		std::vector<KeyboardDriver*> _drivers;
		const std::string _unknown;

#if GKDBUS
		const NSGKDBus::BusConnection & _systemBus = NSGKDBus::GKDBus::SystemBus;

		NSGKDBus::GKDBus* _pDBus;
		SessionFramework _sessionFramework;
		uint8_t _numClients;
		int32_t _delayLockPID;

		void inhibitSleepState(void);
		void releaseDelayLock(void);
		void HandleSleepEvent(const bool mode);
#endif

		void searchSupportedDevices(struct udev * pUdev);
		void initializeDevices(const bool openDevices) noexcept;
		void startSleepingDevices(void);
		void stopInitializedDevices(void);
		void checkInitializedDevicesThreadsStatus(void) noexcept;

		void checkForUnpluggedDevices(void) noexcept;
};

} // namespace GLogiK

#endif
