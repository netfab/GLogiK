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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>

#include <errno.h>

#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#include <syslog.h>

#include <new>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include "lib/shared/glogik.hpp"
#include "lib/utils/utils.hpp"

#include "clientsManager.hpp"

#include "daemon.hpp"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace GLogiK
{

using namespace NSGKUtils;

GLogiKDaemon::GLogiKDaemon()
	:	_pDBus(nullptr)
{
	openlog(GLOGIKD_DAEMON_NAME, LOG_PID|LOG_CONS, LOG_DAEMON);

#if DEBUGGING_ON
	// console output disabled by default
	LOG_TO_FILE_AND_CONSOLE::FileReportingLevel() = DEBUG3;

	if( LOG_TO_FILE_AND_CONSOLE::FileReportingLevel() != NONE ) {
		std::ostringstream buffer(std::ios_base::app);
		buffer	<< DEBUG_DIR << "/" << PACKAGE << "d-debug-"
				<< getpid() << ".log";

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

	if(_LOGfd == nullptr)
		syslog(LOG_INFO, "debug file not opened");
}

GLogiKDaemon::~GLogiKDaemon()
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "exiting daemon process";
#endif

	if( _pidFile.is_open() ) {
		_pidFile.close();

#if DEBUGGING_ON
		LOG(INFO) << "destroying PID file";
#endif

		if( unlink(_pidFileName.c_str()) != 0 ) {
			GKSysLog(LOG_ERR, ERROR, "failed to unlink PID file");
		}
	}

	GKSysLog(LOG_INFO, INFO, "bye !");
	if(_LOGfd != nullptr)
		std::fclose(_LOGfd);
	closelog();
}

int GLogiKDaemon::run( const int& argc, char *argv[] ) {
	std::ostringstream buffer(std::ios_base::app);
	buffer << "Starting " << GLOGIKD_DAEMON_NAME << " vers. " << VERSION;
	GKSysLog(LOG_INFO, INFO, buffer.str());

	try {
		this->dropPrivileges();
		this->parseCommandLine(argc, argv);

		if( GLogiKDaemon::isDaemonRunning() ) {
#if DEBUGGING_ON == 0
			_pid = daemonizeProcess(true);
			syslog(LOG_INFO, "process successfully daemonized");
#else
			_pid = getpid();
			LOG(DEBUG) << "process not daemonized - debug ON - pid: " << _pid;
#endif
			this->createPIDFile();

			// TODO handle return values and errno ?
			std::signal(SIGINT, GLogiKDaemon::handleSignal);
			std::signal(SIGTERM, GLogiKDaemon::handleSignal);
			//std::signal(SIGHUP, GLogiKDaemon::handleSignal);

			try {
				try {
					_pDBus = new NSGKDBus::GKDBus(GLOGIK_DAEMON_DBUS_ROOT_NODE, GLOGIK_DAEMON_DBUS_ROOT_NODE_PATH);
				}
				catch (const std::bad_alloc& e) { /* handle new() failure */
					throw GLogiKBadAlloc("GKDBus bad allocation");
				}

				_pDBus->connectToSystemBus(GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME, NSGKDBus::ConnectionFlag::GKDBUS_SINGLE);

				{
					ClientsManager c(_pDBus);
					c.runLoop();
				}	/* destroy client manager before DBus pointer */

				delete _pDBus; _pDBus = nullptr;
			}
			catch ( const GLogiKExcept & e ) {
				delete _pDBus; _pDBus = nullptr;
				throw;
			}
		}
		else {
			// TODO non-daemon mode
			GKSysLog(LOG_INFO, INFO, "non-daemon mode");
		}

		return EXIT_SUCCESS;
	}
	catch ( const GLogiKExcept & e ) {
		std::ostringstream buffer(std::ios_base::app);
		buffer << e.what();
		if(errno != 0)
			buffer << " : " << strerror(errno);
		GKSysLog(LOG_ERR, ERROR, buffer.str());
		return EXIT_FAILURE;
	}
/*
	catch ( const DisplayHelp & e ) {
		std::cout << "\n" << e.what() << "\n";
		return EXIT_SUCCESS;
	}
*/
}

void GLogiKDaemon::handleSignal(int sig) {
	std::ostringstream buffer("caught signal : ", std::ios_base::app);
	switch( sig ) {
		case SIGINT:
		case SIGTERM:
			buffer << sig << " --> bye bye";
			GKSysLog(LOG_INFO, INFO, buffer.str());
			std::signal(SIGINT, SIG_DFL);
			std::signal(SIGTERM, SIG_DFL);
			GLogiKDaemon::exitDaemon();
			break;
		default:
			buffer << sig << " --> unhandled";
			GKSysLog(LOG_WARNING, WARNING, buffer.str());
			break;
	}
}

void GLogiKDaemon::createPIDFile(void) {
	fs::path path(_pidFileName);

	if( fs::exists(path) ) {
		std::ostringstream buffer(std::ios_base::app);
		buffer	<< "PID file " << _pidFileName << " already exist";
		throw GLogiKExcept( buffer.str() );
	}
	// if path not found reset errno
	errno = 0;

	_pidFile.exceptions( std::ofstream::failbit );
	try {
		_pidFile.open(_pidFileName, std::ofstream::trunc);
		_pidFile << static_cast<long>(_pid);
		_pidFile.flush();

		fs::permissions(path, fs::owner_read|fs::owner_write|fs::group_read|fs::others_read);
	}
	catch (const std::ofstream::failure & e) {
		std::ostringstream buffer(std::ios_base::app);
		buffer	<< "fail to open PID file : " << _pidFileName << " : " << e.what();
		throw GLogiKExcept( buffer.str() );
	}
	catch (const fs::filesystem_error & e) {
		std::ostringstream buffer(std::ios_base::app);
		buffer	<< "set permissions failure on PID file : " << _pidFileName << " : " << e.what();
		throw GLogiKExcept( buffer.str() );
	}
	/*
	 * catch std::ios_base::failure on buggy compilers
	 * should be fixed with gcc >= 7.0
	 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145
	 */
	catch( const std::exception & e ) {
		std::ostringstream buffer(std::ios_base::app);
		buffer << "(buggy exception) fail to open PID file : " << _pidFileName << " : " << e.what();
		throw GLogiKExcept( buffer.str() );
	}

#if DEBUGGING_ON
	LOG(INFO) << "created PID file : " << _pidFileName;
#endif
}

void GLogiKDaemon::parseCommandLine(const int& argc, char *argv[]) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "parsing command line arguments";
#endif

	bool d = false;

	po::options_description desc("Allowed options");
	desc.add_options()
//		("help,h", "produce help message")
		("daemonize,d", po::bool_switch(&d)->default_value(false), "run in daemon mode")
		("pid-file,p", po::value(&_pidFileName), "define the PID file")
	;

	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);
	}
	catch( std::exception & e ) {
		throw GLogiKExcept( e.what() );
	}
	po::notify(vm);

