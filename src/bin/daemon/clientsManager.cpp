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

#include <thread>
#include <chrono>
#include <stdexcept>

#include <syslog.h>

#include "lib/utils/utils.h"

#include "clientsManager.h"

namespace GLogiK
{

ClientsManager::ClientsManager(GKDBus* pDBus) : buffer_("", std::ios_base::app), DBus(pDBus),
	devicesManager(nullptr)
{
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
			{"b", "did_updateclientstate_succeeded", "out", "did the UpdateClientState method succeeded ?"} },
		std::bind(&ClientsManager::updateClientState, this, std::placeholders::_1, std::placeholders::_2) );

	try {
		this->devicesManager = new DevicesManager();
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		throw GLogiKExcept("devices manager allocation failure");
	}
}

ClientsManager::~ClientsManager() {
	LOG(DEBUG2) << "destroying clients manager";

	delete this->devicesManager;
	this->devicesManager = nullptr;

	for( auto & client_it : this->clients_ ) {
		Client* pClient = client_it.second;
		if( pClient != nullptr ) { /* sanity check */
			this->buffer_.str("destroying unfreed client : ");
			this->buffer_ << client_it.first;
			LOG(WARNING) << this->buffer_.str();
			syslog(LOG_WARNING, this->buffer_.str().c_str());
			delete pClient;
			pClient = nullptr;
		}
	}
	this->clients_.clear();

	LOG(DEBUG2) << "exiting clients manager";

}

void ClientsManager::runLoop(void) {
	this->devicesManager->startMonitoring(this->DBus);

	LOG(DEBUG2) << "sending clients DaemonIsStopping signal";
	try {
		this->DBus->initializeBroadcastSignal(BusConnection::GKDBUS_SYSTEM,
			this->DBus_CSMH_object_path_, this->DBus_CSMH_interface_,
			"DaemonIsStopping");
		this->DBus->sendBroadcastSignal();
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << "DBus signal failure : " << e.what();
	}

	uint8_t count = 0;
	LOG(DEBUG2) << "waiting for clients to unregister ...";
	while( count++ < 10 and this->clients_.size() > 0 ) {
		this->devicesManager->checkDBusMessages();
		LOG(DEBUG3) << "sleeping for 400 ms ...";
		std::this_thread::sleep_for(std::chrono::milliseconds(400));
	}

}

const bool ClientsManager::registerClient(const std::string & uniqueString) {
	try {
		this->clients_.at(uniqueString);
		this->buffer_.str("client already registered : ");
		this->buffer_ << uniqueString;
		LOG(WARNING) << this->buffer_.str();
		syslog(LOG_WARNING, this->buffer_.str().c_str());
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("registering client : ");
		this->buffer_ << uniqueString;
		LOG(DEBUG2) << this->buffer_.str();
		syslog(LOG_INFO, this->buffer_.str().c_str());

		this->clients_[uniqueString] = new Client();
		return true;
	}
	return false;
}

const bool ClientsManager::unregisterClient(const std::string & uniqueString) {
	try {
		Client* pClient = this->clients_.at(uniqueString);

		this->buffer_.str("unregistering client : ");
		this->buffer_ << uniqueString;
		LOG(DEBUG2) << this->buffer_.str();
		syslog(LOG_INFO, this->buffer_.str().c_str());

		delete pClient;
		pClient = nullptr;
		this->clients_.erase(uniqueString);
		return true;
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("tried to unregister unknown client : ");
		this->buffer_ << uniqueString;
		LOG(WARNING) << this->buffer_.str();
		syslog(LOG_WARNING, this->buffer_.str().c_str());
	}
	return false;
}

const bool ClientsManager::updateClientState(const std::string & uniqueString, const std::string & state) {
	LOG(DEBUG2) << "updating client state : " << uniqueString << " - " << state;
	try {
		Client* pClient = this->clients_.at(uniqueString);
		pClient->updateSessionState(state);
		return true;
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("client not registered : ");
		this->buffer_ << uniqueString;
		LOG(WARNING) << this->buffer_.str();
		syslog(LOG_WARNING, this->buffer_.str().c_str());
	}

	return false;
}

} // namespace GLogiK

