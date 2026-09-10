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

#include "config.h"

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

#include "lib/shared/glogik.hpp"
#include "lib/utils/utils.hpp"

#include "daemon.hpp"
#include "usbinit.hpp"

#if HAVE_HIDAPI
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

	openlog(GLOGIK_DAEMON_NAME, LOG_PID|LOG_CONS, LOG_DAEMON);

	/* -- -- -- */

	try
	{
		/* boost::po may throw */
		this->parseCommandLine(argc, argv);

		if( GLogiKDaemon::isDaemonRunning() )
		{
			_pid = /*NSGKUtils::*/process::deamonize();
			syslog(LOG_INFO, "process successfully daemonized");

			/* create PID file before dropping privileges */
			this->createPIDFile();
		}

		this->dropPrivileges();

		/* initialize logging */
		/* -- -- -- */
#if DEBUGGING_ON
		if(GKLogging::GKDebug)
			GKLogging::initDebugFile(GLOGIK_DAEMON_NAME, fs::owner_read|fs::owner_write|fs::group_read);
#endif

		if( ! GLogiKDaemon::isDaemonRunning() )
			GKLogging::initConsoleLog(GLOGIK_DAEMON_NAME);

		/* -- -- -- */

		GKSysLogInfo("successfully dropped root privileges");

		if( GLogiKDaemon::isDaemonRunning() )
		{
			GKLog2(info, "created PID file : ", _pidFileName)

			process::setSignalHandler( SIGINT, GLogiKDaemon::handleSignal);
			process::setSignalHandler(SIGTERM, GLogiKDaemon::handleSignal);
			// TODO SIGHUP ?
		}
	}
	catch (const std::exception & e)
	{
		syslog(LOG_ERR, "%s", e.what());
		throw InitFailure();
	}

}

GLogiKDaemon::~GLogiKDaemon()
{
	GK_LOG_FUNC

	GKLog(trace, "exiting daemon process")
	GKSysLogInfo("bye !");

	closelog();
}

int GLogiKDaemon::run(void)
{
	GK_LOG_FUNC

	/* -- -- -- */

	GKDepsMap_type dependencies;

	std::string binaryVersion(GLOGIK_DAEMON_NAME);
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
#if HAVE_HIDAPI
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

	if( GLogiKDaemon::isDaemonRunning() )
	{
#if GKDBUS
		NSGKDBus::GKDBus DBus;

		try
		{ // <<<
			DBus.init();
			DBus.connectToSystemBus(GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME);
#endif
			DevicesManager devicesManager;

#if GKDBUS
			devicesManager.setDBus(&DBus);

			ClientsManager clientsManager(&DBus, &devicesManager, &dependencies);
#endif

			try
			{
				this->startSleepInhibition(&DBus, &devicesManager);

				/* potential D-Bus requests received from services will be
				 * handled after devices initialization into startMonitoring() */
				devicesManager.startMonitoring();
#if GKDBUS
				clientsManager.waitForClientsDisconnections();
				clientsManager.cleanGKDBusEvents();

				this->stopSleepInhibition();
				DBus.exit();
#endif
			}
			catch (const GLogiKExcept & e)
			{
				GKSysLogWarning("caught exception, cleaning");
#if GKDBUS
				clientsManager.waitForClientsDisconnections();
				clientsManager.cleanGKDBusEvents();

				this->stopSleepInhibition();
#endif
				throw;
			}
		} // >>>
		catch (const GLogiKExcept & e)
		{
#if GKDBUS
			DBus.exit();
#endif
			throw;
		}

	}
	else
	{ // non-daemon mode
		GKSysLogInfo("non-daemon mode");

		if(_version)
			printVersionDeps(binaryVersion, dependencies);
	}

	return EXIT_SUCCESS;
}

