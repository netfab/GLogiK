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

#include "lib/dbus/GKDBus.hpp"
#include "lib/utils/utils.hpp"
#include "lib/shared/sessionManager.hpp"
#include "lib/shared/glogik.hpp"

#include "DBusHandler.hpp"

#include "service.hpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace GLogiK
{

using namespace NSGKUtils;

DesktopService::DesktopService(const int& argc, char *argv[]) :
	_pid(0)
{
	GK_LOG_FUNC

	openlog(GLOGIKS_DESKTOP_SERVICE_NAME, LOG_PID|LOG_CONS, LOG_USER);

	// initialize logging
	try {
		/* boost::po may throw */
		this->parseCommandLine(argc, argv);

#if DEBUGGING_ON
		if(GLogiK::GKDebug) {
			GKLogging::initDebugFile(GLOGIKS_DESKTOP_SERVICE_NAME, fs::owner_read|fs::owner_write|fs::group_read);
		}
#endif
		GKLogging::initConsoleLog();
	}
	catch (const std::exception & e) {
		syslog(LOG_ERR, "%s", e.what());
		throw InitFailure();
	}
}

DesktopService::~DesktopService()
{
	GK_LOG_FUNC

	LOG(info) << GLOGIKS_DESKTOP_SERVICE_NAME << " desktop service process exiting, bye !";

	closelog();
}

int DesktopService::run(void)
{
	GK_LOG_FUNC

	/* -- -- -- */
	/* -- -- -- */
	/* -- -- -- */

	LOG(info) << "Starting " << GLOGIKS_DESKTOP_SERVICE_NAME << " vers. " << VERSION;

	_pid = detachProcess();

	GKLog2(trace, "process detached - pid: ", _pid)

	{
		FileSystem GKfs;
		SessionManager session;

		NSGKDBus::GKDBus DBus(GLOGIK_DESKTOP_SERVICE_DBUS_ROOT_NODE, GLOGIK_DESKTOP_SERVICE_DBUS_ROOT_NODE_PATH);

		DBus.connectToSystemBus(GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME);
		DBus.connectToSessionBus(GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME);

		struct pollfd fds[2];
		nfds_t nfds = 2;

		fds[0].fd = session.openConnection();
		fds[0].events = POLLIN;

		fds[1].fd = GKfs.getNotifyQueueDescriptor();
		fds[1].events = POLLIN;

		DBusHandler handler(_pid, session, &GKfs, &DBus);

		while( session.isSessionAlive() and
				handler.getExitStatus() )
		{
			int num = poll(fds, nfds, 150);

			// data to read ?
			if( num > 0 ) {
				if( fds[0].revents & POLLIN ) {
					session.processICEMessages();
					continue;
				}

				if( fds[1].revents & POLLIN ) {
					/* checking if any received filesystem notification matches
					 * any device configuration file. If yes, reload the file,
					 * and send configuration to daemon */
					handler.checkNotifyEvents(&GKfs);
				}
			}

			DBus.checkForMessages();
		}

		// also unregister with daemon before cleaning
		handler.cleanDBusRequests();

		DBus.disconnectFromSessionBus();
		DBus.disconnectFromSystemBus();
	}

	GKLog(trace, "exiting with success")

	return EXIT_SUCCESS;
}

void DesktopService::parseCommandLine(const int& argc, char *argv[])
{
	GK_LOG_FUNC

	po::options_description desc("Allowed options");

#if DEBUGGING_ON
	bool debug = false;

	desc.add_options()
		("debug,D", po::bool_switch(&debug)->default_value(false), "run in debug mode")
	;
#endif

	po::variables_map vm;

	po::store(po::parse_command_line(argc, argv, desc), vm);

	po::notify(vm);

#if DEBUGGING_ON
	if (vm.count("debug")) {
		GLogiK::GKDebug = vm["debug"].as<bool>();
	}
#endif
}

} // namespace GLogiK

