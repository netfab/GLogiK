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

#include <boost/filesystem.hpp>

#include "lib/utils/utils.hpp"
#include "lib/shared/glogik.hpp"

#include "initLog.hpp"

namespace fs = boost::filesystem;

namespace GLogiK
{

using namespace NSGKUtils;

InitLog::InitLog(const int& argc, char *argv[])
{
	GK_LOG_FUNC

	openlog(GLOGIKS_DESKTOP_SERVICE_NAME, LOG_PID|LOG_CONS, LOG_USER);

	// initialize logging
	try {
		/* boost::po may throw */
		this->parseCommandLine(argc, argv);

#if DEBUGGING_ON
		if(GKLogging::GKDebug) {
			GKLogging::initDebugFile(GLOGIKS_DESKTOP_SERVICE_NAME, fs::owner_read|fs::owner_write|fs::group_read);
		}
#endif
		GKLogging::initConsoleLog();
	}
	catch (const std::exception & e) {
		syslog(LOG_ERR, "%s", e.what());
		throw InitFailure();
	}
}

InitLog::~InitLog(void)
{
	//closelog();
}

const bool InitLog::getBooleanOption(const std::string & optstr) const
{
	return (_vm.count(optstr) ? _vm[optstr].as<bool>() : false);
}

void InitLog::parseCommandLine(const int& argc, char *argv[])
{
	GK_LOG_FUNC

	po::options_description desc("Allowed options");

	desc.add_options()
		("version,v", po::bool_switch()->default_value(false),
		 "print some versions informations and exit")
	;

#if DEBUGGING_ON
	desc.add_options()
		("debug,D", po::bool_switch()->default_value(false), "run in debug mode")
	;
#endif

	po::store(po::parse_command_line(argc, argv, desc), _vm);

	po::notify(_vm);

#if DEBUGGING_ON
		if( this->getBooleanOption("debug") ) {
			GKLogging::GKDebug = true;
		}
#endif
}

} // namespace GLogiK
