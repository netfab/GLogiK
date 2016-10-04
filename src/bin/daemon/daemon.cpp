
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>

#include <string>
#include <sstream>

#include <config.h>

#include "exception.h"
#include "daemon.h"
#include "globals.h"
#include "include/log.h"

namespace GLogiK
{

GLogiKDaemon::GLogiKDaemon() : pid(0), log_fd(NULL)
{
	openlog(GLOGIK_DAEMON_NAME, LOG_PID|LOG_CONS, LOG_DAEMON);
	FILELog::ReportingLevel() = FILELog::FromString(DEBUG_LOG_LEVEL);

	if( FILELog::ReportingLevel() != NONE ) {
		std::stringstream log_file;
		log_file << "/tmp/glogikd-debug-" << getpid() << ".log";
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

	try {
		this->daemonize();
		syslog(LOG_INFO, "living in %ld", (long)this->pid);
		return EXIT_SUCCESS;
	}
	catch ( const GLogiKExcept & e ) {
		std::string msg = e.what();
		msg += " : ";
		msg += strerror(errno);
		syslog( LOG_ERR, msg.c_str() );
		LOG(ERROR) << msg.c_str();
		return EXIT_FAILURE;
	}

}

void GLogiKDaemon::daemonize() {
	int fd = 0;

	LOG(DEBUG2) << "starting GLogiKDaemon::daemonize()";

	this->pid = fork();
	if(this->pid == -1)
		throw GLogiKExcept("first fork failure");

	// parent exit
	if(this->pid > 0)
		exit(EXIT_SUCCESS);

	LOG(DEBUG4) << "First fork ! pid:" << getpid();

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
	
	LOG(DEBUG4) << "Second fork ! pid:" << getpid();

	umask(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if(chdir("/") == -1)
		throw GLogiKExcept("change directory failure");

	// closing opened descriptors
	for(fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--)
		close(fd);
	
	// reopening standard outputs
	stdin = fopen("/dev/null", "r");
	stdout = fopen("/dev/null", "w+");
	stderr = fopen("/dev/null", "w+");

	this->pid = getpid();
}


} // namespace GLogiK

