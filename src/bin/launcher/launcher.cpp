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

#include <poll.h>

#include <cstring>
#include <cstdlib>

#include <syslog.h>

#include <new>
#include <fstream>
#include <iostream>
#include <sstream>

#include <boost/filesystem.hpp>
//#include <boost/program_options.hpp>

#include "lib/utils/utils.hpp"
#include "lib/shared/sessionManager.hpp"

#include "DBusHandler.hpp"
#include "launcher.hpp"

namespace fs = boost::filesystem;

namespace GLogiK
{

using namespace NSGKUtils;

DesktopServiceLauncher::DesktopServiceLauncher() :
	_pid(0),
	_LOGfd(nullptr)
{
	openlog(DESKTOP_SERVICE_LAUNCHER_NAME, LOG_PID|LOG_CONS, LOG_USER);

	LOG_TO_FILE_AND_CONSOLE::ConsoleReportingLevel() = INFO;
	if( LOG_TO_FILE_AND_CONSOLE::ConsoleReportingLevel() != NONE ) {
		LOG_TO_FILE_AND_CONSOLE::ConsoleStream() = stderr;
	}

#if DEBUGGING_ON
	LOG_TO_FILE_AND_CONSOLE::FileReportingLevel() = DEBUG3;
#endif
}

DesktopServiceLauncher::~DesktopServiceLauncher() {
#if DEBUGGING_ON
	LOG(DEBUG2) << "exiting desktop service launcher process";
#endif

	LOG(INFO) << DESKTOP_SERVICE_LAUNCHER_NAME << " : bye !";
	if( _LOGfd != nullptr )
		std::fclose(_LOGfd);

	closelog();
}

int DesktopServiceLauncher::run( const int& argc, char *argv[] ) {
	try {

#if DEBUGGING_ON
		FileSystem::openDebugFile(DESKTOP_SERVICE_LAUNCHER_NAME, _LOGfd, fs::owner_read|fs::owner_write|fs::group_read);
#endif

		LOG(INFO) << "Starting " << DESKTOP_SERVICE_LAUNCHER_NAME << " vers. " << VERSION;

		/* no expected arguments */
		//this->parseCommandLine(argc, argv);

		_pid = detachProcess();
#if DEBUGGING_ON
		LOG(DEBUG) << "process detached - pid: " << _pid;
#endif

		{
			SessionManager session;

			struct pollfd fds[1];
			nfds_t nfds = 1;

			fds[0].fd = session.openConnection();
			fds[0].events = POLLIN;

			LauncherDBusHandler DBus;
			while( session.isSessionAlive() ) {
				int num = poll(fds, nfds, 150);

				// data to read ?
				if( num > 0 ) {
					if( fds[0].revents & POLLIN ) {
						session.processICEMessages();
						continue;
					}
				}

				DBus.checkDBusMessages();
			}
		}

#if DEBUGGING_ON
		LOG(DEBUG) << "exiting with success";
#endif
		return EXIT_SUCCESS;
	}
	catch ( const GLogiKExcept & e ) {
		LOG(ERROR) << e.what();
		return EXIT_FAILURE;
	}

	return EXIT_FAILURE;
}

} // namespace GLogiK

