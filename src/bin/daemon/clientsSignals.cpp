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


#include "lib/utils/utils.hpp"
#include "lib/shared/glogik.hpp"

#include "clientsSignals.hpp"

#include "daemonControl.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

void ClientsSignals::sendSignalToClients(
	const uint8_t numClients,
	NSGKDBus::GKDBus* pDBus,
	const std::string & signal,
	const bool forceSend) noexcept
{
	GK_LOG_FUNC

	/* don't try to send signal if we know that there is no clients */
	if( (numClients == 0 ) and (! forceSend) )
		return;

	/*
	 * don't send signal if the daemon is about to exit,
	 * but DaemonIsStopping signal must always be sent
	 */
	if( ! DaemonControl::isDaemonRunning() )
		if(signal != "DaemonIsStopping")
			return;

	GKLog2(trace, "sending signal : ", signal)

	try {
		pDBus->initializeBroadcastSignal(
			_systemBus,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE,
			signal.c_str()
		);
		pDBus->sendBroadcastSignal();
	}
	catch (const GLogiKExcept & e) {
		pDBus->abandonBroadcastSignal();

		std::string warn("DBus targets signal failure : ");
		warn += e.what();
		/* don't warn nor syslog if we force sending the signal */
		if(! forceSend) {
			GKSysLogWarning(warn);
		}
		else {
			GKLog(trace, warn)
		}
	}
}

void ClientsSignals::sendStatusSignalArrayToClients(
	const uint8_t numClients,
	NSGKDBus::GKDBus* pDBus,
	const std::string & signal,
	const std::vector<std::string> & devIDArray) noexcept
{
	GK_LOG_FUNC

	/* don't try to send signal if we know that there is no clients */
	if( numClients == 0 )
		return;

	/* don't send signal if the daemon is about to exit */
	if( ! DaemonControl::isDaemonRunning() )
		return;

	GKLog2(trace, "sending signal : ", signal)

	try {
		pDBus->initializeBroadcastSignal(
			_systemBus,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
			GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
			signal.c_str()
		);
		pDBus->appendStringArrayToBroadcastSignal(devIDArray);
		pDBus->sendBroadcastSignal();
	}
	catch (const GLogiKExcept & e) {
		pDBus->abandonBroadcastSignal();
		std::string warn("failed to send signal : ");
		warn += e.what();
		GKSysLogWarning(warn);
	}
}

} // namespace GLogiK

