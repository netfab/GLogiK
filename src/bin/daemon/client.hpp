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

#ifndef SRC_BIN_DAEMON_CLIENT_HPP_
#define SRC_BIN_DAEMON_CLIENT_HPP_

#include <cstdint>

#include <string>
#include <vector>
#include <map>

#include "devicesManager.hpp"
#include "include/keyEvent.hpp"
#include "lib/shared/deviceProperties.hpp"

namespace GLogiK
{

class Client
{
	public:
		Client(const std::string & objectPath, DevicesManager* pDevicesManager);
		~Client(void);

		const std::string & getSessionObjectPath(void) const;
		const std::string & getSessionCurrentState(void) const;
		void updateSessionState(const std::string & newState);
		void uncheck(void);
		const bool isAlive(void) const;
		const bool isReady(void) const;

		void initializeDevice(
			DevicesManager* pDevicesManager,
			const std::string & devID
		);
		const bool deleteDevice(const std::string & devID);
		const bool setDeviceBacklightColor(
			const std::string & devID,
			const uint8_t r,
			const uint8_t g,
			const uint8_t b
		);

		void setDeviceActiveUser(
			const std::string & devID,
			DevicesManager* pDevicesManager
		);

		void syncDeviceMacrosBanks(
			const std::string & devID,
			const banksMap_type & macrosBanks
		);

		const macro_type & getDeviceMacro(
			const std::string & devID,
			const std::string & keyName,
			const uint8_t bankID
		);

		const bool setDeviceMacrosBank(
			const std::string & devID,
			const uint8_t bankID,
			const mBank_type & bank
		);

		const bool resetDeviceMacrosBank(
			const std::string & devID,
			const uint8_t bankID
		);

		void toggleClientReadyPropertie(void);

	protected:

	private:
		std::string _sessionState;
		const std::string _sessionObjectPath;
		std::map<const std::string, DeviceProperties> _devices;
		bool _check;
		bool _ready;

		void initializeDevices(DevicesManager* pDevicesManager);
};

} // namespace GLogiK

#endif
