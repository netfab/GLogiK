/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2017  Fabrice Delliaux <netbox253@gmail.com>
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
#include <thread>
#include <chrono>

#include "include/log.h"

#include <config.h>

#include "lib/utils/utils.h"

#include "service.h"

namespace GLogiK
{

bool DesktopService::still_running_ = true;

DesktopService::DesktopService() : buffer_("", std::ios_base::app) {

#ifdef DEBUGGING_ON
	if( FILELog::ReportingLevel() != NONE ) {
		std::stringstream log_file;
		log_file << DEBUG_DIR << "/glogiks-debug-" << getpid() << ".log";
		this->log_fd_ = std::fopen( log_file.str().c_str(), "w" );
	}

	LOG2FILE::Stream() = this->log_fd_;
#endif

	if( this->log_fd_ == nullptr )
		GK_STAT << "debug file not opened\n";
}

DesktopService::~DesktopService() {
	LOG(DEBUG2) << "exiting desktop service process";

	LOG(INFO) << "bye !";
	GK_STAT << GLOGIKS_DESKTOP_SERVICE_NAME << ": bye !\n";
	if( this->log_fd_ != nullptr )
		std::fclose(this->log_fd_);
}

int DesktopService::run( const int& argc, char *argv[] ) {
	this->buffer_.str( "Starting " );
	this->buffer_ << GLOGIKS_DESKTOP_SERVICE_NAME << " vers. " << VERSION ;
	GK_STAT << this->buffer_.str().c_str() << "\n";
	LOG(INFO) << this->buffer_.str();

	try {
		this->daemonize();

		std::signal(SIGINT, DesktopService::handle_signal);
		std::signal(SIGTERM, DesktopService::handle_signal);

		try {
			this->DBus = new GKDBus();
			this->DBus->connectToSystemBus(GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME);

			while( DesktopService::still_running_ ) {
				LOG(INFO) << "ok, run";
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}

			if(this->DBus != nullptr)
				delete this->DBus;
		}
		catch ( const GLogiKExcept & e ) {
			if(this->DBus != nullptr)
				delete this->DBus;
			throw;
		}

		return EXIT_SUCCESS;
	}
	catch ( const GLogiKExcept & e ) {
		std::ostringstream buff(e.what(), std::ios_base::app);
		if(errno != 0)
			buff << " : " << strerror(errno);
		LOG(ERROR) << buff.str();
		GK_ERR << buff.str().c_str() << "\n";
		return EXIT_FAILURE;
	}
}

void DesktopService::handle_signal(int sig) {
	std::ostringstream buff("caught signal : ", std::ios_base::app);
	switch( sig ) {
		case SIGINT:
		case SIGTERM:
			buff << sig << " --> bye bye";
			LOG(INFO) << buff.str();
			GK_STAT << buff.str().c_str() << "\n";
			std::signal(SIGINT, SIG_DFL);
			std::signal(SIGTERM, SIG_DFL);
			DesktopService::still_running_ = false;
			break;
		default:
			buff << sig << " --> unhandled";
			LOG(WARNING) << buff.str();
			GK_WARN << buff.str().c_str() << "\n";
			break;
	}
}

void DesktopService::daemonize() {
	//int fd = 0;

	LOG(DEBUG2) << "daemonizing process";

	this->pid_ = fork();
	if(this->pid_ == -1)
		throw GLogiKExcept("first fork failure");

	// parent exit
	if(this->pid_ > 0)
		exit(EXIT_SUCCESS);

	LOG(DEBUG3) << "first fork ! pid:" << getpid();

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
	
	LOG(DEBUG3) << "second fork ! pid:" << getpid();

	umask(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if(chdir("/") == -1)
		throw GLogiKExcept("change directory failure");

	this->pid_ = getpid();
	LOG(INFO) << "daemonized !";

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

