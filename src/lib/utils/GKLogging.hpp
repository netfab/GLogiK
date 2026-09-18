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

#ifndef SRC_LIB_UTILS_GKLOGGING_HPP_
#define SRC_LIB_UTILS_GKLOGGING_HPP_

#if !defined (UTILS_INSIDE_UTILS_H) && !defined (UTILS_COMPILATION)
#error "Only "utils/utils.hpp" can be included directly, this file may disappear or change contents."
#endif

#include "config.h"

#include <ios>
#include <string_view>
#include <ostream>
#include <sstream>

#include <syslog.h>

#include <boost/filesystem.hpp>
#include <boost/log/attributes/named_scope.hpp>

namespace fs = boost::filesystem;

namespace NSGKUtils
{

enum severity_level
{
	trace,
	info,
	warning,
	error,
	critical
};

class LogStream
{
	public:
		explicit LogStream(severity_level severity);
		~LogStream();

		LogStream(const LogStream&) = delete;
		LogStream& operator=(const LogStream&) = delete;

		LogStream(LogStream&&) = default;
		LogStream& operator=(LogStream&&) = default;

		template<typename T>
			LogStream& operator<<(const T& value)
		{
			this->_stream << value;
			return *this;
		}

		// << operator for manipulators
		LogStream& operator<<(std::ostream & (*manip)(std::ostream&));

	private:
		severity_level _severity;
		std::ostringstream _stream;
};

class GKLogging
{
	public:
		GKLogging(void) = delete;
		~GKLogging(void) = delete;

		static bool GKDebug;
		static bool GKVerbose;

		static void initConsoleLog(std::string_view baseName);
		static void initDebugFile(
			std::string_view baseName,
			const fs::perms prms = fs::no_perms);

		static LogStream log(severity_level severity);

	protected:

	private:
		static bool initialized;
		static void init(void);
};

#define LOG(sev) 	GKLogging::log(sev)

#if DEBUGGING_ON
#define GKLog(level, m1) if(GKLogging::GKDebug) { LOG(level) << m1; }
#define GKLog2(level, m1, m2) if(GKLogging::GKDebug) { LOG(level) << m1 << m2; }
#define GKLog3(level, m1, m2, m3) if(GKLogging::GKDebug) { LOG(level) << m1 << m2 << m3; }
#define GKLog4(level, m1, m2, m3, m4) if(GKLogging::GKDebug) { LOG(level) << m1 << m2 << " - " << m3 << m4; }
#define GKLog6(level, m1, m2, m3, m4, m5, m6) \
	if(GKLogging::GKDebug) { LOG(level) << m1 << m2 << " - " << m3 << m4 << " - " << m5 << m6;	}
#else
#define GKLog(level, m1)
#define GKLog2(level, m1, m2)
#define GKLog3(level, m1, m2, m3)
#define GKLog4(level, m1, m2, m3, m4)
#define GKLog6(level, m1, m2, m3, m4, m5, m6)
#endif

inline void GKSysLog(
	const int priority,
	const severity_level level,
	std::string_view msg)
{
	GKLog(level, msg);
	syslog(priority, "%.*s", static_cast<int>(msg.size()), msg.data());
}

inline void GKSysLogInfo(std::string_view msg)
{
	GKSysLog(LOG_INFO, info, msg);
}

inline void GKSysLogWarning(std::string_view msg)
{
	GKSysLog(LOG_WARNING, warning, msg);
}

inline void GKSysLogError(std::string_view msg)
{
	GKSysLog(LOG_ERR, error, msg);
}

inline void GKSysLogWarning(
	std::string_view msg1,
	std::string_view msg2)
{
	GKLog2(warning, msg1, msg2);
	syslog(LOG_WARNING, "%.*s%.*s",
		static_cast<int>(msg1.size()), msg1.data(),
		static_cast<int>(msg2.size()), msg2.data()
	);
}

inline void GKSysLogError(
	std::string_view msg1,
	std::string_view msg2)
{
	GKLog2(error, msg1, msg2);
	syslog(LOG_ERR, "%.*s%.*s",
		static_cast<int>(msg1.size()), msg1.data(),
		static_cast<int>(msg2.size()), msg2.data()
	);
}


#if DEBUGGING_ON
#define GK_LOG_FUNC BOOST_LOG_FUNC()
#else
#define GK_LOG_FUNC
#endif

} // namespace NSGKUtils

#endif
