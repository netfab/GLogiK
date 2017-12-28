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

#include <new>
#include <algorithm>
#include <string>

#include "GKDBus.h"

#include "lib/utils/utils.h"

namespace GLogiK
{

GKDBus::GKDBus(const std::string & rootnode) : buffer_("", std::ios_base::app),
	current_conn_(nullptr),
	session_conn_(nullptr),
	system_conn_(nullptr)
{
#if DEBUGGING_ON
	LOG(DEBUG1) << "dbus object initialization";
#endif
	dbus_error_init(&(this->error_));
	this->defineRootNode(rootnode);
}

GKDBus::~GKDBus()
{
#if DEBUGGING_ON
	LOG(DEBUG1) << "dbus object destruction";
#endif
	if(this->session_conn_) {
#if DEBUGGING_ON
		LOG(DEBUG) << "closing DBus Session connection";
#endif
		int ret = dbus_bus_release_name(this->session_conn_, this->session_name_.c_str(), nullptr);
		this->checkReleasedName(ret);
		dbus_connection_unref(this->session_conn_);
	}

	if(this->system_conn_) {
#if DEBUGGING_ON
		LOG(DEBUG) << "closing DBus System connection";
#endif

		int ret = dbus_bus_release_name(this->system_conn_, this->system_name_.c_str(), nullptr);
		this->checkReleasedName(ret);
		dbus_connection_unref(this->system_conn_);
	}
}

void GKDBus::checkReleasedName(int ret) {
	switch(ret) {
		case DBUS_RELEASE_NAME_REPLY_RELEASED:
			LOG(DEBUG1) << "name released";
			break;
		case DBUS_RELEASE_NAME_REPLY_NOT_OWNER:
			LOG(DEBUG1) << "not owner, cannot release";
			break;
		case DBUS_RELEASE_NAME_REPLY_NON_EXISTENT:
			LOG(DEBUG1) << "nobody owned the name";
			break;
		default:
			LOG(WARNING) << "unknown return value";
			break;
	}
}


} // namespace GLogiK

