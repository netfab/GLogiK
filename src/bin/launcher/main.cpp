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

#include "lib/utils/utils.hpp"
#include "launcher.hpp"

using namespace GLogiK;

int main(int argc, char *argv[])
{
	using namespace NSGKUtils;

	try {
		DesktopServiceLauncher launcher(argc, argv);
		return launcher.run();
	}
	catch(const InitFailure & e) {
		syslog(LOG_ERR, "%s", e.what());
	}
	catch(const GLogiKExcept & e) {
		LOG(error) << e.what();
	}

	return EXIT_FAILURE;
}

