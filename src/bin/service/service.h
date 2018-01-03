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

#ifndef __GLOGIKS_DESKTOP_SERVICE_H__
#define __GLOGIKS_DESKTOP_SERVICE_H__

#include <poll.h>

#include <sys/types.h>

#include <cstdio>

#include <sstream>

namespace GLogiK
{

#define GLOGIKS_DESKTOP_SERVICE_NAME "GLogiKs"

class DesktopService
{
	public:
		DesktopService(void);
		~DesktopService(void);

		int run(const int& argc, char *argv[]);

	protected:

	private:
		pid_t pid_ = 0;
		FILE* log_fd_ = nullptr;
		std::ostringstream buffer_;
		struct pollfd fds[1];

		void daemonize(void);
};

} // namespace GLogiK

#endif
