/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
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

#include <poll.h>

#include <cstring>
#include <cstdlib>

#include <syslog.h>

#include <new>
#include <fstream>
#include <iostream>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <config.h>

#include "lib/utils/GKLogging.hpp"
#include "lib/utils/utils.hpp"
#include "lib/shared/glogik.hpp"
#include "lib/shared/sessionManager.hpp"

#include "DBusHandler.hpp"
#include "launcher.hpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace GLogiK
{

using namespace NSGKUtils;

DesktopServiceLauncher::DesktopServiceLauncher(const int& argc, char *argv[])
	:	_pid(0)
{
	GK_LOG_FUNC

	openlog(DESKTOP_SERVICE_LAUNCHER_NAME, LOG_PID|LOG_CONS, LOG_USER);

	// initialize logging
	try {
		/* boost::po may throw */
		this->parseCommandLine(argc, argv);

#if DEBUGGING_ON
		if(GLogiK::GKDebug) {
			GKLogging::initDebugFile(DESKTOP_SERVICE_LAUNCHER_NAME, fs::owner_read|fs::owner_write|fs::group_read);
		}
#endif

		GKLogging::initConsoleLog();
	}
	catch (const std::exception & e) {
		syslog(LOG_ERR, "%s", e.what());
		throw InitFailure();
	}
}

DesktopServiceLauncher::~DesktopServiceLauncher()
{
	GK_LOG_FUNC

	LOG(info) << DESKTOP_SERVICE_LAUNCHER_NAME << " desktop service launcher process, bye !";

	closelog();
}

int DesktopServiceLauncher::run(void)
{
	GK_LOG_FUNC

	/* -- -- -- */
	/* -- -- -- */
	/* -- -- -- */

	LOG(info) << "Starting " << DESKTOP_SERVICE_LAUNCHER_NAME << " vers. " << VERSION;

	_pid = detachProcess();

	GKLog2(trace, "process detached - pid : ", _pid)

	{
		SessionManager session;

		NSGKDBus::GKDBus DBus(GLOGIK_DESKTOP_SERVICE_LAUNCHER_DBUS_ROOT_NODE,
			GLOGIK_DESKTOP_SERVICE_LAUNCHER_DBUS_ROOT_NODE_PATH);

		DBus.connectToSessionBus(GLOGIK_DESKTOP_SERVICE_LAUNCHER_DBUS_BUS_CONNECTION_NAME,
			NSGKDBus::ConnectionFlag::GKDBUS_SINGLE);

		struct pollfd fds[1];
		nfds_t nfds = 1;

		fds[0].fd = session.openConnection();
		fds[0].events = POLLIN;

		DBusHandler handler(&DBus);

		while( session.isSessionAlive() ) {
			int num = poll(fds, nfds, 150);

			// data to read ?
			if( num > 0 ) {
				if( fds[0].revents & POLLIN ) {
					session.processICEMessages();
					continue;
				}
			}

			DBus.checkForMessages();
		}

		handler.cleanDBusRequests();

		DBus.disconnectFromSessionBus();
	}

	GKLog(trace, "exiting with success")

	return EXIT_SUCCESS;
}

void DesktopServiceLauncher::parseCommandLine(const int& argc, char *argv[])
{
	GK_LOG_FUNC

	bool debug = false;

	po::options_description desc("Allowed options");
	desc.add_options()
		("debug,D", po::bool_switch(&debug)->default_value(false), "run in debug mode")
	;

	po::variables_map vm;

	po::store(po::parse_command_line(argc, argv, desc), vm);

	po::notify(vm);

	if (vm.count("debug")) {
		GLogiK::GKDebug = vm["debug"].as<bool>();
	}
}

} // namespace GLogiK

