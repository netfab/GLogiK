
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

#include <config.h>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include "exception.h"
#include "daemon.h"
#include "globals.h"
#include "include/log.h"

#include "devices_manager.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace GLogiKd
{

boost::atomic<bool> GLogiKDaemon::daemonized_;

GLogiKDaemon::GLogiKDaemon() :	pid_(0), log_fd_(NULL), pid_file_name_(""), buffer_("", std::ios_base::app)
{
	openlog(GLOGIKD_DAEMON_NAME, LOG_PID|LOG_CONS, LOG_DAEMON);
	FILELog::ReportingLevel() = FILELog::FromString(DEBUG_LOG_LEVEL);

	if( FILELog::ReportingLevel() != NONE ) {
		std::stringstream log_file;
		log_file << DEBUG_DIR << "/glogikd-debug-" << getpid() << ".log";
		this->log_fd_ = std::fopen( log_file.str().c_str(), "w" );
	}
	if( this->log_fd_ == NULL )
		syslog(LOG_INFO, "debug file not opened");
		
	LOG2FILE::Stream() = this->log_fd_;
}

GLogiKDaemon::~GLogiKDaemon()
{
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
	if( this->log_fd_ != NULL )
		std::fclose(this->log_fd_);
	closelog();
}

int GLogiKDaemon::run( const int& argc, char *argv[] ) {

	const char * msg = "starting ...";
	syslog(LOG_INFO, msg);
	LOG(INFO) << msg;

	try {
		this->parseCommandLine(argc, argv);

		DevicesManager d;

		if( GLogiKDaemon::is_daemon_enabled() ) {
			this->daemonize();

			std::signal(SIGINT, GLogiKDaemon::handle_signal);
			std::signal(SIGHUP, GLogiKDaemon::handle_signal);

			d.monitor();
		}
		else {
			LOG(INFO) << "non-daemon mode";
			;
		}

		return EXIT_SUCCESS;
	}
	catch ( const GLogiKExcept & e ) {
		this->buffer_.str( e.what() );
		if(errno != 0)
			this->buffer_ << " : " << strerror(errno);
		const char * msg = this->buffer_.str().c_str();
		syslog( LOG_ERR, msg );
		LOG(ERROR) << msg;
		return EXIT_FAILURE;
	}

}

void GLogiKDaemon::disable_daemon( void ) {
	GLogiKDaemon::daemonized_ = false;
}

bool GLogiKDaemon::is_daemon_enabled() {
	return GLogiKDaemon::daemonized_;
}

void GLogiKDaemon::handle_signal(int sig) {
	LOG(DEBUG2) << "starting handle_signal()";
	switch( sig ) {
		case SIGINT: {
			const char * msg = "got SIGINT, exiting ...";
			syslog(LOG_INFO, msg);
			LOG(INFO) << msg;
			GLogiKDaemon::disable_daemon();
			std::signal(SIGINT, SIG_DFL);
		}
	}
}

void GLogiKDaemon::daemonize() {
	//int fd = 0;

	LOG(DEBUG2) << "starting GLogiKDaemon::daemonize()";

	this->pid_ = fork();
	if(this->pid_ == -1)
		throw GLogiKExcept("first fork failure");

	// parent exit
	if(this->pid_ > 0)
		exit(EXIT_SUCCESS);

	LOG(DEBUG3) << "First fork ! pid:" << getpid();

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
	
	LOG(DEBUG3) << "Second fork ! pid:" << getpid();

	umask(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if(chdir("/") == -1)
		throw GLogiKExcept("change directory failure");

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
	LOG(DEBUG2) << "starting GLogiKDaemon::parse_command_line()";

	bool d = false;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("daemonize,d", po::bool_switch(&d)->default_value(false))
		("pid-file,p", po::value(&this->pid_file_name_), "PID file")
	;

	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);
	}
	catch( std::exception & e ) {
		throw GLogiKExcept( e.what() );
	}
	po::notify(vm);

	if (vm.count("daemonize")) {
		GLogiKDaemon::daemonized_ = vm["daemonize"].as<bool>();
	}
}

} // namespace GLogiKd

