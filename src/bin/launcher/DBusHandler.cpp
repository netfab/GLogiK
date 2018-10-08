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

#include <iostream>
#include <sstream>
#include <new>
#include <functional>

#include "lib/shared/glogik.hpp"

#include "DBusHandler.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

LauncherDBusHandler::LauncherDBusHandler(void)
	:	_pDBus(nullptr),
		_sessionBus(NSGKDBus::BusConnection::GKDBUS_SESSION)
{
	try {
		try {
			_pDBus = new NSGKDBus::GKDBus(GLOGIK_DESKTOP_SERVICE_LAUNCHER_DBUS_ROOT_NODE, GLOGIK_DESKTOP_SERVICE_LAUNCHER_DBUS_ROOT_NODE_PATH);
		}
		catch (const std::bad_alloc& e) { /* handle new() failure */
			throw GLogiKBadAlloc("GKDBus bad allocation");
		}

		_pDBus->connectToSessionBus(GLOGIK_DESKTOP_SERVICE_LAUNCHER_DBUS_BUS_CONNECTION_NAME);

		this->initializeGKDBusSignals();
	}
	catch ( const GLogiKExcept & e ) {
		delete _pDBus; _pDBus = nullptr;
		throw;
	}
}

LauncherDBusHandler::~LauncherDBusHandler(void)
{
	delete _pDBus; _pDBus = nullptr;
}

void LauncherDBusHandler::checkDBusMessages(void) {
	_pDBus->checkForNextMessage(_sessionBus);
}

/*
 * --- --- --- --- ---
 * --- --- --- --- ---
 *
 * signals - signals
 *
 * --- --- --- --- ---
 * --- --- --- --- ---
 *
 */

void LauncherDBusHandler::initializeGKDBusSignals(void) {
	_pDBus->NSGKDBus::EventGKDBusCallback<VoidToVoid>::exposeSignal(
		_sessionBus,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		"RestartRequest",
		{},
		std::bind(&LauncherDBusHandler::restartRequest, this)
	);
}

void LauncherDBusHandler::restartRequest(void)
{
	std::ostringstream buffer("received signal: ", std::ios_base::app);
	buffer << __func__;
#if DEBUGGING_ON
	LOG(DEBUG2) << buffer.str();
#endif
}

} // namespace GLogiK

