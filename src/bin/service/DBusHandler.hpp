/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2026  Fabrice Delliaux <netbox253@gmail.com>
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

#pragma once

#include <vector>
#include <string>

#include <cstdint>

#include <sys/types.h>

#include "lib/utils/utils.hpp"
#include "lib/dbus/GKDBus.hpp"

#include "include/base.hpp"
#include "include/LCDPP.hpp"
#include "include/DepsMap.hpp"
#include "include/DeviceID.hpp"

#include "devicesHandler.hpp"
#include "GKeysEventManager.hpp"

namespace GLogiK
{

class DBusHandler
{
	public:
		DBusHandler(
			pid_t pid,
			NSGKUtils::FileSystem* pGKfs,
			NSGKDBus::GKDBus* pDBus,
			GKDepsMap_type* dependencies
		);
		~DBusHandler();

		static bool WantToExit;	/* true if we want to exit */
		static bool WantToRestart;	/* true if we want to restart */
		static void handleSignal(int signum);

		void checkNotifyEvents(NSGKUtils::FileSystem* pGKfs);

		const DevicesMap_type getDevicesMap();

		/* return true when at least one device was updated (started/stopped/unplugged) */
		const bool isAnyDeviceUpdated() const;

		void resetDevicesUpdatedEvent();

		/* exit and restart the service on next main loop iteration */
		void restartService();

		void startDevice(const std::string & devID);
		void stopDevice(const std::string & devID);
		void restartDevice(const std::string & devID);

		/* return true if we want to exit on next main loop iteration */
		const bool wantToStop() const;

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
		NSGKDBus::GKDBus* const _pDBus;

		using SessionFramework = LIBShared::SessionFramework;
		SessionFramework _sessionFramework;

		bool _devicesUpdatedEvent;	/* true when at least one device
									   was updated (started/stopped/unplugged) */
		bool _registerStatus;	/* true == registered with daemon */

		/* -- -- -- */
		void updateSystrayContextMenu();

		void setCurrentSessionObjectPath(pid_t pid);
		const std::string getCurrentSessionState();

		void updateSessionState();

		void registerWithDaemon();
		void unregisterWithDaemon();

		void getDaemonDependenciesMap(GKDepsMap_type* const dependencies);

		void cleanGKDBusEvents() noexcept;
		void prepareToStop(const bool notifications = true);

		void reportChangedState() noexcept;

		void initializeDevices();
		void initializeGKDBusSignals();
		void initializeGKDBusMethods();

		void sendServiceStartRequest() const;
		void sendDevicesUpdatedSignal();

		/* signals from daemon */
		void daemonIsStopping();
		void daemonIsStarting();

		void devicesStarted(const std::vector<std::string> & devicesID);
		void devicesStopped(const std::vector<std::string> & devicesID);
		void devicesUnplugged(const std::vector<std::string> & devicesID);

		void deviceMediaEvent(
			const std::string & devID,
			const std::string & mediaKeyEvent
		);
		void deviceGKeyEvent(
			const std::string & devID,
			const GKeysID keyID
		);
		void deviceMBankSwitch(
			const std::string & devID,
			const MKeysID bankID
		);
		void deviceMacroRecorded(
			const std::string & devID,
			const GKeysID keyID,
			const macro_type & macro
		);
		void deviceMacroCleared(
			const std::string & devID,
			const GKeysID keyID
		);

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

inline const bool DBusHandler::isAnyDeviceUpdated() const
{
	return _devicesUpdatedEvent;
}

inline void DBusHandler::updateSystrayContextMenu()
{
	/* used by service main loop */
	_devicesUpdatedEvent = true;
}

} // namespace GLogiK
