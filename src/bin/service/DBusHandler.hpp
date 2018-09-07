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

#ifndef SRC_BIN_SERVICE_DBUS_HANDLER_HPP_
#define SRC_BIN_SERVICE_DBUS_HANDLER_HPP_

#include <vector>
#include <string>

#include <cstdint>

#include <sys/types.h>

#include "lib/utils/utils.hpp"
#include "lib/dbus/GKDBus.hpp"
#include "lib/shared/sessionManager.hpp"

#include "devicesHandler.hpp"

#define UNREACHABLE_DAEMON_MAX_RETRIES 3

namespace GLogiK
{

enum class SessionFramework : uint8_t
{
	FW_UNKNOWN = 0,
	FW_CONSOLEKIT,
	FW_LOGIND,
};

class DBusHandler
{
	public:
		DBusHandler(
			pid_t pid,
			SessionManager& session,
			NSGKUtils::FileSystem* pGKfs
		);
		~DBusHandler(void);

		void updateSessionState(void);
		void checkDBusMessages(void);

		void checkNotifyEvents(NSGKUtils::FileSystem* pGKfs);

	protected:

	private:
		NSGKDBus::GKDBus* _pDBus;
		const NSGKDBus::BusConnection _systemBus;
		DevicesHandler _devices;
		bool _skipRetry;			/* true == don't retry if register fails */
		bool _registerStatus;		/* true == registered with daemon */
		std::string _clientID;
		SessionFramework _sessionFramework;

		std::string _currentSession;	/* current session object path */
		std::string _sessionState;		/* session state */

		void setCurrentSessionObjectPath(pid_t pid);
		const std::string getCurrentSessionState(const bool disabledDebugOutput=false);

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
			const uint8_t bankID
		);
		const bool macroCleared(
			const std::string & devID,
			const std::string & keyName,
			const uint8_t bankID
		);

		void devicesStarted(const std::vector<std::string> & devicesID);
		void devicesStopped(const std::vector<std::string> & devicesID);
		void devicesUnplugged(const std::vector<std::string> & devicesID);

		void deviceMediaEvent(const std::string & devID, const std::string & mediaKeyEvent);
		/* -- */
};

} // namespace GLogiK

#endif
