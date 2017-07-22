/*
 *
 *	This file is part of GLogiK project.
 *	GLogiKd, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2017  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef __GLOGIKD_DAEMON_H__
#define __GLOGIKD_DAEMON_H__

#include <cstdio>
#include <sys/types.h>

#include <fstream>
#include <sstream>
#include <string>

#include "daemon_control.h"


namespace GLogiKd
{

class GLogiKDaemon : public DaemonControl
{
	public:
		GLogiKDaemon(void);
		~GLogiKDaemon(void);

		int run(const int& argc, char *argv[]);

	protected:
	private:
		pid_t pid_ = 0;
		FILE* log_fd_ = nullptr;
		std::string pid_file_name_ = "";
		std::ofstream pid_file_;
		std::ostringstream buffer_;

		void daemonize(void);
		void parseCommandLine(const int& argc, char *argv[]);
		static void handle_signal(int sig);
};

} // namespace GLogiKd

#endif
