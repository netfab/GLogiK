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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <cstdlib>
#include <csignal>

#include <new>
#include <fstream>
#include <iostream>
#include <sstream>

#include <boost/program_options.hpp>

#include "lib/shared/sessionManager.hpp"

#include "global.hpp"
#include "DBusHandler.hpp"

#include "service.hpp"

namespace po = boost::program_options;

namespace GLogiK
{

using namespace NSGKUtils;

DesktopService::DesktopService() :
	_pid(0),
	_LOGfd(nullptr),
	_verbose(false),
	_pGKfs(nullptr)
{
	LOG_TO_FILE_AND_CONSOLE::ConsoleReportingLevel() = INFO;
	if( LOG_TO_FILE_AND_CONSOLE::ConsoleReportingLevel() != NONE ) {
		LOG_TO_FILE_AND_CONSOLE::ConsoleStream() = stderr;
	}

#if DEBUGGING_ON
	LOG_TO_FILE_AND_CONSOLE::FileReportingLevel() = DEBUG3;

	if( LOG_TO_FILE_AND_CONSOLE::FileReportingLevel() != NONE ) {
		std::ostringstream buffer(DEBUG_DIR, std::ios_base::app);
		buffer << "/" << PACKAGE << "s-debug-" << getpid() << ".log";

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

DesktopService::~DesktopService() {
#if DEBUGGING_ON
	LOG(DEBUG2) << "exiting desktop service process";
#endif

	LOG(INFO) << GLOGIKS_DESKTOP_SERVICE_NAME << " : bye !";
	if( _LOGfd != nullptr )
		std::fclose(_LOGfd);
}

int DesktopService::run( const int& argc, char *argv[] ) {
	LOG(INFO) << "Starting " << GLOGIKS_DESKTOP_SERVICE_NAME << " vers. " << VERSION;

	try {
		this->parseCommandLine(argc, argv);

		this->daemonize();

		{
			try {
				_pGKfs = new FileSystem();
			}
			catch (const std::bad_alloc& e) { /* handle new() failure */
				throw GLogiKBadAlloc("GKfs bad allocation");
			}

			SessionManager session;

			struct pollfd fds[2];
			nfds_t nfds = 2;

			fds[0].fd = session.openConnection();
			fds[0].events = POLLIN;

			fds[1].fd = _pGKfs->getNotifyQueueDescriptor();
			fds[1].events = POLLIN;

			DBusHandler DBus(_pid, session, _pGKfs);

			while( session.isSessionAlive() ) {
				int num = poll(fds, nfds, 150);

				// data to read ?
				if( num > 0 ) {
					if( fds[0].revents & POLLIN ) {
						session.processICEMessages();
						continue;
					}

					if( fds[1].revents & POLLIN) {
						/* checking if any received filesystem notification matches
						 * any device configuration file. If yes, reload the file,
						 * and send configuration to daemon */
						DBus.checkNotifyEvents(_pGKfs);
					}
				}

				DBus.updateSessionState();
				DBus.checkDBusMessages();
			}
		}

		delete _pGKfs; _pGKfs = nullptr;

#if DEBUGGING_ON
		LOG(DEBUG) << "exiting with success";
#endif
		return EXIT_SUCCESS;
	}
	catch ( const GLogiKExcept & e ) {
		delete _pGKfs; _pGKfs = nullptr;

		std::ostringstream buffer(e.what(), std::ios_base::app);
		if(errno != 0)
			buffer << " : " << strerror(errno);
		LOG(ERROR) << buffer.str();
		return EXIT_FAILURE;
	}
	catch ( const GLogiKFatalError & e ) {
		delete _pGKfs; _pGKfs = nullptr;

		LOG(ERROR) << e.what();
		return EXIT_FAILURE;
	}
}

void DesktopService::daemonize() {
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

void DesktopService::parseCommandLine(const int& argc, char *argv[]) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "parsing command line arguments";
#endif

	po::options_description desc("Allowed options");
	desc.add_options()
		("verbose,v", po::bool_switch(&_verbose)->default_value(false), "verbose mode")
	;

	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);
	}
	catch( std::exception & e ) {
		throw GLogiKExcept( e.what() );
	}
	po::notify(vm);

	if( _verbose ) {
		LOG_TO_FILE_AND_CONSOLE::ConsoleReportingLevel() = VERB;
		LOG(VERB) << "verbose mode on";
	}
}

} // namespace GLogiK

