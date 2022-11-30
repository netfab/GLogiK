/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2022  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_BIN_SERVICE_DESKTOP_NOTIFICATION_HPP_
#define SRC_BIN_SERVICE_DESKTOP_NOTIFICATION_HPP_

#include <string>

#include <config.h>

#if HAVE_LIBNOTIFY
#include <libnotify/notify.h>
#else
#error "notifications via D-Bus only are currently not implemented" // TODO
#endif

namespace GLogiK
{

class desktopNotification
{
	public:
		desktopNotification(void);
		desktopNotification(
			const std::string & summary,
			const std::string & body,
			const std::string & icon
		);
		~desktopNotification(void);

	protected:

	private:
#if HAVE_LIBNOTIFY
		NotifyNotification* _pNotification;
#endif

		void setParameters(
			const std::string & summary,
			const std::string & body,
			const std::string & icon
		);
		void show(void);
};

} // namespace GLogiK

#endif
