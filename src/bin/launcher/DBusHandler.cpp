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

#include <new>
#include <functional>
#include <thread>

#include <config.h>

#include "lib/utils/utils.hpp"
#include "lib/shared/glogik.hpp"

#include "DBusHandler.hpp"

namespace chr = std::chrono;

namespace GLogiK
{

using namespace NSGKUtils;

DBusHandler::DBusHandler(NSGKDBus::GKDBus* pDBus)
	:	_pDBus(pDBus)
{
	this->initializeGKDBusSignals();

	/* spawn desktop service on start */
	this->spawnService(100);
}

DBusHandler::~DBusHandler(void)
{
}

void DBusHandler::cleanDBusRequests(void)
{
	_pDBus->removeInterface(_sessionBus,
		GLOGIK_DESKTOP_QT_SESSION_DBUS_OBJECT_PATH,
		GLOGIK_DESKTOP_QT_SESSION_DBUS_INTERFACE);

	_pDBus->removeInterface(_sessionBus,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE);
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

void DBusHandler::initializeGKDBusSignals(void)
{
	_pDBus->NSGKDBus::Callback<SIGq2v>::receiveSignal(
		_sessionBus,
		GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		GK_DBUS_LAUNCHER_SIGNAL_SERVICE_START_REQUEST,
		{ {"q", "sleep_ms", "in", "sleeping time in milliseconds before spawning service"} },
		std::bind(&DBusHandler::spawnService, this, std::placeholders::_1)
	);

	_pDBus->NSGKDBus::Callback<SIGq2v>::receiveSignal(
		_sessionBus,
		GLOGIK_DESKTOP_QT_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DESKTOP_QT_SESSION_DBUS_OBJECT_PATH,
		GLOGIK_DESKTOP_QT_SESSION_DBUS_INTERFACE,
		GK_DBUS_LAUNCHER_SIGNAL_SERVICE_START_REQUEST,
		{ {"q", "sleep_ms", "in", "sleeping time in milliseconds before spawning service"} },
		std::bind(&DBusHandler::spawnService, this, std::placeholders::_1)
	);
}

void DBusHandler::spawnService(const uint16_t delay)
{
	GK_LOG_FUNC

	LOG(info)	<< "received signal: " << __func__;

	std::vector<std::string> args;

#if DEBUGGING_ON
	if(GKLogging::GKDebug)
		args.push_back("-D");
#endif

	process::runDelayedCommand(GLOGIK_DESKTOP_SERVICE_NAME, args, delay);
}

} // namespace GLogiK

