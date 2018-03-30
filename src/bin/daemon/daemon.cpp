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
#include <sys/stat.h>

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

#include "lib/shared/glogik.h"
#include "lib/utils/utils.h"

#include "clientsManager.h"

#include "daemon.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace GLogiK
{

using namespace NSGKUtils;

GLogiKDaemon::GLogiKDaemon() : buffer_("", std::ios_base::app), pDBus_(nullptr)
{
	openlog(GLOGIKD_DAEMON_NAME, LOG_PID|LOG_CONS, LOG_DAEMON);

#if DEBUGGING_ON
	// console output disabled by default
	LOG_TO_FILE_AND_CONSOLE::FileReportingLevel() = DEBUG3;

	if( LOG_TO_FILE_AND_CONSOLE::FileReportingLevel() != NONE ) {
		this->buffer_.str(DEBUG_DIR);
		this->buffer_ << "/" << PACKAGE << "d-debug-" << getpid() << ".log";

		errno = 0;
		this->log_fd_ = std::fopen(this->buffer_.str().c_str(), "w");

		if(this->log_fd_ == nullptr) {
			LOG(ERROR) << "failed to open debug file";
			if(errno != 0) {
				LOG(ERROR) << strerror(errno);
			}
		}

		LOG_TO_FILE_AND_CONSOLE::FileStream() = this->log_fd_;
	}
#endif

	if( this->log_fd_ == nullptr )
		syslog(LOG_INFO, "debug file not opened");
}

GLogiKDaemon::~GLogiKDaemon()
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "exiting daemon process";
#endif

	if( this->pid_file_.is_open() ) {
		this->pid_file_.close();

#if DEBUGGING_ON
		LOG(INFO) << "destroying PID file";
#endif

		if( unlink(this->pid_file_name_.c_str()) != 0 ) {
			GKSysLog(LOG_ERR, ERROR, "failed to unlink PID file");
		}
	}

	GKSysLog(LOG_INFO, INFO, "bye !");
	if( this->log_fd_ != nullptr )
		std::fclose(this->log_fd_);
	closelog();
}

int GLogiKDaemon::run( const int& argc, char *argv[] ) {

	this->buffer_.str( "Starting " );
	this->buffer_ << GLOGIKD_DAEMON_NAME << " vers. " << VERSION ;
	GKSysLog(LOG_INFO, INFO, this->buffer_.str());

	try {
		this->dropPrivileges();
		this->parseCommandLine(argc, argv);

		if( GLogiKDaemon::isDaemonRunning() ) {
#if DEBUGGING_ON == 0
			this->daemonize();
#endif
			this->createPIDFile();

			// TODO handle return values and errno ?
			std::signal(SIGINT, GLogiKDaemon::handle_signal);
			std::signal(SIGTERM, GLogiKDaemon::handle_signal);
			//std::signal(SIGHUP, GLogiKDaemon::handle_signal);

			try {
				try {
					this->pDBus_ = new NSGKDBus::GKDBus(GLOGIK_DAEMON_DBUS_ROOT_NODE);
				}
				catch (const std::bad_alloc& e) { /* handle new() failure */
					throw GLogiKBadAlloc("GKDBus bad allocation");
				}

				this->pDBus_->connectToSystemBus(GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME);

				{
					ClientsManager c(this->pDBus_);
					c.runLoop();
				}	/* destroy client manager before DBus pointer */

				delete this->pDBus_;
				this->pDBus_ = nullptr;
			}
			catch ( const GLogiKExcept & e ) {
				delete this->pDBus_;
				this->pDBus_ = nullptr;
				throw;
			}
		}
		else {
			// TODO
			GKSysLog(LOG_INFO, INFO, "non-daemon mode");
		}

		return EXIT_SUCCESS;
	}
	catch ( const GLogiKExcept & e ) {
		this->buffer_.str( e.what() );
		if(errno != 0)
			this->buffer_ << " : " << strerror(errno);
		GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
		return EXIT_FAILURE;
	}
/*
	catch ( const DisplayHelp & e ) {
		std::cout << "\n" << e.what() << "\n";
		return EXIT_SUCCESS;
	}
*/
}

