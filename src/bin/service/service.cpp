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
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <cstdlib>

#include <syslog.h>

#include <new>
#include <fstream>
#include <iostream>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "lib/shared/sessionManager.hpp"
#include "lib/shared/glogik.hpp"

#include "DBusHandler.hpp"

#include "service.hpp"

namespace fs = boost::filesystem;
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
	openlog(GLOGIKS_DESKTOP_SERVICE_NAME, LOG_PID|LOG_CONS, LOG_USER);

	LOG_TO_FILE_AND_CONSOLE::ConsoleReportingLevel() = INFO;
	if( LOG_TO_FILE_AND_CONSOLE::ConsoleReportingLevel() != NONE ) {
		LOG_TO_FILE_AND_CONSOLE::ConsoleStream() = stderr;
	}

#if DEBUGGING_ON
	LOG_TO_FILE_AND_CONSOLE::FileReportingLevel() = DEBUG3;
#endif
}

DesktopService::~DesktopService() {
#if DEBUGGING_ON
	LOG(DEBUG2) << "exiting desktop service process";
#endif

	LOG(INFO) << GLOGIKS_DESKTOP_SERVICE_NAME << " : bye !";
	if( _LOGfd != nullptr )
		std::fclose(_LOGfd);

	closelog();
}

void DesktopService::openDebugLogFile(void)
{
#if DEBUGGING_ON
	if( LOG_TO_FILE_AND_CONSOLE::FileReportingLevel() != NONE ) {
		const std::string pid( std::to_string( getpid() ) );

		fs::path debugFile(DEBUG_DIR);

		FileSystem::createDirectory(debugFile);

		debugFile /= PACKAGE;
		debugFile += "s-debug-";
		debugFile += pid;
		debugFile += ".log";

		FileSystem::openFile(debugFile, _LOGfd, fs::owner_read|fs::owner_write|fs::group_read);

		LOG_TO_FILE_AND_CONSOLE::FileStream() = _LOGfd;
	}
#endif

	if( _LOGfd == nullptr ) {
		LOG(INFO) << "debug file not opened";
	}
}

int DesktopService::run( const int& argc, char *argv[] ) {
	try {
		this->openDebugLogFile();

		LOG(INFO) << "Starting " << GLOGIKS_DESKTOP_SERVICE_NAME << " vers. " << VERSION;

		this->parseCommandLine(argc, argv);

		_pid = detachProcess();
#if DEBUGGING_ON
		LOG(DEBUG) << "process detached - pid: " << _pid;
#endif

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

			while( session.isSessionAlive() and
					DBus.getExitStatus() )
			{
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

		LOG(ERROR) << e.what();
		return EXIT_FAILURE;
	}
	catch ( const GLogiKFatalError & e ) {
		delete _pGKfs; _pGKfs = nullptr;

		LOG(ERROR) << e.what();
		return EXIT_FAILURE;
	}
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

#if DEBUGGING_ON
	LOG(DEBUG3) << "parsing arguments done";
#endif
}

} // namespace GLogiK

