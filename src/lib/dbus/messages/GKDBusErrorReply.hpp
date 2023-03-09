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

#ifndef SRC_LIB_DBUS_MSG_GKDBUS_ERROR_REPLY_HPP_
#define SRC_LIB_DBUS_MSG_GKDBUS_ERROR_REPLY_HPP_

#include <dbus/dbus.h>

#include "GKDBusMessage.hpp"

namespace NSGKDBus
{

class GKDBusErrorReply : public GKDBusMessage
{
	public:
		GKDBusErrorReply(
			DBusConnection* const connection,
			DBusMessage* message,
			const char* errorMessage
		);
		~GKDBusErrorReply();

	protected:
	private:

};

class GKDBusMessageErrorReply
{
	public:

	protected:
		GKDBusMessageErrorReply();
		~GKDBusMessageErrorReply();

		void initializeErrorReply(
			DBusConnection* const connection,
			DBusMessage* message,
			const char* errorMessage
		);
		void sendErrorReply(void);

		void buildAndSendErrorReply(
			DBusConnection* const connection,
			DBusMessage* message,
			const char* errorMessage
		);

	private:
		GKDBusErrorReply* _errorReply;

};

} // namespace NSGKDBus

#endif
