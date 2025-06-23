/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2025  Fabrice Delliaux <netbox253@gmail.com>
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

#include <boost/asio.hpp>
#include <boost/process.hpp>

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
	LOG(error)	<< errstr << " : " << getErrnoString(errnum);
}

void process::closeFD(int fd, const std::string & tracestr)
{
	const int ret = close(fd);
	const int close_errno = errno;

	if(ret == -1)
		process::logErrno(close_errno, "fd close");
#if DEBUGGING_ON
	else if(ret == 0) {
		if(process::options & process::mask::PROCESS_LOG_ENTRIES) {
			LOG(trace) << "closed fd: " << tracestr;
		}
	}
#endif
}

void process::notifyParentProcess(int pipefd[], const int message)
{
	GK_LOG_FUNC

	/* close unused read-end */
	process::closeFD(pipefd[0], "unused read-end");

	const char byte = static_cast<const char>(message);
	const ssize_t bytes_written = write(pipefd[1], &byte, 1);
	const int write_errno = errno;

	process::closeFD(pipefd[1], "remaining write-end");

	if(bytes_written == -1) {
		process::logErrno(write_errno, "write");
		throw GLogiKExcept("error while trying to write pipe");
	}
	else if(bytes_written < 1)
		throw GLogiKExcept("byte not written");

	if(process::options & process::mask::PROCESS_LOG_ENTRIES) {
		GKLog2(trace, "byte(s) written to pipe: ", std::to_string(bytes_written))
	}
}

const int process::waitForChildNotification(int pipefd[])
{
	GK_LOG_FUNC

	/* close unused write-end */
	process::closeFD(pipefd[1], "unused write-end");

	char buf = -1;
	const ssize_t bytes_read = read(pipefd[0], &buf, 1);
	const int read_errno = errno;

	process::closeFD(pipefd[0], "remaining read-end");

	if(bytes_read == -1) {
		process::logErrno(read_errno, "read");
		throw GLogiKExcept("error while trying to read pipe");
	}
	else if(bytes_read == 0)
		throw GLogiKExcept("end of file reached while trying to read pipe");

	// received one byte
	return static_cast<const int>(buf);
}

void process::newSessionID(void)
{
	GK_LOG_FUNC

	// new session for child process
	if(setsid() == -1)
		throw GLogiKExcept("session creation failure");

#if DEBUGGING_ON
	if(process::options & process::mask::PROCESS_LOG_ENTRIES) {
		GKLog(trace, "new session done")
	}
#endif
}

void process::forkProcess(const bool newSessionID)
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
		if(newSessionID) {
			/* detach from parent terminal by creating new session ID
			 * before notifying parent process */
			process::newSessionID();
		}

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
	auto clearSignalMask = [] () -> void
	{
		sigset_t new_set;
		if(sigemptyset(&new_set) == -1) {
			process::logErrno(errno, "sigemptyset");
		}
		else {
			if(sigprocmask(SIG_SETMASK, &new_set, NULL) == -1)
				process::logErrno(errno, "sigprocmask");
#if DEBUGGING_ON
			else {
				if(process::options & process::mask::PROCESS_LOG_ENTRIES) {
					GKLog(trace, "signal mask cleared")
				}
			}
#endif
		}
	};

	clearSignalMask();

	/* ignore signals */
	process::setSignalHandler(SIGCHLD, SIG_IGN);
	process::setSignalHandler(SIGHUP, SIG_IGN);

	/* detach child from parent terminal on first fork */
	process::forkProcess(true);

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

void process::runCommand(const std::string & binary, const std::vector<std::string> & args)
{
	GK_LOG_FUNC

	namespace bp = boost::process;
	namespace io = boost::asio;

	auto search = bp::v2::environment::find_executable(binary);
	if( search.empty() )
	{
		LOG(error) << binary << " executable not found in PATH";
		return;
	}
	const std::string command_bin( search.string() );

	{
		std::string whole_cmd(command_bin);
		for(const auto & arg : args)
		{
			whole_cmd += " ";
			whole_cmd += arg;
		}
		GKLog2(info, "spawning: ", whole_cmd)
	}

	io::io_context ctx;

	bp::v2::process proc(ctx, command_bin, args);
	proc.detach();
}

const std::string process::runCommandAndGetOutput(const std::string & binary, const std::vector<std::string> & args)
{
	GK_LOG_FUNC

	namespace bp = boost::process;
	namespace io = boost::asio;

	std::string output;
	boost::system::error_code ec;
	io::io_context ctx;
	io::readable_pipe rp{ctx};

	auto search = bp::v2::environment::find_executable(binary);
	if( search.empty() )
		throw GLogiKExcept("searched binary not found in PATH");
	const std::string command_bin( search.string() );

	{
		std::string whole_cmd(command_bin);
		for(const auto & arg : args)
		{
			whole_cmd += " ";
			whole_cmd += arg;
		}
		GKLog2(info, "spawning: ", whole_cmd)
	}

	bp::v2::process proc(
		ctx,
		command_bin,
		args,
		bp::v2::process_stdio
		{
			.in = {}, /* in to default */
			.out = rp,
			.err = {} /* err to default */
		}
	);

	[[maybe_unused]] std::size_t num = io::read(rp, io::dynamic_buffer(output), ec);
	GKLog2(trace, "size read: ", num)

	if(ec == io::error::eof)
	{
		GKLog(trace, "reached eof, connection closed cleanly while reading pipe")
	}
	else if( ! ec )
		throw GLogiKExcept(ec.message());
	else
	{
		LOG(warning) << "waiting for process";
		proc.wait();
	}

	return output;
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

