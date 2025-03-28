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

#ifndef SRC_BIN_DAEMON_SLEEP_INHIBITION_HPP_
#define SRC_BIN_DAEMON_SLEEP_INHIBITION_HPP_

#include "lib/dbus/GKDBus.hpp"
#include "lib/shared/glogik.hpp"

#include "devicesManager.hpp"

#define LogRemoteCallFailure \
	LOG(critical) << remoteMethod.c_str() << CONST_STRING_METHOD_CALL_FAILURE << e.what();
#define LogRemoteCallGetReplyFailure \
	LOG(error) << remoteMethod.c_str() << CONST_STRING_METHOD_REPLY_FAILURE << e.what();

namespace GLogiK
{

class SleepInhibition
{
	public:

	protected:
		SleepInhibition(void);
		~SleepInhibition(void);

		void startSleepInhibition(
			NSGKDBus::GKDBus* const pDBus,
			DevicesManager* const pDevicesManager
		);
		void stopSleepInhibition(void) noexcept;

	private:
		const NSGKDBus::BusConnection & _systemBus = NSGKDBus::GKDBus::SystemBus;
		NSGKDBus::GKDBus* _pDBus;
		DevicesManager* _pDevicesManager;

		SessionFramework _sessionFramework;
		int32_t _delayLockPID;

		void inhibitSleepState(void);
		void handleSleepEvent(const bool mode);
		void releaseDelayLock(void);
};

} // namespace GLogiK

#endif
