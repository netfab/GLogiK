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

#include <errno.h>

#include <poll.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include <cstring>
#include <cstdlib>
#include <csignal>

//#include <new>
#include <fstream>
#include <iostream>
#include <sstream>

//#include <boost/program_options.hpp>

#include "lib/utils/utils.hpp"
#include "lib/shared/sessionManager.hpp"

#include "launcher.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

DesktopServiceLauncher::DesktopServiceLauncher() :
	_pid(0),
	_LOGfd(nullptr)
{
	LOG_TO_FILE_AND_CONSOLE::ConsoleReportingLevel() = INFO;
	if( LOG_TO_FILE_AND_CONSOLE::ConsoleReportingLevel() != NONE ) {
		LOG_TO_FILE_AND_CONSOLE::ConsoleStream() = stderr;
	}

#if DEBUGGING_ON
	LOG_TO_FILE_AND_CONSOLE::FileReportingLevel() = DEBUG3;

	if( LOG_TO_FILE_AND_CONSOLE::FileReportingLevel() != NONE ) {
		// FIXME boost path
		std::ostringstream buffer(DEBUG_DIR, std::ios_base::app);
		buffer << "/" << DESKTOP_SERVICE_LAUNCHER_NAME << "-debug-" << getpid() << ".log";

		errno = 0;
		_LOGfd = std::fopen(buffer.str().c_str(), "w");

		if(_LOGfd == nullptr) {
			LOG(ERROR) << "failed to open debug file";
			if(errno != 0) {
				LOG(ERROR) << strerror(errno);
			}
		}

		LOG_TO_FILE_AND_CONSOLE::FileStream() = _LOGfd;
	}
#endif

	if( _LOGfd == nullptr ) {
		LOG(INFO) << "debug file not opened";
	}
}

DesktopServiceLauncher::~DesktopServiceLauncher() {
#if DEBUGGING_ON
	LOG(DEBUG2) << "exiting desktop service launcher process";
#endif

	LOG(INFO) << DESKTOP_SERVICE_LAUNCHER_NAME << " : bye !";
	if( _LOGfd != nullptr )
		std::fclose(_LOGfd);
}

void DesktopServiceLauncher::daemonize() {
#if DEBUGGING_ON
	LOG(DEBUG2) << "daemonizing process";
#endif

	_pid = fork();
	if(_pid == -1)
		throw GLogiKExcept("first fork failure");

	// parent exit
	if(_pid > 0)
		exit(EXIT_SUCCESS);

#if DEBUGGING_ON
	LOG(DEBUG3) << "first fork ! pid:" << getpid();
#endif

	if(setsid() == -1)
		throw GLogiKExcept("session creation failure");

	// Ignore signal sent from child to parent process
	std::signal(SIGCHLD, SIG_IGN);

	_pid = fork();
	if(_pid == -1)
		throw GLogiKExcept("second fork failure");

	// parent exit
	if(_pid > 0)
		exit(EXIT_SUCCESS);

#if DEBUGGING_ON
	LOG(DEBUG3) << "second fork ! pid:" << getpid();
#endif

	umask(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if(chdir("/") == -1)
		throw GLogiKExcept("change directory failure");

	_pid = getpid();
#if DEBUGGING_ON
	LOG(DEBUG) << "daemonized !";
#endif
}

int DesktopServiceLauncher::run( const int& argc, char *argv[] ) {
	LOG(INFO) << "Starting " << DESKTOP_SERVICE_LAUNCHER_NAME << " vers. " << VERSION;

	try {
		/* no expected arguments */
		//this->parseCommandLine(argc, argv);

		this->daemonize();

		{
			SessionManager session;

			struct pollfd fds[1];
			nfds_t nfds = 1;

			fds[0].fd = session.openConnection();
			fds[0].events = POLLIN;

			while( session.isSessionAlive() ) {
				int num = poll(fds, nfds, 150);

				// data to read ?
				if( num > 0 ) {
					if( fds[0].revents & POLLIN ) {
						session.processICEMessages();
						continue;
					}
				}
			}
		}

#if DEBUGGING_ON
		LOG(DEBUG) << "exiting with success";
#endif
		return EXIT_SUCCESS;
	}
	catch ( const GLogiKExcept & e ) {
		std::ostringstream buffer(e.what(), std::ios_base::app);
		if(errno != 0)
			buffer << " : " << strerror(errno);
		LOG(ERROR) << buffer.str();
		return EXIT_FAILURE;
	}

	return EXIT_FAILURE;
}

} // namespace GLogiK

