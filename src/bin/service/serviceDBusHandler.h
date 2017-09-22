/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
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

#ifndef __GLOGIKS_DESKTOP_SERVICE_DBUS_HANDLER_H__
#define __GLOGIKS_DESKTOP_SERVICE_DBUS_HANDLER_H__

#include <string>

#include "lib/dbus/GKDBus.h"

#define MAXIMUM_WARNINGS_BEFORE_FATAL_ERROR 10

namespace GLogiK
{

class ServiceDBusHandler
{
	public:
		ServiceDBusHandler(void);
		~ServiceDBusHandler(void);

		void updateSessionState(void);
		void checkDBusMessages(void);

	protected:

	private:
		uint8_t warn_count_;
		GKDBus* DBus;

		std::string current_session_;	/* current session object path */
		std::string session_state_;		/* session state */

		void setCurrentSessionObjectPath(void);
		const std::string getCurrentSessionState(const bool logoff=false);

		void reportChangedState(void);
};

} // namespace GLogiK

#endif
