/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2024  Fabrice Delliaux <netbox253@gmail.com>
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

#include <chrono>
#include <thread>
#include <string>
#include <sstream>

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <cstring>
#include <cerrno>

#include <config.h>

#define UTILS_COMPILATION 1

#include "GKLogging.hpp"
#include "exception.hpp"
#include "functions.hpp"
#include "process.hpp"

#undef UTILS_COMPILATION

namespace NSGKUtils
{

uint8_t process::options = 0;

const pid_t process::detach(void)
{
	process::options |= process::mask::PROCESS_LOG_ENTRIES;
	return process::newPID();
}

const pid_t process::deamonize(void)
{	/* main process is daemonized before opening debug file --> no log entries */
	process::options |= process::mask::PROCESS_CLOSE_DESCRIPTORS;
	return process::newPID();
}

void process::logErrno(const int errnum, const std::string & errstr)
{
	LOG(error)	<< errstr << " : "
				<< strerrorname_np(errnum) << " - "
				<< strerrordesc_np(errnum);
}

void process::closeFD(int fd)
{
	if(close(fd) == -1)
		process::logErrno(errno, "fd close");
}

void process::notifyParentProcess(int pipefd[], const int message)
{
	GK_LOG_FUNC

	process::closeFD(pipefd[0]); /* close unused read-end */

#if DEBUGGING_ON
	if(process::options & process::mask::PROCESS_LOG_ENTRIES) {
		GKLog(trace, "closed unused read pipe file descriptor before write")
	}
#endif

	const char byte = static_cast<const char>(message);
	const ssize_t bytes_written = write(pipefd[1], &byte, 1);

	if(bytes_written == -1) {
		process::logErrno(errno, "write");
		throw GLogiKExcept("error while trying to write pipe");
	}
	else if(bytes_written < 1) {
		throw GLogiKExcept("byte not written");
	}

	if(process::options & process::mask::PROCESS_LOG_ENTRIES) {
		GKLog2(trace, "byte(s) written to pipe: ", std::to_string(bytes_written))
	}
}

const int process::waitForChildNotification(int pipefd[])
{
	GK_LOG_FUNC

	process::closeFD(pipefd[1]); /* close unused write-end */

#if DEBUGGING_ON
	if(process::options & process::mask::PROCESS_LOG_ENTRIES) {
		GKLog(trace, "closed unused write pipe file descriptor before read")
	}
#endif

	char buf = -1;
	const ssize_t bytes_read = read(pipefd[0], &buf, 1);

	if(bytes_read == -1) {
		process::logErrno(errno, "read");
		throw GLogiKExcept("error while trying to read pipe");
	}

	if(bytes_read == 0)
		throw GLogiKExcept("end of file reached while trying to read pipe");

	// received one byte
	return static_cast<const int>(buf);
}

void process::forkProcess(void)
{
	GK_LOG_FUNC

	int pipefd[2];
	pid_t pid;

	/* create data channel before forking */
	if(pipe(pipefd) == -1)
		throw GLogiKExcept("failed to create pipe");

	pid = fork();
	if(pid == -1) {
		throw GLogiKExcept("fork failure");
	}
	else if(pid > 0) { /* parent process */
		if(process::waitForChildNotification(pipefd) != EXIT_SUCCESS)
			throw GLogiKExcept("parent process wrong return value");

#if DEBUGGING_ON
		if(process::options & process::mask::PROCESS_LOG_ENTRIES) {
			GKLog2(trace, "exiting parent. first fork done. pid : ", pid)
		}
#endif
		std::exit(EXIT_SUCCESS);
	}
	else { /* child process */
		process::notifyParentProcess(pipefd, EXIT_SUCCESS);

#if DEBUGGING_ON
		if(process::options & process::mask::PROCESS_LOG_ENTRIES) {
			GKLog(trace, "continue child execution")
		}
#endif
	}
}

const pid_t process::newPID(void)
{
	GK_LOG_FUNC

#if DEBUGGING_ON
	if(process::options & process::mask::PROCESS_LOG_ENTRIES) {
		GKLog(trace, "detaching process")
	}
#endif

	// ignore signals
	process::setSignalHandler(SIGCHLD, SIG_IGN);
	process::setSignalHandler(SIGHUP, SIG_IGN);

	process::forkProcess();

	// new session for child process
	if(setsid() == -1)
		throw GLogiKExcept("session creation failure");

#if DEBUGGING_ON
	if(process::options & process::mask::PROCESS_LOG_ENTRIES) {
		GKLog(trace, "new session done")
	}
#endif

	process::forkProcess();

	umask(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if(chdir("/") == -1)
		throw GLogiKExcept("change directory failure");

	if(process::options & process::mask::PROCESS_CLOSE_DESCRIPTORS) {
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

#if DEBUGGING_ON
		if(process::options & process::mask::PROCESS_LOG_ENTRIES) {
			GKLog(trace, "descriptors closed, process daemonized")
		}
#endif
	}

	pid_t pid = getpid();

#if DEBUGGING_ON
		if(process::options & process::mask::PROCESS_LOG_ENTRIES) {
			GKLog2(trace, "returning pid : ", pid)
		}
#endif

	return pid;
}

void process::setSignalHandler(int signum, __signal_handler_t __handler)
{
	GK_LOG_FUNC

#if DEBUGGING_ON
	if(process::options & process::mask::PROCESS_LOG_ENTRIES) {
		const std::string sigdesc( process::getSignalAbbrev(signum) );

		if(__handler == SIG_DFL) {
			GKLog2(trace, "resetting signal handler: ", sigdesc)
		}
		else if(__handler == SIG_IGN) {
			GKLog2(trace, "ignoring signal: ", sigdesc)
		}
		else {
			GKLog2(trace, "setting signal handler: ", sigdesc)
		}
	}
#endif

	struct sigaction new_action;

	new_action.sa_handler = __handler;
	if(sigemptyset(&new_action.sa_mask) == -1) {
		process::logErrno(errno, "sigemptyset");
	}
	else {
		new_action.sa_flags = 0;

		if(sigaction(signum, &new_action, nullptr) == -1) {
			process::logErrno(errno, "sigaction");
		}
	}
}

void process::resetSignalHandler(int signum)
{
	process::setSignalHandler(signum, SIG_DFL);
}

const std::string process::getSignalHandlingDesc(const int & signum, const std::string & desc)
{
	const std::string sigdesc( process::getSignalAbbrev(signum) );
	std::ostringstream buffer("caught signal: ", std::ios_base::app);
	buffer << sigdesc << "(" << signum << ")" << desc;
	return buffer.str();
}

const std::string process::getSignalAbbrev(int signum)
{
	GK_LOG_FUNC

	std::string sigdesc("");
	try {
		sigdesc = toString(sigabbrev_np(signum));
		if(sigdesc.empty())
			sigdesc = "invalid signal number";
	}
	catch (const GLogiKExcept & e) {
		std::string warn("string conversion exception: ");
		warn += e.what();
		GKSysLogWarning(warn);
		sigdesc = warn;
	}
	return sigdesc;
}

} // namespace NSGKUtils

