/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
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

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include <cstdio>
#include <cstdlib>
#include <csignal>

#include <new>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <thread>
#include <limits>

#include <config.h>

#include "GKLogging.hpp"

#define UTILS_COMPILATION 1

#include "exception.hpp"
#include "functions.hpp"

#undef UTILS_COMPILATION

namespace NSGKUtils
{

const std::string toString(const char* s)
{
	if(s == nullptr)
		return "";

	try {
		const std::string ret(s);
		return ret;
	}
	catch (const std::bad_alloc& e) {
		throw GLogiKBadAlloc( e.what() );
	}
	catch (const std::length_error& e) {
		std::string error("string length error : ");
		error += e.what();
		throw GLogiKExcept(error);
	}
}

const std::wstring toWString(const wchar_t* s)
{
	if(s == nullptr)
		return std::wstring();

	try {
		const std::wstring ret(s);
		return ret;
	}
	catch (const std::bad_alloc& e) {
		throw GLogiKBadAlloc( e.what() );
	}
	catch (const std::length_error& e) {
		std::string error("wstring length error : ");
		error += e.what();
		throw GLogiKExcept(error);
	}
}

const unsigned int toUInt(const std::string & s)
{
	unsigned long ret = toUL(s);

	if(ret > std::numeric_limits<unsigned int>::max() )
		throw GLogiKExcept("UINT_MAX out of range");

	return static_cast<unsigned int>(ret);
}

const unsigned short toUShort(const std::string & s, int base)
{
	unsigned long ret = toUL(s, base);

	if(ret > std::numeric_limits<unsigned short>::max() )
		throw GLogiKExcept("USHRT_MAX out of range");

	return static_cast<unsigned short>(ret);
}

const unsigned long toUL(const std::string & s, int base)
{
	unsigned long ret = 0;
	try {
		ret = std::stoul(s, nullptr, base);
	}
	catch (const std::invalid_argument& ia) {
		throw GLogiKExcept("stoul invalid argument");
	}
	catch (const std::out_of_range& oor) {
		throw GLogiKExcept("stoul out of range");
	}

	return ret;
}

const unsigned long long toULL(const std::string & s)
{
	unsigned long long ret = 0;
	try {
		ret = std::stoull(s);
	}
	catch (const std::invalid_argument& ia) {
		throw GLogiKExcept("stoull invalid argument");
	}
	catch (const std::out_of_range& oor) {
		throw GLogiKExcept("stoull out of range");
	}

	return ret;
}

const std::string getHexRGB(
	const uint8_t red,
	const uint8_t green,
	const uint8_t blue)
{
	std::ostringstream ret("", std::ios_base::app);
	ret << std::hex << std::setfill('0')
		<< std::setw(2) << toUInt(red) << " "
		<< std::setw(2) << toUInt(green) << " "
		<< std::setw(2) << toUInt(blue);
	return ret.str();
}

void yield_for(std::chrono::microseconds us)
{
	auto start = std::chrono::high_resolution_clock::now();
	auto end = start + us;
	do {
		std::this_thread::yield();
	} while (std::chrono::high_resolution_clock::now() < end);
}

pid_t detachProcess(const bool closeDescriptors)
{
	GK_LOG_FUNC

#if DEBUGGING_ON
	LOG(DEBUG) << "detaching process";
#endif
	pid_t pid;

	pid = fork();
	if(pid == -1)
		throw GLogiKExcept("first fork failure");

	// parent exit
	if(pid > 0) {
#if DEBUGGING_ON
		LOG(DEBUG1) << "first fork done. pid: " << pid;
#endif
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
#if DEBUGGING_ON
		LOG(DEBUG1) << "exiting parent";
#endif
		exit(EXIT_SUCCESS);
	}
	else {
#if DEBUGGING_ON
		LOG(DEBUG2) << "continue child execution";
#endif
	}

	// new session for child process
	if(setsid() == -1)
		throw GLogiKExcept("session creation failure");

#if DEBUGGING_ON
	LOG(DEBUG) << "new session done";
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
		LOG(DEBUG1) << "second fork done. pid: " << pid;
#endif
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
#if DEBUGGING_ON
		LOG(DEBUG1) << "exiting parent";
#endif
		exit(EXIT_SUCCESS);
	}
	else {
#if DEBUGGING_ON
		LOG(DEBUG2) << "continue child execution";
#endif
	}

	umask(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if(chdir("/") == -1)
		throw GLogiKExcept("change directory failure");

	if( closeDescriptors ) {
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
		LOG(DEBUG) << "descriptors closed, process daemonized";
#endif
	}

	pid = getpid();

#if DEBUGGING_ON
	LOG(DEBUG) << "returning pid: " << pid;
#endif

	return pid;
}

} // namespace NSGKUtils