void GLogiKDaemon::handle_signal(int sig) {
	std::ostringstream buff("caught signal : ", std::ios_base::app);
	switch( sig ) {
		case SIGINT:
		case SIGTERM:
			buff << sig << " --> bye bye";
			GKSysLog(LOG_INFO, INFO, buff.str());
			std::signal(SIGINT, SIG_DFL);
			std::signal(SIGTERM, SIG_DFL);
			GLogiKDaemon::exitDaemon();
			break;
		default:
			buff << sig << " --> unhandled";
			GKSysLog(LOG_WARNING, WARNING, buff.str());
			break;
	}
}

void GLogiKDaemon::daemonize() {
	//int fd = 0;

#if DEBUGGING_ON
	LOG(DEBUG2) << "daemonizing process";
#endif

	this->pid_ = fork();
	if(this->pid_ == -1)
		throw GLogiKExcept("first fork failure");

	// parent exit
	if(this->pid_ > 0)
		exit(EXIT_SUCCESS);

#if DEBUGGING_ON
	LOG(DEBUG3) << "first fork ! pid:" << getpid();
#endif

	if(setsid() == -1)
		throw GLogiKExcept("session creation failure");

	// Ignore signal sent from child to parent process
	std::signal(SIGCHLD, SIG_IGN);

	this->pid_ = fork();
	if(this->pid_ == -1)
		throw GLogiKExcept("second fork failure");

	// parent exit
	if(this->pid_ > 0)
		exit(EXIT_SUCCESS);

#if DEBUGGING_ON
	LOG(DEBUG3) << "second fork ! pid:" << getpid();
#endif

	umask(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if(chdir("/") == -1)
		throw GLogiKExcept("change directory failure");

#if DEBUGGING_ON == 0
	// closing opened descriptors
	//for(fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--)
	//	close(fd);
	std::fclose(stdin);
	std::fclose(stdout);
	std::fclose(stderr);
	
	// reopening standard outputs
	stdin = std::fopen("/dev/null", "r");
	stdout = std::fopen("/dev/null", "w+");
	stderr = std::fopen("/dev/null", "w+");
#endif

	this->pid_ = getpid();

#if DEBUGGING_ON
	LOG(INFO) << "daemonized !";
#endif
}

void GLogiKDaemon::createPIDFile(void) {
	fs::path path(this->pid_file_name_);

	if( fs::exists(path) ) {
		this->buffer_.str( "PID file " );
		this->buffer_ << this->pid_file_name_ << " already exist";
		throw GLogiKExcept( this->buffer_.str() );
	}
	// if path not found reset errno
	errno = 0;

	this->pid_file_.exceptions( std::ofstream::failbit );
	try {
		this->pid_file_.open(this->pid_file_name_.c_str(), std::ofstream::trunc);
		this->pid_file_ << (long)this->pid_;
		this->pid_file_.flush();

		fs::permissions(path, fs::owner_read|fs::owner_write|fs::group_read|fs::others_read);
	}
	catch (const std::ofstream::failure & e) {
		this->buffer_.str( "fail to open PID file : " );
		this->buffer_ << this->pid_file_name_ << " : " << e.what();
		throw GLogiKExcept( this->buffer_.str() );
	}
	catch (const fs::filesystem_error & e) {
		this->buffer_.str( "set permissions failure on PID file : " );
		this->buffer_ << this->pid_file_name_ << " : " << e.what();
		throw GLogiKExcept( this->buffer_.str() );
	}
	/*
	 * catch std::ios_base::failure on buggy compilers
	 * should be fixed with gcc >= 7.0
	 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145
	 */
	catch( const std::exception & e ) {
		this->buffer_.str( "(buggy exception) fail to open PID file : " );
		this->buffer_ << this->pid_file_name_ << " : " << e.what();
		throw GLogiKExcept( this->buffer_.str() );
	}

#if DEBUGGING_ON
	LOG(INFO) << "created PID file : " << this->pid_file_name_;
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
		("pid-file,p", po::value(&this->pid_file_name_), "define the PID file")
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
		this->buffer_.str("");
		desc.print( this->buffer_ );
		throw DisplayHelp( this->buffer_.str() );
	}
*/

	if (vm.count("daemonize")) {
		GLogiKDaemon::daemonized_ = vm["daemonize"].as<bool>();
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

