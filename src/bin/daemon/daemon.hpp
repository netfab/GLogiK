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

#include <sys/types.h>

#include <string>

#include "daemonStatus.hpp"

#if GKDBUS
#include "sleepInhibition.hpp"
#endif

namespace GLogiK::Daemon
{

class GLogiKDaemon
#if GKDBUS
	:	public DaemonStatus,
		private SleepInhibition
#else
	:	public DaemonStatus
#endif
{
	public:
		GLogiKDaemon(const int& argc,char *argv[]);
		~GLogiKDaemon();

		int run();

	protected:
	private:
		std::string _pidFileName;
		pid_t _pid = 0;
		bool _version;
		bool _PIDFileCreated;

		void createPIDFile();
		void dropPrivileges();
		void parseCommandLine(const int& argc, char *argv[]);
		static void handleSignal(int signum);
};

} // namespace GLogiK::Daemon
