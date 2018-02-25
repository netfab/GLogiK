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

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <csignal>

#include <fstream>
#include <iostream>

#include "lib/utils/utils.h"
#include "lib/shared/sessionManager.h"

#include "serviceDBusHandler.h"

#include "service.h"

namespace GLogiK
{

using namespace NSGKUtils;

DesktopService::DesktopService() : buffer_("", std::ios_base::app)
{
	LOG_TO_FILE_AND_CONSOLE::ConsoleReportingLevel() = INFO;
	if( LOG_TO_FILE_AND_CONSOLE::ConsoleReportingLevel() != NONE ) {
		LOG_TO_FILE_AND_CONSOLE::ConsoleStream() = stderr;
	}

#if DEBUGGING_ON
	LOG_TO_FILE_AND_CONSOLE::FileReportingLevel() = DEBUG3;

	if( LOG_TO_FILE_AND_CONSOLE::FileReportingLevel() != NONE ) {
		this->buffer_.str(DEBUG_DIR);
		this->buffer_ << "/" << PACKAGE << "s-debug-" << getpid() << ".log";

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

	if( this->log_fd_ == nullptr ) {
		LOG(INFO) << "debug file not opened";
	}

	this->fds[0].fd = -1;
	this->fds[0].events = POLLIN;
}

DesktopService::~DesktopService() {
#if DEBUGGING_ON
	LOG(DEBUG2) << "exiting desktop service process";
#endif

	LOG(INFO) << GLOGIKS_DESKTOP_SERVICE_NAME << " : bye !";
	if( this->log_fd_ != nullptr )
		std::fclose(this->log_fd_);
}

int DesktopService::run( const int& argc, char *argv[] ) {
	LOG(INFO) << "Starting " << GLOGIKS_DESKTOP_SERVICE_NAME << " vers. " << VERSION;

	try {
		this->daemonize();

		{
			SessionManager session;
			this->fds[0].fd = session.openConnection();

			ServiceDBusHandler DBusHandler(this->pid_, session);

			while( session.isSessionAlive() ) {
				int ret = poll(this->fds, 1, 150);
				// data to read ?
				if( ret > 0 ) {
					session.processICEMessages();
					continue;
				}
				DBusHandler.updateSessionState();
				DBusHandler.checkDBusMessages();
			}
		}

#if DEBUGGING_ON
		LOG(DEBUG) << "exiting with success";
#endif
		return EXIT_SUCCESS;
	}
	catch ( const GLogiKExcept & e ) {
		this->buffer_.str( e.what() );
		if(errno != 0)
			this->buffer_ << " : " << strerror(errno);
		LOG(ERROR) << this->buffer_.str();
		return EXIT_FAILURE;
	}
}

void DesktopService::daemonize() {
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

	this->pid_ = getpid();
#if DEBUGGING_ON
	LOG(DEBUG) << "daemonized !";
#endif

/*
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
		this->buffer_.str( "Fail to open PID file : " );
		this->buffer_ << this->pid_file_name_ << " : " << e.what();
		throw GLogiKExcept( this->buffer_.str() );
	}
	catch (const fs::filesystem_error & e) {
		this->buffer_.str( "Set permissions failure on PID file : " );
		this->buffer_ << this->pid_file_name_ << " : " << e.what();
		throw GLogiKExcept( this->buffer_.str() );
	}
	LOG(INFO) << "created PID file : " << this->pid_file_name_;
*/
}

} // namespace GLogiK

