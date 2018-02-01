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

#ifndef __GLOGIKD_CLIENT_H__
#define __GLOGIKD_CLIENT_H__

#include <cstdint>

#include <string>
#include <vector>
#include <map>

#include "devicesManager.h"
#include "lib/shared/keyEvent.h"
#include "lib/shared/deviceProperties.h"

namespace GLogiK
{

class Client
{
	public:
		Client(const std::string & object_path, DevicesManager* dev_manager);
		~Client(void);

		const std::string & getSessionObjectPath(void) const;
		const std::string & getSessionCurrentState(void) const;
		void updateSessionState(const std::string & new_state);
		void uncheck(void);
		const bool isAlive(void) const;
		const bool isReady(void) const;

		const std::vector<std::string> getDeviceProperties(const std::string & devID, DevicesManager* dev_manager);
		const bool deleteDevice(const std::string & devID);
		const bool setDeviceBacklightColor(
			const std::string & devID,
			const uint8_t r,
			const uint8_t g,
			const uint8_t b
		);

/*
		const bool setDeviceBacklightColor(
			const std::string & devID,
			DevicesManager* dev_manager
		);
*/

		void setDeviceActiveUser(
			const std::string & devID,
			DevicesManager* dev_manager
		);

		void syncDeviceMacrosProfiles(
			const std::string & devID,
			const macros_map_t & macros_profiles
		);

		const macro_t & getDeviceMacro(
			const std::string & devID,
			const std::string & keyName,
			const uint8_t profile
		);

		const bool setDeviceMacrosBank(
			const std::string & devID,
			const uint8_t profile,
			const macros_bank_t & bank
		);

		void toggleClientReadyPropertie(void);

	protected:

	private:
		std::string session_state_;
		std::string client_session_object_path_;
		std::map<const std::string, DeviceProperties> devices_;
		bool check_;
		bool ready_;
};

} // namespace GLogiK

#endif
