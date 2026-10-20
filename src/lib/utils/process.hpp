/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2026  Fabrice Delliaux <netbox253@gmail.com>
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

#pragma once

#if !defined (UTILS_INSIDE_UTILS_H) && !defined (UTILS_COMPILATION)
#error "Only "utils/utils.hpp" can be included directly, this file may disappear or change contents."
#endif

#include <cstddef>

#include <array>
#include <chrono>
#include <map>
#include <string>
#include <vector>

#include <cstdint>
#include <sys/types.h>

#include "templates.hpp"

namespace NSGKUtils
{

class process
{
	private:
		using __signal_handler_t = void (*)(int);

	public:
		enum class mask : std::uint8_t
		{
			PROCESS_LOG_ENTRIES = 1 << 0,
			PROCESS_CLOSE_DESCRIPTORS = 1 << 1,
		};

		static pid_t detach();
		static pid_t deamonize();
		static void setSignalHandler(
			int signum,
			__signal_handler_t __handler
		);
		static void resetSignalHandler(int signum);
		static std::string getSignalHandlingDesc(
			int signum,
			const std::string & desc
		);

		static void runCommand(
			const std::string & binary,
			const std::vector<std::string> & args
		);
		static std::string runCommandAndGetOutput(
			const std::string & binary,
			const std::vector<std::string> & args
		);
		static void runDelayedCommand(
			const std::string & binary,
			const std::vector<std::string> & args,
			std::uint16_t delay
		);

	protected:

	private:
		process() = delete;
		~process() = delete;

		static std::map<const std::string, std::chrono::steady_clock::time_point> lastCalls;
		static const std::chrono::steady_clock::duration threeSeconds;
		static const std::chrono::steady_clock::duration oneSecond;

		static std::uint8_t options;

		static void logErrno(
			int errnum,
			const std::string & errstr
		);
		static void closeFD(
			int fd,
			const std::string & tracestr
		);
		static void newSessionID();

		static void forkProcess(bool newSessionID=false);

		static pid_t newPID();
		static std::string getSignalAbbrev(int signum);

		static void notifyParentProcess_impl(int read_end, int write_end, int message);
		static int waitForChildNotification_impl(int read_end, int write_end);

		template<std::size_t I, typename T, std::size_t N>
		static constexpr T getFD(const std::array<T, N> & arr)
		{
			static_assert(I < N, "out-of-bounds access");
			return arr[I];
		}

		using pipeArray = std::array<int, 2>;

		static void notifyParentProcess(const pipeArray & pipefd, int message)
		{
			const int  read_end = process::getFD<0>(pipefd);
			const int write_end = process::getFD<1>(pipefd);
			process::notifyParentProcess_impl(read_end, write_end, message);
		}

		static int waitForChildNotification(const pipeArray & pipefd)
		{
			const int  read_end = process::getFD<0>(pipefd);
			const int write_end = process::getFD<1>(pipefd);
			return process::waitForChildNotification_impl(read_end, write_end);
		}
};

inline const std::uint8_t operator & (
	std::uint8_t value,
	process::mask option)
{
	std::uint8_t ret(value);
	ret &= toEnumType(option);
	return ret;
}

inline std::uint8_t & operator |= (
	std::uint8_t & value,
	process::mask option)
{
	return value=(value|toEnumType(option));
}

} // namespace NSGKUtils
