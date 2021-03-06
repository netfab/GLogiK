/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2020  Fabrice Delliaux <netbox253@gmail.com>
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

#include <boost/process.hpp>

#include "lib/shared/glogik.hpp"

#include "DBusHandler.hpp"

namespace bp = boost::process;
namespace chr = std::chrono;

namespace GLogiK
{

using namespace NSGKUtils;

DBusHandler::DBusHandler(NSGKDBus::GKDBus* pDBus)
	:	_tenSeconds(chr::duration<int>(10)),
		_pDBus(pDBus),
		_sessionBus(NSGKDBus::BusConnection::GKDBUS_SESSION)
{
	this->initializeGKDBusSignals();

	/* initializing first time point */
	_lastCall = chr::steady_clock::now();
	{
		chr::steady_clock::duration elevenSeconds(chr::duration<int>(11));
		_lastCall -= elevenSeconds;
	}

	/* spawn desktop service on start */
	this->restartRequest();
}

DBusHandler::~DBusHandler(void)
{
}

void DBusHandler::cleanDBusRequests(void)
{
	_pDBus->removeSignalsInterface(_sessionBus,
		GLOGIK_DESKTOP_QT5_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DESKTOP_QT5_SESSION_DBUS_OBJECT,
		GLOGIK_DESKTOP_QT5_SESSION_DBUS_INTERFACE);

	_pDBus->removeSignalsInterface(_sessionBus,
		GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT,
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

void DBusHandler::initializeGKDBusSignals(void) {
	_pDBus->NSGKDBus::EventGKDBusCallback<VoidToVoid>::exposeSignal(
		_sessionBus,
		GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		"RestartRequest",
		{},
		std::bind(&DBusHandler::restartRequest, this)
	);

	_pDBus->NSGKDBus::EventGKDBusCallback<VoidToVoid>::exposeSignal(
		_sessionBus,
		GLOGIK_DESKTOP_QT5_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DESKTOP_QT5_SESSION_DBUS_OBJECT,
		GLOGIK_DESKTOP_QT5_SESSION_DBUS_INTERFACE,
		"RestartRequest",
		{},
		std::bind(&DBusHandler::restartRequest, this)
	);
}

void DBusHandler::restartRequest(void)
{
	using steady = chr::steady_clock;

	LOG(INFO) << "received signal: " << __func__;
	LOG(INFO) << "sleeping 1 second before trying to spawn " << GLOGIKS_DESKTOP_SERVICE_NAME;
	std::this_thread::sleep_for(chr::seconds(1));

	const steady::time_point now = steady::now();
	const steady::duration timeLapse = now - _lastCall;
	if(timeLapse > _tenSeconds) {
		try {
			bp::group g;
			bp::spawn(GLOGIKS_DESKTOP_SERVICE_NAME, g);
			g.wait();
		}
		catch (const bp::process_error & e) {
			LOG(ERROR) << "exception catched while trying to spawn process: " << GLOGIKS_DESKTOP_SERVICE_NAME;
			LOG(ERROR) << e.what();
		}

		_lastCall = now;
	}
	else {
		double nsec = static_cast<double>(timeLapse.count()) * steady::period::num / steady::period::den;
		LOG(INFO) << "time lapse since last call : " << nsec << " seconds - ignoring";
	}
}

} // namespace GLogiK

