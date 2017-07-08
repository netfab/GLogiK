
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <sys/stat.h>

#include <errno.h>

#include <sys/types.h>
#include <unistd.h>

#include <syslog.h>

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <config.h>

#include "exception.h"
#include "daemon.h"
#include "globals.h"
#include "include/log.h"

#include "devices_manager.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace GLogiKd
{

GLogiKDaemon::GLogiKDaemon() : buffer_("", std::ios_base::app)
{
	openlog(GLOGIKD_DAEMON_NAME, LOG_PID|LOG_CONS, LOG_DAEMON);

#ifdef DEBUGGING_ON
	FILELog::ReportingLevel() = FILELog::FromString(DEBUG_LOG_LEVEL);

	if( FILELog::ReportingLevel() != NONE ) {
		std::stringstream log_file;
		log_file << DEBUG_DIR << "/glogikd-debug-" << getpid() << ".log";
		this->log_fd_ = std::fopen( log_file.str().c_str(), "w" );
	}

	LOG2FILE::Stream() = this->log_fd_;
#endif

	if( this->log_fd_ == nullptr )
		syslog(LOG_INFO, "debug file not opened");
}

GLogiKDaemon::~GLogiKDaemon()
{
	LOG(DEBUG2) << "exiting daemon process";
	if( this->pid_file_.is_open() ) {
		this->pid_file_.close();
		LOG(INFO) << "destroying PID file";
		if( unlink(this->pid_file_name_.c_str()) != 0 ) {
			const char * msg = "failed to unlink PID file";
			syslog(LOG_ERR, msg);
			LOG(ERROR) << msg;
		}
	}

	LOG(INFO) << "bye !";
	if( this->log_fd_ != nullptr )
		std::fclose(this->log_fd_);
	closelog();
}

int GLogiKDaemon::run( const int& argc, char *argv[] ) {

	this->buffer_.str( "Starting " );
	this->buffer_ << GLOGIKD_DAEMON_NAME << " vers. " << VERSION ;
	syslog(LOG_INFO, this->buffer_.str().c_str() );
	LOG(INFO) << this->buffer_.str();

	try {
		this->parseCommandLine(argc, argv);

		DevicesManager d;

		if( GLogiKDaemon::is_daemon_enabled() ) {
			this->daemonize();

			std::signal(SIGINT, GLogiKDaemon::handle_signal);
			std::signal(SIGHUP, GLogiKDaemon::handle_signal);

			d.startMonitoring();
		}
		else {
			syslog(LOG_INFO, "non-daemon mode" );
			LOG(INFO) << "non-daemon mode";
			;
		}

		return EXIT_SUCCESS;
	}
	catch ( const GLogiKExcept & e ) {
		std::ostringstream buff(e.what(), std::ios_base::app);
		if(errno != 0)
			buff << " : " << strerror(errno);
		syslog( LOG_ERR, buff.str().c_str() );
		LOG(ERROR) << buff.str();
		return EXIT_FAILURE;
	}
	catch ( const DisplayHelp & e ) {
		std::cout << "\n" << e.what() << "\n";
		return EXIT_SUCCESS;
	}
}

void GLogiKDaemon::handle_signal(int sig) {
	switch( sig ) {
		case SIGINT: {
			const char * msg = "catch SIGINT signal : bye bye";
			syslog(LOG_INFO, msg);
			LOG(INFO) << msg;
			GLogiKDaemon::disable_daemon();
			std::signal(SIGINT, SIG_DFL);
		}
	}
}

void GLogiKDaemon::daemonize() {
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

#ifndef DEBUGGING_ON
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
	LOG(INFO) << "daemonized !";

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
}

void GLogiKDaemon::parseCommandLine(const int& argc, char *argv[]) {
	LOG(DEBUG2) << "parsing command line arguments";

	bool d = false;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "produce help message")
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

	if (vm.count("help")) {
		const char * msg = "displaying help";
		syslog(LOG_INFO, msg);
		LOG(INFO) << msg;
		desc.print( this->buffer_ );
		throw DisplayHelp( this->buffer_.str() );
	}

	if (vm.count("daemonize")) {
		GLogiKDaemon::daemonized_ = vm["daemonize"].as<bool>();
	}
}

} // namespace GLogiKd

