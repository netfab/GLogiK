/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_BIN_DAEMON_CLIENTS_MANAGER_HPP_
#define SRC_BIN_DAEMON_CLIENTS_MANAGER_HPP_

#include <cstdint>

#include <vector>
#include <string>
#include <map>

#include "lib/dbus/GKDBus.hpp"

#include "devicesManager.hpp"

#include "clientsSignals.hpp"
#include "client.hpp"

#include "include/base.hpp"
#include "include/LCDPP.hpp"
#include "include/DepsMap.hpp"

namespace GLogiK
{

class ClientsManager
	:	public ClientsSignals
{
	public:
		ClientsManager(
			NSGKDBus::GKDBus* const pDBus,
			DevicesManager* const pDevicesManager,
			GKDepsMap_type* const pDepsMap
		);
		~ClientsManager(void);

		void cleanDBusRequests(void) noexcept;
		void waitForClientsDisconnections(void) noexcept;

	protected:

	private:
		NSGKDBus::GKDBus* const _pDBus;
		DevicesManager* const _pDevicesManager;
		GKDepsMap_type* const _pDepsMap;

		const std::string _active;
		std::map<std::string, Client*> _connectedClients;

		/* internal counter of active clients used
		 * in ::updateClientState()
		 */
		unsigned int _numActive;

		/* boolean flag used internally to temporarly disable
		 * signals sending when ::restartDevice() is called
		 */
		bool _enabledSignals; 

		const std::string generateRandomClientID(void) const;

		void initializeDBusRequests(void);

		/* exposed over DBus */
			/* ClientsManager D-Bus object */
		const bool registerClient(
			const std::string & clientSessionObjectPath
		);
		const bool unregisterClient(
			const std::string & clientID
		);
		const bool updateClientState(
			const std::string & clientID,
			const std::string & state
		);

		const bool toggleClientReadyPropertie(
			const std::string & clientID
		);
		const bool deleteDeviceConfiguration(
			const std::string & clientID,
			const std::string & devID
		);

		/* -- */
			/* DevicesManager D-Bus object */

		const bool stopDevice(
			const std::string & clientID,
			const std::string & devID
		);
		const bool startDevice(
			const std::string & clientID,
			const std::string & devID
		);
		const bool restartDevice(
			const std::string & clientID,
			const std::string & devID
		);

		const std::vector<std::string> getStartedDevices(
			const std::string & clientID
		);
		const std::vector<std::string> getStoppedDevices(
			const std::string & clientID
		);

		const std::string getDeviceStatus(
			const std::string & clientID,
			const std::string & devID
		);
		void getDeviceProperties(
			const std::string & clientID,
			const std::string & devID
		);
		const LCDPPArray_type & getDeviceLCDPluginsProperties(
			const std::string & clientID,
			const std::string & devID
		);
		const MKeysIDArray_type getDeviceMKeysIDArray(
			const std::string & clientID,
			const std::string & devID
		);
		const GKeysIDArray_type getDeviceGKeysIDArray(
			const std::string & clientID,
			const std::string & devID
		);

		const bool setDeviceBacklightColor(
			const std::string & clientID,
			const std::string & devID,
			const uint8_t r,
			const uint8_t g,
			const uint8_t b
		);
		const bool setDeviceLCDPluginsMask(
			const std::string & clientID,
			const std::string & devID,
			const uint8_t LCDPluginsMask,
			const uint64_t mask
		);

		/* -- */
};

} // namespace GLogiK

#endif