/*
	if (vm.count("help")) {
		GKSysLog(LOG_INFO, INFO, "displaying help");
		std::ostringstream buffer(std::ios_base::app);
		desc.print( buffer );
		throw DisplayHelp( buffer.str() );
	}
*/

	if (vm.count("daemonize")) {
		GLogiKDaemon::daemonized = vm["daemonize"].as<bool>();
	}
}

void GLogiKDaemon::dropPrivileges(void) {
	errno = 0;
	struct passwd * pw = getpwnam(GLOGIKD_USER);
	if(pw == nullptr)
		throw GLogiKExcept("can't get password structure for GLOGIKD_USER");

	errno = 0;
	struct group * gr = getgrnam(GLOGIKD_GROUP);
	if(gr == nullptr)
		throw GLogiKExcept("can't get group structure for GLOGIKD_GROUP");

	errno = 0;
	if(initgroups(GLOGIKD_USER, gr->gr_gid) < 0)
		throw GLogiKExcept("failure to initialize group access list");

	errno = 0;
	int ret = -1;

#if defined(HAVE_SETRESGID)
	ret = setresgid(gr->gr_gid, gr->gr_gid, gr->gr_gid);
#elif defined(HAVE_SETEGID)
	if( (ret = setgid(gr->gr_gid)) == 0 )
		ret = setegid(gr->gr_gid);
#elif defined(HAVE_SETREGID)
	ret = setregid(gr->gr_gid, gr->gr_gid);
#else
#error "No API to drop group privileges"
#endif

	if( ret < 0 )
		throw GLogiKExcept("failure to change group ID");

	errno = 0;
	ret = -1;

#if defined(HAVE_SETRESUID)
	ret = setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid);
#elif defined(HAVE_SETEUID)
	if( (ret = setuid(pw->pw_uid)) == 0 )
		ret = seteuid(pw->pw_uid);
#elif defined(HAVE_SETREUID)
	ret = setreuid(pw->pw_uid, pw->pw_uid);
#else
#error "No API to drop user privileges"
#endif

	if( ret < 0 )
		throw GLogiKExcept("failure to change user ID");

	GKSysLog(LOG_INFO, INFO, "successfully dropped root privileges");
}

} // namespace GLogiK

