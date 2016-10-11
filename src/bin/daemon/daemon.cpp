
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

namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace GLogiK
{

bool GLogiKDaemon::daemon = false;

GLogiKDaemon::GLogiKDaemon() :	pid(0), log_fd(NULL), pid_file_name(""), buffer("", std::ios_base::app)
{
	openlog(GLOGIK_DAEMON_NAME, LOG_PID|LOG_CONS, LOG_DAEMON);
	FILELog::ReportingLevel() = FILELog::FromString(DEBUG_LOG_LEVEL);

	if( FILELog::ReportingLevel() != NONE ) {
		std::stringstream log_file;
		log_file << DEBUG_DIR << "/glogikd-debug-" << getpid() << ".log";
		this->log_fd = std::fopen( log_file.str().c_str(), "w" );
	}
	if( this->log_fd == NULL )
		syslog(LOG_INFO, "debug file not opened");
		
	LOG2FILE::Stream() = this->log_fd;
}

GLogiKDaemon::~GLogiKDaemon()
{
	if( this->pid_file.is_open() ) {
		this->pid_file.close();
		if( unlink(this->pid_file_name.c_str()) != 0 ) {
			const char * msg = "failed to unlink PID file";
			syslog(LOG_ERR, msg);
			LOG(ERROR) << msg;
		}
	}

	if( this->log_fd != NULL )
		std::fclose(this->log_fd);
	closelog();
}

int GLogiKDaemon::run( const int& argc, char *argv[] ) {

	syslog(LOG_INFO, "starting");
	LOG(INFO) << "starting";

	try {
		this->parse_command_line(argc, argv);

		if( this->daemon ) {
			this->daemonize();

			syslog(LOG_INFO, "living in %ld", (long)this->pid);
			LOG(INFO) << "living in " << (long)this->pid;

			std::signal(SIGINT, this->handle_signal);
			std::signal(SIGHUP, this->handle_signal);

			while( this->daemon ) {
				sleep(1);
			}

		}
		else {
			LOG(INFO) << "non-daemon mode";
			;
		}

		return EXIT_SUCCESS;
	}
	catch ( const GLogiKExcept & e ) {
		this->buffer.str( e.what() );
		if(errno != 0)
			this->buffer << " : " << strerror(errno);
		const char * msg = this->buffer.str().c_str();
		syslog( LOG_ERR, msg );
		LOG(ERROR) << msg;
		return EXIT_FAILURE;
	}

}

void GLogiKDaemon::handle_signal(int sig) {
	LOG(DEBUG2) << "starting handle_signal()";
	switch( sig ) {
		case SIGINT: {
			const char * msg = "got SIGINT, exiting ...";
			syslog(LOG_INFO, msg);
			LOG(INFO) << msg;
			GLogiKDaemon::daemon = false;
			std::signal(SIGINT, SIG_DFL);
		}
	}
}

void GLogiKDaemon::daemonize() {
	//int fd = 0;

	LOG(DEBUG2) << "starting GLogiKDaemon::daemonize()";

	this->pid = fork();
	if(this->pid == -1)
		throw GLogiKExcept("first fork failure");

	// parent exit
	if(this->pid > 0)
		exit(EXIT_SUCCESS);

	LOG(DEBUG3) << "First fork ! pid:" << getpid();

	if(setsid() == -1)
		throw GLogiKExcept("session creation failure");

	// Ignore signal sent from child to parent process
	std::signal(SIGCHLD, SIG_IGN);

	this->pid = fork();
	if(this->pid == -1)
		throw GLogiKExcept("second fork failure");

	// parent exit
	if(this->pid > 0)
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

	this->pid = getpid();
	LOG(INFO) << "daemonized !";

	fs::path path(this->pid_file_name);

	if( fs::exists(path) ) {
		this->buffer.str( "PID file " );
		this->buffer << this->pid_file_name << " already exist";
		throw GLogiKExcept( this->buffer.str() );
	}

	this->pid_file.exceptions( std::ofstream::failbit );
	try {
		this->pid_file.open(this->pid_file_name.c_str(), std::ofstream::trunc);
		this->pid_file << (long)this->pid;
		this->pid_file.flush();

		fs::permissions(path, fs::owner_read|fs::owner_write|fs::group_read|fs::others_read);
	}
	catch (const std::ofstream::failure & e) {
		this->buffer.str( "Fail to open PID file : " );
		this->buffer << this->pid_file_name << " : " << e.what();
		throw GLogiKExcept( this->buffer.str() );
	}
	catch (const fs::filesystem_error & e) {
		this->buffer.str( "Set permissions failure on PID file : " );
		this->buffer << this->pid_file_name << " : " << e.what();
		throw GLogiKExcept( this->buffer.str() );
	}
	LOG(INFO) << "created PID file : " << this->pid_file_name;
}

void GLogiKDaemon::parse_command_line(const int& argc, char *argv[]) {
	LOG(DEBUG2) << "starting GLogiKDaemon::parse_command_line()";

	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("daemonize,d", po::bool_switch(&this->daemon)->default_value(false))
		("pid-file,p", po::value(&this->pid_file_name), "PID file")
	;

	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);
	}
	catch( std::exception & e ) {
		throw GLogiKExcept( e.what() );
	}
	po::notify(vm);
}

} // namespace GLogiK

