/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_LIB_DBUS_MSG_GKDBUS_ASYNC_CONTAINER_HPP_
#define SRC_LIB_DBUS_MSG_GKDBUS_ASYNC_CONTAINER_HPP_

#include <string>

#include <dbus/dbus.h>

#include "GKDBusMessage.hpp"

namespace NSGKDBus
{

class GKDBusMessageAsyncContainer
	:	public GKDBusMessage
{
	public:
		void appendAsyncString(const std::string & value);
		void appendAsyncUInt64(const uint64_t value);

	protected:
		GKDBusMessageAsyncContainer(void);
		~GKDBusMessageAsyncContainer(void);

		void resetAsyncContainer(void);
		DBusMessage* getAsyncContainer(void) const;

	private:
		void newAsyncContainer(void);
		void freeAsyncContainer(void);
};

} // namespace NSGKDBus

#endif
