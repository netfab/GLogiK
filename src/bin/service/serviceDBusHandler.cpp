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

#include "lib/utils/utils.h"
#include "include/log.h"

#include "serviceDBusHandler.h"

namespace GLogiK
{

ServiceDBusHandler::ServiceDBusHandler() : DBus(nullptr) {
	try {
		this->DBus = new GKDBus();
		this->DBus->connectToSystemBus(GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME);

		/* getting consolekit current session */
		this->DBus->initializeRemoteMethodCall(BusConnection::GKDBUS_SYSTEM, "org.freedesktop.ConsoleKit",
			"/org/freedesktop/ConsoleKit/Manager", "org.freedesktop.ConsoleKit.Manager", "GetCurrentSession");
		this->DBus->sendRemoteMethodCall();
		this->DBus->waitForRemoteMethodCallReply();
		try {
			this->ck_current_session_ = this->DBus->getNextStringArgument();
			LOG(DEBUG2) << "CK current session: " << this->ck_current_session_;
		}
		catch ( const EmptyContainer & e ) {
			std::string err("can't get CK current session !");
			LOG(ERROR) << err;
			GK_ERR << err << "\n";
			throw;
		}
	}
	catch ( const GLogiKExcept & e ) {
		if(this->DBus != nullptr)
			delete this->DBus;
		throw;
	}
}

ServiceDBusHandler::~ServiceDBusHandler() {
	if(this->DBus != nullptr)
		delete this->DBus;
}

void ServiceDBusHandler::checkDBusMessages(void) {
	LOG(INFO) << "ok, run";
	//if( DBus->checkForNextMessage(BusConnection::GKDBUS_SYSTEM) ) {
	//}
	this->DBus->initializeRemoteMethodCall(BusConnection::GKDBUS_SYSTEM, "org.freedesktop.ConsoleKit",
		this->ck_current_session_.c_str(), "org.freedesktop.ConsoleKit.Session", "GetSessionState");
	this->DBus->sendRemoteMethodCall();

	//while(! waitForRemoteMethodCallReply() ) {
	//}
	this->DBus->waitForRemoteMethodCallReply();
	try {
		LOG(INFO) << this->DBus->getNextStringArgument();
	}
	catch ( const EmptyContainer & e ) {
		LOG(DEBUG3) << e.what();
	}
}

} // namespace GLogiK

