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

#include <sstream>
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
		{	{"s", "client_session_object_path", "in", "client session object path"},
			{"b", "did_register_succeeded", "out", "did the RegisterClient method succeeded ?"},
			{"s", "failure_reason_or_client_id", "out", "if register success (bool==true), unique client ID, else (bool=false) failure reason"} },
		std::bind(&ClientsManager::registerClient, this, std::placeholders::_1) );

	this->DBus->addEvent_StringToBool_Callback( this->DBus_object_, this->DBus_interface_, "UnregisterClient",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"b", "did_unregister_succeeded", "out", "did the UnregisterClient method succeeded ?"} },
		std::bind(&ClientsManager::unregisterClient, this, std::placeholders::_1) );

	this->DBus->addEvent_TwoStringsToBool_Callback( this->DBus_object_, this->DBus_interface_, "UpdateClientState",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
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

	for( auto & client_pair : this->clients_ ) {
		Client* pClient = client_pair.second;
		if( pClient != nullptr ) { /* sanity check */
			this->buffer_.str("destroying unfreed client : ");
			this->buffer_ << client_pair.first;
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

const std::string ClientsManager::generateRandomClientID(void) {
	RandomGenerator rand;
	std::ostringstream ret(std::ios_base::app);
	ret << rand.getString(6) << "-" << rand.getString(4) << "-"
		<< rand.getString(4) << "-" << rand.getString(6);
	return ret.str();
}

const bool ClientsManager::registerClient(const std::string & clientSessionObjectPath) {
	try {
		for(const auto & client_pair : this->clients_ ) {
			const std::string & clientID = client_pair.first;
			Client* pClient = client_pair.second;

			/* handling crashed clients */
			if( ! pClient->isAlive() ) {
				this->buffer_.str("unregistering lost client (maybe crashed) with ID : ");
				this->buffer_ << clientID;
				LOG(WARNING) << this->buffer_.str();
				syslog(LOG_WARNING, this->buffer_.str().c_str());
				this->unregisterClient(clientID);
				continue;
			}

			if( pClient->getSessionObjectPath() == clientSessionObjectPath ) {
				this->buffer_.str("client already registered : ");
				this->buffer_ << clientSessionObjectPath;
				LOG(WARNING) << this->buffer_.str();
				syslog(LOG_WARNING, this->buffer_.str().c_str());
				const std::string reason("already registered, please try again");
				/* appending failure reason to DBus reply */
				this->DBus->appendExtraToMethodCallReply(reason);

				/* process to check if registered clients are still alives */
				for(auto & new_pair : this->clients_ ) {
					new_pair.second->uncheck();
				}
				LOG(DEBUG2) << "sending clients ReportYourself signal";
				try {
					this->DBus->initializeBroadcastSignal(BusConnection::GKDBUS_SYSTEM,
					this->DBus_CSMH_object_path_, this->DBus_CSMH_interface_,
					"ReportYourself");
					this->DBus->sendBroadcastSignal();
				}
				catch (const GLogiKExcept & e) {
					LOG(ERROR) << "DBus signal failure : " << e.what();
				}

				/* register failure, sender should wait and retry */
				return false;
			}
		}
		throw std::out_of_range("not found");
	}
	catch (const std::out_of_range& oor) {
		const std::string clientID = this->generateRandomClientID();

		this->buffer_.str("registering new client with ID : ");
		this->buffer_ << clientID;
		LOG(DEBUG2) << this->buffer_.str();
		syslog(LOG_INFO, this->buffer_.str().c_str());

		this->clients_[clientID] = new Client(clientSessionObjectPath);
		/* appending client id to DBus reply */
		this->DBus->appendExtraToMethodCallReply(clientID);
		return true;
	}
	return false;
}

const bool ClientsManager::unregisterClient(const std::string & clientID) {
	try {
		Client* pClient = this->clients_.at(clientID);

		this->buffer_.str("unregistering client : ");
		this->buffer_ << clientID;
		LOG(DEBUG2) << this->buffer_.str();
		syslog(LOG_INFO, this->buffer_.str().c_str());

		delete pClient;
		pClient = nullptr;
		this->clients_.erase(clientID);
		return true;
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("tried to unregister unknown client : ");
		this->buffer_ << clientID;
		LOG(WARNING) << this->buffer_.str();
		syslog(LOG_WARNING, this->buffer_.str().c_str());
	}
	return false;
}

const bool ClientsManager::updateClientState(const std::string & clientID, const std::string & state) {
	LOG(DEBUG2) << "updating client state : " << clientID << " - " << state;
	try {
		Client* pClient = this->clients_.at(clientID);
		pClient->updateSessionState(state);
		return true;
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("client not registered : ");
		this->buffer_ << clientID;
		LOG(WARNING) << this->buffer_.str();
		syslog(LOG_WARNING, this->buffer_.str().c_str());
	}

	return false;
}

} // namespace GLogiK

