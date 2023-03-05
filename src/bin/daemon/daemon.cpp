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

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>

#include <pwd.h>
#include <grp.h>

#include <syslog.h>

#include <new>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <boost/version.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <config.h>

#include "lib/utils/GKLogging.hpp"
#include "lib/shared/glogik.hpp"
#include "lib/utils/utils.hpp"

#include "daemon.hpp"
#include "usbinit.hpp"

#if GKHIDAPI
#include "hidapi.hpp"
#endif

#include "devicesManager.hpp"

#if GKDBUS
#include "lib/dbus/GKDBus.hpp"

#include "clientsManager.hpp"
#endif

#include "include/DepsMap.hpp"


namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace GLogiK
{

using namespace NSGKUtils;

GLogiKDaemon::GLogiKDaemon(const int& argc, char *argv[])
	:	_version(false),
		_PIDFileCreated(false)
{
	GK_LOG_FUNC

	openlog(GLOGIKD_DAEMON_NAME, LOG_PID|LOG_CONS, LOG_DAEMON);

	// drop privileges before doing anything else
	this->dropPrivileges();

	/* -- -- -- */

	// initialize logging
	try {
		/* boost::po may throw */
		this->parseCommandLine(argc, argv);

#if DEBUGGING_ON
		if(GKLogging::GKDebug) {
			FileSystem::createDirectory(DEBUG_DIR, fs::owner_all | fs::group_all);

			GKLogging::initDebugFile("GLogiKd", fs::owner_read|fs::owner_write|fs::group_read);

			FileSystem::traceLastDirectoryCreation();
		}
#endif

		if( ! GLogiKDaemon::isDaemonRunning() ) {
			GKLogging::initConsoleLog();
		}

		GKSysLogInfo("successfully dropped root privileges");
	}
	catch (const std::exception & e) {
		syslog(LOG_ERR, "%s", e.what());
		syslog(LOG_ERR, "%s", "debug file not opened");
		throw InitFailure();
	}
}

GLogiKDaemon::~GLogiKDaemon()
{
	GK_LOG_FUNC

	GKLog(trace, "exiting daemon process")

	try {
		if( _PIDFileCreated && fs::is_regular_file(_pidFileName) ) {
			GKLog(info, "destroying PID file")
			if( unlink(_pidFileName.c_str()) != 0 ) {
				GKSysLogError("failed to unlink PID file");
			}
			else
				_PIDFileCreated = false;
		}
	}
	catch (const fs::filesystem_error & e) {
		GKSysLogError("boost::filesystem::is_regular_file() : ", e.what());
	}
	catch (const std::exception & e) {
		GKSysLogError("error destroying PID file : ", e.what());
	}

	GKSysLogInfo("bye !");

	closelog();
}

int GLogiKDaemon::run(void)
{
	GK_LOG_FUNC

	/* -- -- -- */

	GKDepsMap_type dependencies;

	std::string binaryVersion(GLOGIKD_DAEMON_NAME);
	binaryVersion += " version ";
	binaryVersion += VERSION;
	GKSysLogInfo(binaryVersion);

	{
		std::string boost_version;
		{
			int major = BOOST_VERSION / 100000;
			int minor = BOOST_VERSION / 100 % 1000;
			int patch = BOOST_VERSION % 100;

			boost_version += std::to_string(major);
			boost_version += ".";
			boost_version += std::to_string(minor);
			boost_version += ".";
			boost_version += std::to_string(patch);
		}

		dependencies[GKBinary::GK_DAEMON] =
			{
				{"boost", boost_version},
				{"libudev", GK_DEP_LIBUDEV_VERSION_STRING, DevicesManager::getLibudevVersion()},
				{"libusb", GK_DEP_LIBUSB_VERSION_STRING, USBInit::getLibUSBVersion()},
#if GKHIDAPI
				{"hidapi", GK_DEP_LIBHIDAPI_VERSION_STRING, hidapi::getHIDAPIVersion()},
#else
				{"hidapi", "-"},
#endif
#if GKDBUS
				{"DBus", GK_DEP_DBUS_VERSION_STRING, NSGKDBus::GKDBus::getDBusVersion()},
#else
				{"DBus", "-"},
#endif
			};
	}

	if( GLogiKDaemon::isDaemonRunning() ) {

		_pid = detachProcess(true);
		syslog(LOG_INFO, "process successfully daemonized");

		this->createPIDFile();

		// TODO handle return values and errno ?
		std::signal(SIGINT, GLogiKDaemon::handleSignal);
		std::signal(SIGTERM, GLogiKDaemon::handleSignal);
		//std::signal(SIGHUP, GLogiKDaemon::handleSignal);

#if GKDBUS
		NSGKDBus::GKDBus DBus(GLOGIK_DAEMON_DBUS_ROOT_NODE, GLOGIK_DAEMON_DBUS_ROOT_NODE_PATH);
		DBus.connectToSystemBus(GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME, NSGKDBus::ConnectionFlag::GKDBUS_SINGLE);
#endif

		DevicesManager devicesManager;

#if GKDBUS
		devicesManager.setDBus(&DBus);

		ClientsManager clientsManager(&devicesManager);
		clientsManager.initializeDBusRequests(&DBus);
#endif

		try {
			/* potential D-Bus requests received from services will be
			 * handled after devices initialization into startMonitoring() */
			devicesManager.startMonitoring();
#if GKDBUS
			clientsManager.waitForClientsDisconnections();
			clientsManager.cleanDBusRequests();
			DBus.disconnectFromSystemBus();
#endif
		}
		catch (const GLogiKExcept & e) {	// catch any monitoring failure
			std::ostringstream buffer(std::ios_base::app);
			buffer << "catched exception from device monitoring : " << e.what();
			GKSysLogWarning(buffer.str());

#if GKDBUS
			clientsManager.waitForClientsDisconnections();
			clientsManager.cleanDBusRequests();
			DBus.disconnectFromSystemBus();
#endif
			throw;
		}

	}
	else { // non-daemon mode
		GKSysLogInfo("non-daemon mode");

		if(_version) {
			printVersionDeps(binaryVersion, dependencies);
		}
	}

	return EXIT_SUCCESS;
}

void GLogiKDaemon::handleSignal(int sig) {
	GK_LOG_FUNC

	std::ostringstream buffer("caught signal : ", std::ios_base::app);
	switch( sig ) {
		case SIGINT:
		case SIGTERM:
			buffer << sig << " --> bye bye";
			GKSysLogInfo(buffer.str());
			std::signal(SIGINT, SIG_DFL);
			std::signal(SIGTERM, SIG_DFL);
			GLogiKDaemon::exitDaemon();
			break;
		default:
			buffer << sig << " --> unhandled";
			GKSysLogWarning(buffer.str());
			break;
	}
}

void GLogiKDaemon::createPIDFile(void) {
	GK_LOG_FUNC

	const fs::path PIDFile(_pidFileName);

	auto throwError = [&PIDFile] (const std::string & error, const char* what = nullptr) -> void {
		std::ostringstream buffer(std::ios_base::app);
		buffer << "failed to create PID file" << " : " << PIDFile.c_str() << " : " << error;
		if( what != nullptr ) {
			buffer << " : " << what;
		}
		throw GLogiKExcept( buffer.str() );
	};

	try {
		if( fs::exists(PIDFile) ) {
			throwError("already exist");
		}
	}
	catch (const fs::filesystem_error & e) {
		throwError("boost::filesystem error", e.what());
	}
	catch (const std::exception & e) {
		throwError("boost::filesystem (allocation) error", e.what());
	}

	// if path not found reset errno
	errno = 0;

	std::ofstream PIDStream;
	PIDStream.exceptions( std::ofstream::failbit );
	try {
		PIDStream.open(_pidFileName, std::ofstream::trunc);
		PIDStream << static_cast<long>(_pid);
		PIDStream.close();

		fs::permissions(PIDFile, fs::owner_read|fs::owner_write|fs::group_read|fs::others_read);
	}
	catch (const std::ofstream::failure & e) {
		throwError("open failure", e.what());
	}
	catch (const fs::filesystem_error & e) {
		throwError("set permissions failure", e.what());
	}
	catch (const std::exception & e) {
		throwError("set permissions (allocation) failure", e.what());
	}

	_PIDFileCreated = true;

	GKLog2(info, "created PID file : ", _pidFileName)
}

void GLogiKDaemon::parseCommandLine(const int& argc, char *argv[])
{
	po::options_description desc("Allowed options");

	desc.add_options()
//		("help,h", "produce help message")
		("daemonize,d", po::bool_switch()->default_value(false), "run in daemon mode")
		("pid-file,p", po::value(&_pidFileName), "define the PID file")
		("version,v", po::bool_switch()->default_value(false), "print some versions informations and exit")
	;

#if DEBUGGING_ON
	desc.add_options()
		("debug,D", po::bool_switch()->default_value(false), "run in debug mode")
	;
#endif

	po::variables_map vm;

	po::store(po::parse_command_line(argc, argv, desc), vm);

	po::notify(vm);

	bool daemonized = vm.count("daemonize") ? vm["daemonize"].as<bool>() : false;
	_version = vm.count("version") ? vm["version"].as<bool>() : false;

/*
	if (vm.count("help")) {
		GKSysLogInfo("displaying help");
		std::ostringstream buffer(std::ios_base::app);
		desc.print( buffer );
		throw DisplayHelp( buffer.str() );
	}
*/

	if( _version ) {
		/* disable daemon mode */
		GLogiKDaemon::daemonized = false;
	}
	else if( daemonized ) {
		GLogiKDaemon::daemonized = true;
	}

#if DEBUGGING_ON
	bool debug = vm.count("debug") ? vm["debug"].as<bool>() : false;

	if( debug ) {
		GKLogging::GKDebug = true;
	}
#endif
}

void GLogiKDaemon::dropPrivileges(void) {
	auto throwError = [] (const std::string & error) -> void {
		if(errno != 0) {
			std::ostringstream buffer(std::ios_base::app);
			buffer << error;
			buffer << " : " << strerror(errno);
			throw GLogiKExcept(buffer.str());
		}
		throw GLogiKExcept(error);
	};

	errno = 0;
	struct passwd * pw = getpwnam(GLOGIKD_USER);
	if(pw == nullptr)
		throwError("can't get password structure for GLOGIKD_USER");

	errno = 0;
	struct group * gr = getgrnam(GLOGIKD_GROUP);
	if(gr == nullptr)
		throwError("can't get group structure for GLOGIKD_GROUP");

	errno = 0;
	if(initgroups(GLOGIKD_USER, gr->gr_gid) < 0)
		throwError("failed to initialize group access list");

	errno = 0;
	int ret = -1;

#if defined(HAVE_SETRESGID)
	ret = setresgid(gr->gr_gid, gr->gr_gid, gr->gr_gid);
#elif defined(HAVE_SETEGID)
	if( (ret = setgid(gr->gr_gid)) == 0 )
		ret = setegid(gr->gr_gid);
#elif defined(HAVE_SETREGID)
	ret = setregid(gr->gr_gid, gr->gr_gid);
#else
#error "No API to drop group privileges"
#endif

	if( ret < 0 )
		throwError("failed to change group ID");

	errno = 0;
	ret = -1;

#if defined(HAVE_SETRESUID)
	ret = setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid);
#elif defined(HAVE_SETEUID)
	if( (ret = setuid(pw->pw_uid)) == 0 )
		ret = seteuid(pw->pw_uid);
#elif defined(HAVE_SETREUID)
	ret = setreuid(pw->pw_uid, pw->pw_uid);
#else
#error "No API to drop user privileges"
#endif

	if( ret < 0 )
		throwError("failed to change user ID");
}

} // namespace GLogiK

