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

#ifndef SRC_BIN_SERVICE_DBUS_HANDLER_HPP_
#define SRC_BIN_SERVICE_DBUS_HANDLER_HPP_

#include <vector>
#include <string>
#include <exception>

#include <cstdint>

#include <sys/types.h>

#include "lib/utils/utils.hpp"
#include "lib/dbus/GKDBus.hpp"
#include "lib/shared/sessionManager.hpp"

#include "include/base.hpp"
#include "include/LCDPluginProperties.hpp"

#include "devicesHandler.hpp"
#include "GKeysEventManager.hpp"

#define UNREACHABLE_DAEMON_MAX_RETRIES 3

namespace GLogiK
{

enum class SessionFramework : uint8_t
{
	FW_UNKNOWN = 0,
	FW_LOGIND,
};

class restartRequest : public std::exception
{
	public :
		restartRequest( const std::string& msg = "" );

		virtual ~restartRequest( void ) throw();
		virtual const char* what( void ) const throw();

	protected :
		std::string message;
};

class DBusHandler
{
	public:
		DBusHandler(
			pid_t pid,
			SessionManager& session,
			NSGKUtils::FileSystem* pGKfs,
			NSGKDBus::GKDBus* pDBus
		);
		~DBusHandler(void);

		const bool getExitStatus(void) const;
		void checkNotifyEvents(NSGKUtils::FileSystem* pGKfs);
		void cleanDBusRequests(void);

	protected:

	private:
		GKeysEventManager _GKeysEvent;
		DevicesHandler _devices;

		std::string _clientID;
		std::string _daemonVersion;
		std::string _currentSession;	/* current session object path */
		std::string _sessionState;		/* session state */

		NSGKDBus::GKDBus* _pDBus;

		SessionFramework _sessionFramework;
		const NSGKDBus::BusConnection _sessionBus;
		const NSGKDBus::BusConnection _systemBus;

		bool _registerStatus;		/* true == registered with daemon */
		bool _wantToExit;			/* true if we want to exit after a restart request */

		/* -- -- -- */

		void setCurrentSessionObjectPath(pid_t pid);
		const std::string getCurrentSessionState(void);

		void updateSessionState(void);

		void registerWithDaemon(void);
		void unregisterWithDaemon(void);

		void clearAndUnregister(const bool notifications = true);

		void reportChangedState(void);

		void initializeDevices(void);
		void initializeGKDBusSignals(void);
		void initializeGKDBusMethods(void);

		void sendRestartRequest(void);
		void sendDevicesUpdatedSignal(void);

		/* signals from daemon */
		void daemonIsStopping(void);
		void daemonIsStarting(void);

		void devicesStarted(const std::vector<std::string> & devicesID);
		void devicesStopped(const std::vector<std::string> & devicesID);
		void devicesUnplugged(const std::vector<std::string> & devicesID);

		void deviceMediaEvent(const std::string & devID, const std::string & mediaKeyEvent);
		void deviceGKeyEvent(const std::string & devID, const GKeysID keyID);
		void deviceMBankSwitch(const std::string & devID, const MKeysID bankID);
		void deviceMacroRecorded(
			const std::string & devID,
			const GKeysID keyID,
			const macro_type & macro
		);
		void deviceMacroCleared(const std::string & devID, const GKeysID keyID);

		/* -- */

		/* signal and request from GUI  */
		void deviceStatusChangeRequest(
			const std::string & devID,
			const std::string & remoteMethod
		);

		const std::vector<std::string> getDevicesList(const std::string & reserved);
		const std::vector<std::string> getInformations(const std::string & reserved);
		const LCDPluginsPropertiesArray_type & getDeviceLCDPluginsProperties(
			const std::string & devID,
			const std::string & reserved
		);
};

} // namespace GLogiK

#endif
