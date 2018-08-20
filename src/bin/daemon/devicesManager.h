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

#ifndef __GLOGIKD_DEVICES_MANAGER_H__
#define __GLOGIKD_DEVICES_MANAGER_H__

#include <poll.h>

#include <cstdint>

#include <string>
#include <map>
#include <vector>
#include <set>
#include <sstream>

#include <config.h>

#include "include/keyEvent.h"
#include "lib/dbus/GKDBus.h"

#include "DeviceID.h"
#include "clientsSignals.h"

#include "keyboardDriver.h"

namespace GLogiK
{

class DetectedDevice
	:	public BusNumDeviceID
{
	public:
		DetectedDevice(void) = default;
		DetectedDevice(
			const std::string & n,
			const std::string & v,
			const std::string & p,
			const uint64_t c,
			const uint8_t b,
			const uint8_t nu,
			const std::string & node,
			const std::string & ven,
			const std::string & mod,
			const std::string & ser,
			const std::string & us,
			const uint16_t d_ID
		)	:	BusNumDeviceID(n,v,p,c,b,nu),
				input_dev_node(node),
				vendor(ven),
				model(mod),
				serial(ser),
				usec(us),
				driver_ID(d_ID) {};

		const std::string & getInputDevNode(void) const { return this->input_dev_node; }
		const std::string & getVendor(void) const { return this->vendor; }
		const std::string & getModel(void) const { return this->model; }
		const std::string & getUSec(void) const { return this->usec; }
		const uint16_t getDriverID(void) const { return this->driver_ID; }

	private:
		std::string input_dev_node;
		std::string vendor;
		std::string model;
		std::string serial;
		std::string usec;
		uint16_t driver_ID;
};

#if DEBUGGING_ON
void udevDeviceProperties(struct udev_device *dev, const std::string &subsystem);
#endif

class DevicesManager
	:	public ClientsSignals
{
	public:
		DevicesManager(void);
		~DevicesManager(void);

		void startMonitoring(NSGKDBus::GKDBus* pDBus);
		void checkDBusMessages(void);
		void resetDevicesStates(void);
		void setNumClients(uint8_t num);

		const bool startDevice(const std::string & devID);
		const bool stopDevice(const std::string & devID);
		const std::vector<std::string> getStartedDevices(void) const;
		const std::vector<std::string> getStoppedDevices(void) const;

		const std::string & getDeviceVendor(const std::string & devID) const;
		const std::string & getDeviceModel(const std::string & devID) const;
		const uint64_t getDeviceCapabilities(const std::string & devID) const;

		void setDeviceActiveConfiguration(
			const std::string & devID,
			const macros_map_type & macros_profiles,
			const uint8_t r,
			const uint8_t g,
			const uint8_t b
		);

		const macros_map_type & getDeviceMacrosBanks(const std::string & devID) const;
		const std::vector<std::string> & getDeviceMacroKeysNames(const std::string & devID) const;
		const std::string getDeviceStatus(const std::string & devID) const;

	protected:

	private:
		struct udev *udev = nullptr;
		struct udev_monitor *monitor = nullptr;
		struct pollfd fds[1];
		int fd_ = -1;
		NSGKDBus::GKDBus* pDBus_ = nullptr;
		uint8_t num_clients_;

		std::ostringstream buffer_;
		const std::string unknown_;
		std::vector<KeyboardDriver*> drivers_;
		std::map<const std::string, DetectedDevice> detected_devices_;
		std::map<const std::string, DetectedDevice> initialized_devices_;
		std::map<const std::string, DetectedDevice> plugged_but_stopped_devices_;
		std::set<std::string> unplugged_devices_;

		void searchSupportedDevices(void);
		void initializeDevices(void);
		void stopInitializedDevices(void);
		void checkInitializedDevicesThreadsStatus(void);

		void checkForUnpluggedDevices(void);
};

} // namespace GLogiK

#endif
