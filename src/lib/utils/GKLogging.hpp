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

#include <string>

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
	WARNING,
	ERROR,
	CRITICAL
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

inline void GKSysLog(const int priority, const severity_level level, const std::string & to_log) {
#if DEBUGGING_ON
	if(GLogiK::GKDebug) {
		LOG(level) << to_log;
	}
#endif
	syslog(priority, "%s", to_log.c_str());
}

#define GKSysLog_UnknownDevice \
	std::string error(CONST_STRING_UNKNOWN_DEVICE); error += devID;\
	GKSysLog(LOG_ERR, ERROR, error);

} // namespace NSGKUtils

#endif
