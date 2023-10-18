/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <cstring>

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

const pid_t process::newPID(void)
{
	GK_LOG_FUNC

#if DEBUGGING_ON
	if(process::options & process::mask::PROCESS_LOG_ENTRIES) {
		GKLog(trace, "detaching process")
	}
#endif

	pid_t pid;

	pid = fork();
	if(pid == -1)
		throw GLogiKExcept("first fork failure");

	// parent exit
	if(pid > 0) {
#if DEBUGGING_ON
		if(process::options & process::mask::PROCESS_LOG_ENTRIES) {
			GKLog2(trace, "exiting parent. first fork done. pid : ", pid)
		}
#endif
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		exit(EXIT_SUCCESS);
	}

#if DEBUGGING_ON
	if(process::options & process::mask::PROCESS_LOG_ENTRIES) {
		GKLog(trace, "continue child execution")
	}
#endif

	// new session for child process
	if(setsid() == -1)
		throw GLogiKExcept("session creation failure");

#if DEBUGGING_ON
	if(process::options & process::mask::PROCESS_LOG_ENTRIES) {
		GKLog(trace, "new session done")
	}
#endif

	// Ignore signals
	std::signal(SIGCHLD, SIG_IGN);
	std::signal(SIGHUP, SIG_IGN);

	pid = fork();
	if(pid == -1)
		throw GLogiKExcept("second fork failure");

	// parent exit
	if(pid > 0) {
#if DEBUGGING_ON
		if(process::options & process::mask::PROCESS_LOG_ENTRIES) {
			GKLog2(trace, "exiting parent. second fork done. pid : ", pid)
		}
#endif
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		exit(EXIT_SUCCESS);
	}

#if DEBUGGING_ON
	if(process::options & process::mask::PROCESS_LOG_ENTRIES) {
		GKLog(trace, "continue child execution")
	}
#endif

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

	pid = getpid();

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

	const std::string sigdesc(toString(sigabbrev_np(signum)));
	if(sigdesc.empty())
		throw GLogiKExcept("invalid signal number");
	GKLog2(trace, "setting signal handler: ", sigdesc)
	if(std::signal(signum, __handler) == SIG_ERR) {
		GKSysLogError("std::signal failure: ", sigdesc);
	}
}

void process::resetSignalHandler(int signum)
{
	GK_LOG_FUNC

	try {
		process::setSignalHandler(signum, SIG_DFL);
	}
	catch (const GLogiKExcept & e) {
		GKSysLogWarning(e.what());
	}
}

} // namespace NSGKUtils

