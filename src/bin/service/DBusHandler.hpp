/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2025  Fabrice Delliaux <netbox253@gmail.com>
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

#include "include/base.hpp"
#include "include/LCDPP.hpp"
#include "include/DepsMap.hpp"

#include "devicesHandler.hpp"
#include "GKeysEventManager.hpp"
#include "DBus.hpp"

namespace GLogiK
{

class DBusHandler
	:	public DBusInst
{
	public:
		DBusHandler(
			pid_t pid,
			NSGKUtils::FileSystem* pGKfs,
			GKDepsMap_type* dependencies
		);
		~DBusHandler(void);

		static bool WantToExit;	/* true if we want to exit */
		static void handleSignal(int signum);

		const bool getExitStatus(void) const;
		void checkNotifyEvents(NSGKUtils::FileSystem* pGKfs);

	protected:

	private:
		static constexpr NSGKDBus::BusConnection const & _sessionBus = NSGKDBus::GKDBus::SessionBus;
		static constexpr NSGKDBus::BusConnection const & _systemBus = NSGKDBus::GKDBus::SystemBus;

		GKeysEventManager _GKeysEvent;
		DevicesHandler _devices;

		std::string _clientID;
		std::string _daemonVersion;
		std::string _CURRENT_SESSION_DBUS_OBJECT_PATH;	/* current session object path */
		std::string _sessionState;		/* session state */

		const GKDepsMap_type* const _pDepsMap;

		SessionFramework _sessionFramework;

		bool _registerStatus;		/* true == registered with daemon */

		/* -- -- -- */

		void setCurrentSessionObjectPath(pid_t pid);
		const std::string getCurrentSessionState(void);

		void updateSessionState(void);

		void registerWithDaemon(void);
		void unregisterWithDaemon(void);

		void getDaemonDependenciesMap(GKDepsMap_type* const dependencies);

		void cleanGKDBusEvents(void) noexcept;
		void prepareToStop(const bool notifications = true);

		void reportChangedState(void) noexcept;

		void initializeDevices(void);
		void initializeGKDBusSignals(void);
		void initializeGKDBusMethods(void);

		static void sendServiceStartRequest(void);
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
		const LCDPPArray_type & getDeviceLCDPluginsProperties(
			const std::string & devID,
			const std::string & reserved
		);
		const GKDepsMap_type & getExecutablesDependenciesMap(const std::string & reserved);
};

} // namespace GLogiK

#endif
