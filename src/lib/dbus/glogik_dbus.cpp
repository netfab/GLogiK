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


#include "glogik_dbus.h"
#include "include/log.h"

namespace GLogiK
{

DBus::DBus() : sessionConnection(nullptr), systemConnection(nullptr)
{
	LOG(DEBUG1) << "dbus object initialization";
	dbus_error_init(&(this->error));
}

DBus::~DBus()
{
	LOG(DEBUG1) << "dbus object destruction";
	if(this->sessionConnection) {
		LOG(DEBUG) << "closing DBus Session connection";
		dbus_connection_close(this->sessionConnection);
	}
	if(this->systemConnection) {
		LOG(DEBUG) << "closing DBus System connection";
		dbus_connection_close(this->systemConnection);
	}
}

void DBus::connectToSessionBus(void) {
	this->sessionConnection = dbus_bus_get(DBUS_BUS_SESSION, &this->error);
	if( dbus_error_is_set(&this->error) ) {
		LOG(ERROR) << "DBus Session connection failure : " << this->error.message;
		dbus_error_free(&this->error);
	}
	if( this->sessionConnection == nullptr ) {
		// TODO exception
		return;
	}
	LOG(INFO) << "DBus Session connection opened";
}

} // namespace GLogiK

