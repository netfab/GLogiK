
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

#include "exception.h"
#include "daemon.h"
#include "globals.h"
#include "include/log.h"

namespace po = boost::program_options;

namespace GLogiK
{

GLogiKDaemon::GLogiKDaemon() : pid(0), log_fd(NULL), pid_file_name(""), buffer("", std::ios_base::app)
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
	if( this->log_fd != NULL )
		fclose(this->log_fd);
	closelog();
}

int GLogiKDaemon::run( const int& argc, char *argv[] ) {

	syslog(LOG_INFO, "starting");
	LOG(INFO) << "starting";

	try {
		this->parse_command_line(argc, argv);

		this->daemonize();

		this->pid_file.exceptions( std::ofstream::failbit );
		try {
			this->pid_file.open(this->pid_file_name.c_str(), std::ofstream::trunc);
		}
		catch (const std::ofstream::failure & e) {
			this->buffer.str( "Fail to open PID file : " );
			this->buffer << this->pid_file_name << " : " << e.what();
			throw GLogiKExcept( this->buffer.str() );
		}
		this->pid_file << (long)this->pid;

		syslog(LOG_INFO, "living in %ld", (long)this->pid);
		LOG(INFO) << "living in " << (long)this->pid;
		return EXIT_SUCCESS;
	}
	catch ( const GLogiKExcept & e ) {
		this->buffer.str( e.what() );
		if(errno != 0)
			this->buffer << " : " << strerror(errno);
		syslog( LOG_ERR, this->buffer.str().c_str() );
		LOG(ERROR) << this->buffer.str();
		return EXIT_FAILURE;
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
	//signal(SIGCHLD, SIG_IGN);

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
}

void GLogiKDaemon::parse_command_line(const int& argc, char *argv[]) {
	LOG(DEBUG2) << "starting GLogiKDaemon::parse_command_line()";

	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
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

