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


#include "lib/utils/utils.h"
#include "lib/shared/glogik.h"

#include "daemonControl.h"

#include "clientsSignals.h"

namespace GLogiK
{

using namespace NSGKUtils;

void ClientsSignals::sendSignalToClients(
	const uint8_t num_clients,
	NSGKDBus::GKDBus* pDBus,
	const std::string & signal)
{
	/* don't try to send signal if we know that there is no clients */
	if( num_clients == 0 )
		return;

	/*
	 * don't send signal if the daemon is about to exit,
	 * but DaemonIsStopping signal must always be sent
	 */
	if( ! DaemonControl::isDaemonRunning() )
		if(signal != "DaemonIsStopping")
			return;

#if DEBUGGING_ON
	LOG(DEBUG2) << "sending clients " << signal << " signal";
#endif
	try {
		pDBus->initializeTargetsSignal(
			NSGKDBus::BusConnection::GKDBUS_SYSTEM,
			GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT_PATH,
			GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
			signal.c_str()
		);
		pDBus->sendTargetsSignal();
	}
	catch (const GLogiKExcept & e) {
		std::string warn("DBus targets signal failure : ");
		warn += e.what();
		GKSysLog(LOG_WARNING, WARNING, warn);
	}
}

void ClientsSignals::sendStatusSignalArrayToClients(
	const uint8_t num_clients,
	NSGKDBus::GKDBus* pDBus,
	const std::string & signal,
	const std::vector<std::string> & devIDArray
) {
	/* don't try to send signal if we know that there is no clients */
	if( num_clients == 0 )
		return;

	/* don't send signal if the daemon is about to exit */
	if( ! DaemonControl::isDaemonRunning() )
		return;

#if DEBUGGING_ON
	LOG(DEBUG2) << "sending clients " << signal << " signal";
#endif
	try {
		pDBus->initializeTargetsSignal(
			NSGKDBus::BusConnection::GKDBUS_SYSTEM,
			GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT_PATH,
			GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
			signal.c_str()
		);
		pDBus->appendStringVectorToTargetsSignal(devIDArray);
		pDBus->sendTargetsSignal();
	}
	catch (const GLogiKExcept & e) {
		std::string warn("DBus targets signal failure : ");
		warn += e.what();
		GKSysLog(LOG_WARNING, WARNING, warn);
	}
}

} // namespace GLogiK
