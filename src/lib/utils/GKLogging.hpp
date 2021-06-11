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

#include <ios>
#include <string>
#include <sstream>

#include <syslog.h>

#include <boost/filesystem.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include "lib/shared/glogik.hpp"

#include <config.h>

#ifndef SRC_LIB_UTILS_GKLOGGING_HPP_
#define SRC_LIB_UTILS_GKLOGGING_HPP_

namespace fs = boost::filesystem;
namespace src = boost::log::sources;

namespace NSGKUtils
{

enum severity_level
{
	DEBUG4,
	DEBUG3,
	DEBUG2,
	DEBUG1,
	DEBUG,
	trace,
	INFO,
	info,
	WARNING,
	warning,
	ERROR,
	error,
	CRITICAL,
	critical
};

class GKLogging
{
	public:
		GKLogging(void) = delete;
		~GKLogging(void) = delete;

		static src::severity_logger< severity_level > GKLogger;

		static void initConsoleLog(void);
		static void initDebugFile(
			const std::string & baseName,
			const fs::perms prms = fs::no_perms);

	protected:

	private:
		static bool initialized;
		static void init(void);
};

#define LOG(sev)	BOOST_LOG_SEV(GKLogging::GKLogger, sev)

#if DEBUGGING_ON
#define GKLog(level, m1) if(GLogiK::GKDebug) { LOG(level) << m1; }
#define GKLog2(level, m1, m2) if(GLogiK::GKDebug) { LOG(level) << m1 << m2; }
#define GKLog3(level, m1, m2, m3) if(GLogiK::GKDebug) { LOG(level) << m1 << m2 << m3; }
#define GKLog4(level, m1, m2, m3, m4) if(GLogiK::GKDebug) { LOG(level) << m1 << m2 << " - " << m3 << m4; }
#define GKLog6(level, m1, m2, m3, m4, m5, m6) \
	if(GLogiK::GKDebug) { LOG(level) << m1 << m2 << " - " << m3 << m4 << " - " << m5 << m6;	}
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
	const std::string & msg)
{
	GKLog(level, msg);
	syslog(priority, "%s", msg.c_str());
}

inline void GKSysLogInfo(const std::string & msg)
{
	GKSysLog(LOG_INFO, info, msg);
}

inline void GKSysLogWarning(const std::string & msg)
{
	GKSysLog(LOG_WARNING, warning, msg);
}

inline void GKSysLogError(const std::string & msg)
{
	GKSysLog(LOG_ERR, error, msg);
}

inline void GKSysLogWarning(
	const std::string & msg1,
	const std::string & msg2)
{
	std::ostringstream buffer(std::ios_base::app);
	buffer << msg1 << msg2;
	GKSysLogWarning(buffer.str());
}

inline void GKSysLogError(
	const std::string & msg1,
	const std::string & msg2)
{
	std::ostringstream buffer(std::ios_base::app);
	buffer << msg1 << msg2;
	GKSysLogError(buffer.str());
}


#if DEBUGGING_ON
#define GK_LOG_FUNC BOOST_LOG_FUNC()
#else
#define GK_LOG_FUNC
#endif

} // namespace NSGKUtils

#endif