void GLogiKDaemon::handleSignal(int signum)
{
	GK_LOG_FUNC

	switch( signum )
	{
		case SIGINT:
		case SIGTERM:
			GKSysLogInfo( process::getSignalHandlingDesc(signum, " --> bye bye") );

			process::resetSignalHandler(SIGINT);
			process::resetSignalHandler(SIGTERM);

			GLogiKDaemon::exitDaemon();
			break;
		default:
			GKSysLogWarning( process::getSignalHandlingDesc(signum, " --> unhandled") );
			break;
	}
}

void GLogiKDaemon::createPIDFile(void)
{
	GK_LOG_FUNC

	const fs::path PIDFile(_pidFileName);

	// lambda
	auto throw_error = [&PIDFile] (
		const std::string & error,
		const char* what = nullptr) -> void
	{
		std::ostringstream buffer(std::ios_base::app);
		buffer << "failed to create PID file" << " : " << PIDFile.c_str() << " : " << error;
		if( what != nullptr )
			buffer << " : " << what;
		throw GLogiKExcept( buffer.str() );
	};

	try
	{
		if( fs::exists(PIDFile) )
			throw GLogiKExcept("already exist");
	}
	catch (const GLogiKExcept & e)
	{
		throw_error(e.what());
	}
	catch (const fs::filesystem_error & e)
	{
		throw_error("boost::filesystem error", e.what());
	}
	catch (const std::exception & e)
	{
		throw_error("boost::filesystem (allocation) error", e.what());
	}

	// if path not found reset errno
	errno = 0;

	std::ofstream PIDStream;
	PIDStream.exceptions( std::ofstream::failbit );
	try
	{
		PIDStream.open(_pidFileName, std::ofstream::trunc);
		PIDStream << static_cast<long>(_pid);
		PIDStream.close();

		fs::permissions(PIDFile, fs::owner_read|fs::owner_write|fs::group_read|fs::others_read);
	}
	catch (const std::ofstream::failure & e)
	{
		throw_error("open failure", e.what());
	}
	catch (const fs::filesystem_error & e)
	{
		throw_error("set permissions failure", e.what());
	}
	catch (const std::exception & e)
	{
		throw_error("set permissions (allocation) failure", e.what());
	}

	_PIDFileCreated = true;
}

void GLogiKDaemon::parseCommandLine(const int& argc, char *argv[])
{
	po::options_description desc("Allowed options");

	desc.add_options()
//		("help,h", "produce help message")
		("daemonize,d", po::bool_switch()->default_value(false), "run in daemon mode")
		("pid-file,p", po::value(&_pidFileName), "define the PID file")
		("verbose,V", po::bool_switch()->default_value(false), "verbose mode")
		("version,v", po::bool_switch()->default_value(false), "print some versions informations")
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
	if (vm.count("help")
	{
		GKSysLogInfo("displaying help");
		std::ostringstream buffer(std::ios_base::app);
		desc.print( buffer );
		throw DisplayHelp( buffer.str() );
	}
*/

	if( _version ) /* disable daemon mode */
		GLogiKDaemon::daemonized = false;
	else if( daemonized )
		GLogiKDaemon::daemonized = true;

	bool verbose = vm.count("verbose") ? vm["verbose"].as<bool>() : false;
	if( verbose )
		GKLogging::GKVerbose = true;

#if DEBUGGING_ON
	bool debug = vm.count("debug") ? vm["debug"].as<bool>() : false;

	if( debug )
		GKLogging::GKDebug = true;
#endif
}

void GLogiKDaemon::dropPrivileges(void)
{
	// lambda
	auto throw_error = [] (const std::string & error) -> void
	{
		if(errno != 0)
		{
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
		throw_error("can't get password structure for GLOGIKD_USER");

	errno = 0;
	struct group * gr = getgrnam(GLOGIKD_GROUP);
	if(gr == nullptr)
		throw_error("can't get group structure for GLOGIKD_GROUP");

	errno = 0;
	if(initgroups(GLOGIKD_USER, gr->gr_gid) < 0)
		throw_error("failed to initialize group access list");

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
		throw_error("failed to change group ID");

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
		throw_error("failed to change user ID");
}

} // namespace GLogiK

