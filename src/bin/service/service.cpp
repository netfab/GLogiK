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

#include <poll.h>

#include <cstring>
#include <cstdlib>

#include <syslog.h>

#include <new>
#include <fstream>
#include <iostream>
#include <sstream>

#include <config.h>

#if HAVE_SYSTRAY && HAVE_QT
/* must be included before sessionManager.hpp
 * because of conflict with X11/SM/SMlib.h
 * #error qdatastream.h must be included before
 * any header file that defines Status */
#include "systray.hpp"
#include <QApplication>
#include <QtGlobal>
#endif

#include "lib/dbus/GKDBus.hpp"
#include "lib/utils/utils.hpp"
#include "lib/shared/sessionManager.hpp"
#include "lib/shared/glogik.hpp"

#include "DBusHandler.hpp"

#include "service.hpp"

#include "include/DepsMap.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

DesktopService::DesktopService(const bool & version)
	:	_pid(0),
		_version(version)
{
}

DesktopService::~DesktopService()
{
	GK_LOG_FUNC

	LOG(info) << GLOGIK_DESKTOP_SERVICE_NAME << " desktop service process exiting, bye !";
}

int DesktopService::run(void)
{
	GK_LOG_FUNC

	/* -- -- -- */

	GKDepsMap_type dependencies;

	std::string binaryVersion(GLOGIK_DESKTOP_SERVICE_NAME);
	binaryVersion += " version ";
	binaryVersion += VERSION;

		dependencies[GKBinary::GK_DESKTOP_SERVICE] =
			{
				{"libevdev", GK_DEP_LIBEVDEV_VERSION_STRING},
				{"libSM", GK_DEP_SM_VERSION_STRING},
				{"libICE", GK_DEP_ICE_VERSION_STRING},
				{"libX11", GK_DEP_LIBX11_VERSION_STRING},
				{"libXtst", GK_DEP_LIBXTST_VERSION_STRING},
#if HAVE_DESKTOP_NOTIFICATIONS
				{"libnotify", GK_DEP_LIBNOTIFY_VERSION_STRING},
#else
				{"libnotify", "-"},
#endif
#if HAVE_SYSTRAY && HAVE_QT
				{"Qt", GK_DEP_QT_VERSION_STRING, qVersion()}, /* qVersion() from <QtGlobal> */
#endif
			};

	/* -- -- -- */

	if(_version)
	{
		printVersionDeps(binaryVersion, dependencies);
		return EXIT_SUCCESS;
	}

	/* -- -- -- */

	{
		LOG(info) << "Starting " << binaryVersion;

		_pid = /*NSGKUtils::*/process::detach();

		GKLog2(trace, "process detached - pid: ", _pid)
	}

	{

#if HAVE_SYSTRAY && HAVE_QT
		int argc = 1;
		char * argv[] = {(char*)GLOGIK_DESKTOP_SERVICE_NAME};
		QApplication app(argc, argv);
#endif

		FileSystem GKfs;
		/* SessionManager must be instantiated *after* QApplication
		 * to take precedence over SIGINT/SIGTERM signals. */
		SessionManager session;

		NSGKDBus::GKDBus DBus;
		DBus.init();

		DBus.connectToSystemBus(
			GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME,
			NSGKDBus::ConnectionFlag::GKDBUS_MULTIPLE
		);
		DBus.connectToSessionBus(
			GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME,
			NSGKDBus::ConnectionFlag::GKDBUS_MULTIPLE
		);

		struct pollfd fds[2];
		nfds_t nfds = 2;

		fds[0].fd = session.openConnection();
		fds[0].events = POLLIN;

		fds[1].fd = GKfs.getNotifyQueueDescriptor();
		fds[1].events = POLLIN;

		try
		{ // <<<
			{
				/* DBusHandler constructor can potentially throw GLogiKExcept */
				DBusHandler dbusHandler(_pid, &GKfs, &DBus, &dependencies);

#if HAVE_SYSTRAY && HAVE_QT
				DesktopServiceSystray systray;
				systray.hide(); // QMainWindow
#endif

				while( true )
				{
					if( (! session.isAlive()) or dbusHandler.wantToStop() )
						break;

					int num = poll(fds, nfds, 150);

					// data to read ?
					if( num > 0 )
					{
						if( fds[0].revents & POLLIN )
						{
							session.processICEMessages();
							continue;
						}

						if( fds[1].revents & POLLIN )
						{
							/* check if any received filesystem notification matches
							 * any device configuration file. If yes, reload the file,
							 * and send configuration to daemon. Can throw. */
							dbusHandler.checkNotifyEvents(&GKfs);
						}
					}

					DBus.checkForMessages();

#if HAVE_SYSTRAY && HAVE_QT
					app.processEvents();

					if( systray.wantToStop() )
					{
						LOG(info) << "process systray stop action --> bye bye";
						break;
					}
					else if( systray.wantToRestart() )
					{
						/* on next loop iteration, send a restart request on DBus
						 * and break the loop with dbusHandler.wantToStop() */
						dbusHandler.restartService();
						continue;
					}

					if( dbusHandler.isAnyDeviceUpdated() )
					{
						GKLog(trace, "updating systray context menu")
						const DevicesMap_type devices = dbusHandler.getDevicesMap();
						systray.updateContextMenu(devices); // may throw
						dbusHandler.resetDevicesUpdatedEvent();
					}

					if( systray.deviceEventTriggered() )
					{
						GKLog(trace, "process systray device event")
						const std::string & devID = systray.getDeviceID();
						switch( systray.getDeviceEvent() )
						{
							case SystrayDeviceEvent::DEVICE_START:
								dbusHandler.startDevice(devID);
								break;
							case SystrayDeviceEvent::DEVICE_STOP:
								dbusHandler.stopDevice(devID);
								break;
							case SystrayDeviceEvent::DEVICE_RESTART:
								dbusHandler.restartDevice(devID);
								break;
							default:
								LOG(warning) << "wrong systray device event";
								break;
						}

						systray.resetDeviceEvent();
					}
#endif
				} // while(true) main loop
			} // make sure DBusHandler object is destroyed to clean GKDBus events before exit

			DBus.exit();
		} // >>>
		catch (const GLogiKExcept & e)
		{
			DBus.exit();
			throw;
		}

#if HAVE_SYSTRAY && HAVE_QT
		app.exit();
#endif

	}

	GKLog(trace, "exiting with success")

	return EXIT_SUCCESS;
}

} // namespace GLogiK

