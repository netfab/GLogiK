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

#ifndef __GLOGIKD_CLIENTS_MANAGER_H__
#define __GLOGIKD_CLIENTS_MANAGER_H__

#include <vector>
#include <string>
#include <map>
#include <sstream>

#include "lib/shared/keyEvent.h"
#include "lib/dbus/GKDBus.h"

#include "devicesManager.h"
#include "clientsSignals.h"
#include "client.h"

#define GKSysLog_UnknownClient \
	std::string error(s_UnknownClient); error += clientID;\
	GKSysLog(LOG_ERR, ERROR, error);

namespace GLogiK
{

class ClientsManager
	:	public ClientsSignals
{
	public:
		ClientsManager(NSGKDBus::GKDBus* pDBus);
		~ClientsManager(void);

		void runLoop(void);

	protected:

	private:
		std::ostringstream buffer_;
		NSGKDBus::GKDBus* pDBus_;
		DevicesManager* devicesManager;

		unsigned int active_clients_;
		std::map<const std::string, Client*> clients_;
		bool enabled_signals_;

		const std::string generateRandomClientID(void) const;

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
		const std::vector<std::string> getDeviceProperties(
			const std::string & clientID,
			const std::string & devID
		);
		const std::vector<std::string> & getDeviceMacroKeysNames(
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
		const macro_t & getDeviceMacro(
			const std::string & clientID,
			const std::string & devID,
			const std::string & keyName,
			const uint8_t profile
		);

		const bool setDeviceMacrosBank(
			const std::string & clientID,
			const std::string & devID,
			const uint8_t profile,
			const macros_bank_t & bank
		);
		/* -- */
};

} // namespace GLogiK

#endif
