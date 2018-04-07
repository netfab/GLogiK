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

#ifndef __GLOGIKS_DESKTOP_SERVICE_DBUS_HANDLER_H__
#define __GLOGIKS_DESKTOP_SERVICE_DBUS_HANDLER_H__

#include <sstream>
#include <vector>
#include <string>

#include <sys/types.h>

#include "lib/utils/utils.h"
#include "lib/dbus/GKDBus.h"
#include "lib/shared/sessionManager.h"

#include "devicesHandler.h"

#define UNREACHABLE_DAEMON_MAX_RETRIES 12

namespace GLogiK
{

enum class SessionTracker
{
	F_UNKNOWN = 0,
	F_CONSOLEKIT,
	F_LOGIND,
};

class ServiceDBusHandler
{
	public:
		ServiceDBusHandler(
			pid_t pid,
			SessionManager& session,
			NSGKUtils::FileSystem* pGKfs
		);
		~ServiceDBusHandler(void);

		void updateSessionState(void);
		void checkDBusMessages(void);

		const devices_files_map_t getDevicesMap(void);
		void checkDeviceConfigurationFile(const std::string & devID);

	protected:

	private:
		NSGKDBus::GKDBus* pDBus_;
		const NSGKDBus::BusConnection system_bus_;
		DevicesHandler devices_;
		bool skip_retry_;
		bool are_we_registered_;
		std::string client_id_;
		SessionTracker session_framework_;

		std::ostringstream buffer_;

		std::string current_session_;	/* current session object path */
		std::string session_state_;		/* session state */

		void setCurrentSessionObjectPath(pid_t pid);
		const std::string getCurrentSessionState(const bool logoff=false);

		void registerWithDaemon(void);
		void unregisterWithDaemon(void);

		void reportChangedState(void);

		void initializeDevices(void);
		void initializeGKDBusSignals(void);

		/* signals */
		void daemonIsStopping(void);
		void daemonIsStarting(void);

		const bool macroRecorded(
			const std::string & devID,
			const std::string & keyName,
			const uint8_t profile
		);
		const bool macroCleared(
			const std::string & devID,
			const std::string & keyName,
			const uint8_t profile
		);

		void devicesStarted(const std::vector<std::string> & devicesID);
		void devicesStopped(const std::vector<std::string> & devicesID);
		void devicesUnplugged(const std::vector<std::string> & devicesID);

		void multimediaEvent(const std::string & devID, const std::string & media_event);
		/* -- */
};

} // namespace GLogiK

#endif
