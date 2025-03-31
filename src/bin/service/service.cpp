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

#include <poll.h>

#include <cstring>
#include <cstdlib>

#include <syslog.h>

#include <new>
#include <fstream>
#include <iostream>
#include <sstream>

#include <config.h>

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

	LOG(info) << GLOGIKS_DESKTOP_SERVICE_NAME << " desktop service process exiting, bye !";
}

int DesktopService::run(void)
{
	GK_LOG_FUNC

	/* -- -- -- */

	GKDepsMap_type dependencies;

	std::string binaryVersion(GLOGIKS_DESKTOP_SERVICE_NAME);
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
			};

	/* -- -- -- */

	if(_version) {
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
		FileSystem GKfs;
		SessionManager session;

		DBus.init();

		DBus.connectToSystemBus(GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME);
		DBus.connectToSessionBus(GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME);

		struct pollfd fds[2];
		nfds_t nfds = 2;

		fds[0].fd = session.openConnection();
		fds[0].events = POLLIN;

		fds[1].fd = GKfs.getNotifyQueueDescriptor();
		fds[1].events = POLLIN;

		DBusHandler handler(_pid, &GKfs, &dependencies);

		while( session.isSessionAlive() and
				handler.getExitStatus() )
		{
			int num = poll(fds, nfds, 150);

			// data to read ?
			if( num > 0 ) {
				if( fds[0].revents & POLLIN ) {
					session.processICEMessages();
					continue;
				}

				if( fds[1].revents & POLLIN ) {
					/* checking if any received filesystem notification matches
					 * any device configuration file. If yes, reload the file,
					 * and send configuration to daemon */
					handler.checkNotifyEvents(&GKfs);
				}
			}

			DBus.checkForMessages();
		}

		// also unregister with daemon before cleaning
		handler.cleanDBusRequests();

		DBus.exit();
	}

	GKLog(trace, "exiting with success")

	return EXIT_SUCCESS;
}

} // namespace GLogiK

