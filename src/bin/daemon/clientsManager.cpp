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

#include <stdexcept>

#include <syslog.h>

#include "lib/utils/utils.h"

#include "clientsManager.h"

namespace GLogiK
{

ClientsManager::ClientsManager(GKDBus* pDBus) : DBus(pDBus) {
	LOG(DEBUG2) << "initializing clients manager";

	this->DBus->addEvent_StringToBool_Callback( this->DBus_object_, this->DBus_interface_, "RegisterClient",
		{	{"s", "client_unique_id", "in", "client unique ID"},
			{"b", "did_register_succeeded", "out", "did the RegisterClient method succeeded ?"} },
		std::bind(&ClientsManager::registerClient, this, std::placeholders::_1) );

	this->DBus->addEvent_StringToBool_Callback( this->DBus_object_, this->DBus_interface_, "UnregisterClient",
		{	{"s", "client_unique_id", "in", "client unique ID"},
			{"b", "did_unregister_succeeded", "out", "did the UnregisterClient method succeeded ?"} },
		std::bind(&ClientsManager::unregisterClient, this, std::placeholders::_1) );

	this->DBus->addEvent_TwoStringsToBool_Callback( this->DBus_object_, this->DBus_interface_, "UpdateClientState",
		{	{"s", "client_unique_id", "in", "client unique ID"},
			{"s", "client_new_state", "in", "client new state"},
			{"b", "did_unregister_succeeded", "out", "did the UpdateClientState method succeeded ?"} },
		std::bind(&ClientsManager::updateClientState, this, std::placeholders::_1, std::placeholders::_2) );
}

ClientsManager::~ClientsManager() {
	LOG(DEBUG2) << "exiting clients manager";
}

void ClientsManager::runLoop(void) {
	this->devicesManager.startMonitoring(this->DBus);
}

const bool ClientsManager::registerClient(const std::string & uniqueString) {
	try {
		const auto & client = this->clients_.at(uniqueString);
		if( client ) {
			LOG(WARNING) << uniqueString << " client already registered";
			syslog(LOG_WARNING, "client already registered");
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(DEBUG2) << "registering client : " << uniqueString;
		this->clients_[uniqueString] = true;
		return true;
	}
	return false;
}

const bool ClientsManager::unregisterClient(const std::string & uniqueString) {
	try {
		const auto & client = this->clients_.at(uniqueString);
		if( client ) {
			this->clients_.erase(uniqueString);
			LOG(DEBUG2) << "unregistered client : " << uniqueString;
			return true;
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(WARNING) << uniqueString << " client not registered";
		syslog(LOG_WARNING, "client not registered");
	}
	return false;
}

const bool ClientsManager::updateClientState(const std::string & uniqueString, const std::string & state) {
	LOG(DEBUG2) << "client : " << uniqueString << " - state : " << state;
	return false;
}

} // namespace GLogiK

