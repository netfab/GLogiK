/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2018  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_BIN_DAEMON_GLOGIKD_DAEMON_HPP_
#define SRC_BIN_DAEMON_GLOGIKD_DAEMON_HPP_

#define GLOGIKD_DAEMON_NAME "GLogiKd"

#include <cstdio>
#include <sys/types.h>

#include <fstream>
#include <string>

#include "daemonControl.hpp"

#include "lib/dbus/GKDBus.hpp"

namespace GLogiK
{

class GLogiKDaemon
	:	public DaemonControl
{
	public:
		GLogiKDaemon(void);
		~GLogiKDaemon(void);

		int run(const int& argc, char *argv[]);

	protected:
	private:
		pid_t _pid = 0;
		FILE* _LOGfd = nullptr;
		std::string _pidFileName;
		std::ofstream _pidFile;
		NSGKDBus::GKDBus* _pDBus;

		void daemonize(void);
		void createPIDFile(void);
		void dropPrivileges(void);
		void parseCommandLine(const int& argc, char *argv[]);
		static void handleSignal(int sig);
};

} // namespace GLogiK

#endif
