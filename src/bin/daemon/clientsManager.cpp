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

#include <functional>
#include <sstream>
#include <thread>
#include <chrono>
#include <stdexcept>

#include <syslog.h>

#include <config.h>

#include "lib/utils/utils.h"

#include "clientsManager.h"

namespace GLogiK
{

ClientsManager::ClientsManager(GKDBus* pDBus) : buffer_("", std::ios_base::app), DBus(pDBus),
	devicesManager(nullptr)
{
	LOG(DEBUG2) << "initializing clients manager";

	this->DBus->addEvent_StringToBool_Callback( this->DBus_CM_object_, this->DBus_CM_interface_, "RegisterClient",
		{	{"s", "client_session_object_path", "in", "client session object path"},
			{"b", "did_register_succeeded", "out", "did the RegisterClient method succeeded ?"},
			{"s", "failure_reason_or_client_id", "out", "if register success (bool==true), unique client ID, else (bool=false) failure reason"} },
		std::bind(&ClientsManager::registerClient, this, std::placeholders::_1) );

	this->DBus->addEvent_StringToBool_Callback( this->DBus_CM_object_, this->DBus_CM_interface_, "UnregisterClient",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"b", "did_unregister_succeeded", "out", "did the UnregisterClient method succeeded ?"} },
		std::bind(&ClientsManager::unregisterClient, this, std::placeholders::_1) );

	this->DBus->addEvent_TwoStringsToBool_Callback( this->DBus_CM_object_, this->DBus_CM_interface_, "UpdateClientState",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "client_new_state", "in", "client new state"},
			{"b", "did_updateclientstate_succeeded", "out", "did the UpdateClientState method succeeded ?"} },
		std::bind(&ClientsManager::updateClientState, this, std::placeholders::_1, std::placeholders::_2) );

	try {
		this->devicesManager = new DevicesManager();
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		throw GLogiKBadAlloc("devices manager allocation failure");
	}

	this->DBus->addEvent_TwoStringsToBool_Callback( this->DBus_DM_object_, this->DBus_DM_interface_, "StopDevice",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStartedDevices"},
			{"b", "did_stop_succeeded", "out", "did the StopDevice method succeeded ?"} },
		std::bind(&ClientsManager::stopDevice, this, std::placeholders::_1, std::placeholders::_2) );

	this->DBus->addEvent_TwoStringsToBool_Callback( this->DBus_DM_object_, this->DBus_DM_interface_, "StartDevice",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStoppedDevices"},
			{"b", "did_start_succeeded", "out", "did the StartDevice method succeeded ?"} },
		std::bind(&ClientsManager::startDevice, this, std::placeholders::_1, std::placeholders::_2) );

	this->DBus->addEvent_TwoStringsToBool_Callback( this->DBus_DM_object_, this->DBus_DM_interface_, "RestartDevice",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStartedDevices"},
			{"b", "did_restart_succeeded", "out", "did the RestartDevice method succeeded ?"} },
		std::bind(&ClientsManager::restartDevice, this, std::placeholders::_1, std::placeholders::_2) );

	this->DBus->addEvent_StringToStringsArray_Callback( this->DBus_DM_object_, this->DBus_DM_interface_, "GetStartedDevices",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"as", "array_of_strings", "out", "array of started devices ID strings"} },
		std::bind(&ClientsManager::getStartedDevices, this, std::placeholders::_1) );

	this->DBus->addEvent_StringToStringsArray_Callback( this->DBus_DM_object_, this->DBus_DM_interface_, "GetStoppedDevices",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"as", "array_of_strings", "out", "array of stopped devices ID strings"} },
		std::bind(&ClientsManager::getStoppedDevices, this, std::placeholders::_1) );

	this->DBus->addEvent_TwoStringsToStringsArray_Callback( this->DBus_DM_object_, this->DBus_DM_interface_, "GetDeviceProperties",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStartedDevices or GetStoppedDevices"},
			{"as", "array_of_strings", "out", "string array of device properties"} },
		std::bind(&ClientsManager::getDeviceProperties, this, std::placeholders::_1, std::placeholders::_2) );
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

void ClientsManager::sendSignalToClients(const std::string & signal) {
	LOG(DEBUG2) << "sending clients " << signal << " signal";
	try {
		this->DBus->initializeTargetsSignal(
			BusConnection::GKDBUS_SYSTEM,
			this->DBus_clients_name_,
			this->DBus_CSMH_object_path_,
			this->DBus_CSMH_interface_,
			signal.c_str()
		);
		this->DBus->sendTargetsSignal();
	}
	catch (const GLogiKExcept & e) {
		LOG(WARNING) << "DBus targets signal failure : " << e.what();
	}
}

void ClientsManager::runLoop(void) {
	this->devicesManager->startMonitoring(this->DBus);

	if( this->clients_.empty() ) {
		LOG(DEBUG2) << "no client, empty container";
		return;
	}

	this->sendSignalToClients("DaemonIsStopping");

	uint8_t count = 0;
	LOG(DEBUG2) << "waiting for clients to unregister ...";
	while( count++ < 10 and this->clients_.size() > 0 ) {
		this->devicesManager->checkDBusMessages();
		LOG(DEBUG3) << "sleeping for 400 ms ...";
		std::this_thread::sleep_for(std::chrono::milliseconds(400));
	}
}

const std::string ClientsManager::generateRandomClientID(void) const {
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
				const std::string reason("already registered");
				/* appending failure reason to DBus reply */
				this->DBus->appendExtraToMethodCallReply(reason);

				/* process to check if registered clients are still alives */
				for(auto & new_pair : this->clients_ ) {
					new_pair.second->uncheck();
				}

				this->sendSignalToClients("ReportYourself");

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

		/* resetting devices states first */
		if( pClient->getSessionCurrentState() == "active" )
			this->devicesManager->resetDevicesStates();

		delete pClient; pClient = nullptr;
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
		// if state == online
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

const bool ClientsManager::stopDevice(const std::string & clientID, const std::string & devID) {
	LOG(DEBUG2) << "stopDevice " << devID << " called by client " << clientID;
	try {
		Client* pClient = this->clients_.at(clientID);
		if( pClient->isAlive() ) {
			const bool ret = this->devicesManager->stopDevice(devID);
			if( ret )
				this->sendSignalToClients("SomethingChanged");
			return ret;
		}
		LOG(DEBUG3) << "stopDevice failure because client is not alive";
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("unknown client ");
		this->buffer_ << clientID << " tried to stop device " << devID;
		LOG(WARNING) << this->buffer_.str();
		syslog(LOG_WARNING, this->buffer_.str().c_str());
	}
	return false;
}

const bool ClientsManager::startDevice(const std::string & clientID, const std::string & devID) {
	LOG(DEBUG2) << "startDevice " << devID << " called by client " << clientID;
	try {
		Client* pClient = this->clients_.at(clientID);
		if( pClient->isAlive() ) {
			const bool ret = this->devicesManager->startDevice(devID);
			if( ret )
				this->sendSignalToClients("SomethingChanged");
			return ret;
		}
		LOG(DEBUG3) << "startDevice failure because client is not alive";
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("unknown client ");
		this->buffer_ << clientID << " tried to start device " << devID;
		LOG(WARNING) << this->buffer_.str();
		syslog(LOG_WARNING, this->buffer_.str().c_str());
	}
	return false;
}

const bool ClientsManager::restartDevice(const std::string & clientID, const std::string & devID) {
	LOG(DEBUG2) << "restartDevice " << devID << " called by client " << clientID;
	if( this->stopDevice(clientID, devID) ) {
#if DEBUGGING_ON
		LOG(DEBUG) << "sleeping for 1000 ms";
#endif
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		if( this->startDevice(clientID, devID) ) {
			return true;
		}

		this->buffer_.str("device restarting failure : start failed");
		LOG(ERROR) << this->buffer_.str();
		syslog(LOG_ERR, this->buffer_.str().c_str());
		return false;
	}

	this->buffer_.str("device restarting failure : stop failed");
	LOG(ERROR) << this->buffer_.str();
	syslog(LOG_ERR, this->buffer_.str().c_str());
	return false;
}

const std::vector<std::string> ClientsManager::getStartedDevices(const std::string & clientID) {
	LOG(DEBUG2) << "getStartedDevices called by client " << clientID;
	try {
		Client* pClient = this->clients_.at(clientID);
		pClient->isAlive(); /* to avoid warning */
		return this->devicesManager->getStartedDevices();
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("unknown client ");
		this->buffer_ << clientID << " tried to get started devices array";
		LOG(WARNING) << this->buffer_.str();
		syslog(LOG_WARNING, this->buffer_.str().c_str());
	}

	const std::vector<std::string> ret;
	return ret;
}

const std::vector<std::string> ClientsManager::getStoppedDevices(const std::string & clientID) {
	LOG(DEBUG2) << "getStoppedDevices called by client " << clientID;
	try {
		Client* pClient = this->clients_.at(clientID);
		pClient->isAlive(); /* to avoid warning */
		return this->devicesManager->getStoppedDevices();
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("unknown client ");
		this->buffer_ << clientID << " tried to get stopped devices array";
		LOG(WARNING) << this->buffer_.str();
		syslog(LOG_WARNING, this->buffer_.str().c_str());
	}

	const std::vector<std::string> ret;
	return ret;
}

const std::vector<std::string> ClientsManager::getDeviceProperties(const std::string & clientID, const std::string & devID) {
	LOG(DEBUG2) << "getDeviceProperties " << devID << " called by client " << clientID;
	try {
		Client* pClient = this->clients_.at(clientID);
		if( pClient->isAlive() ) {
			return this->devicesManager->getDeviceProperties(devID);
		}
		LOG(DEBUG3) << "getDeviceProperties failure because client is not alive";
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("unknown client ");
		this->buffer_ << clientID << " tried to get devices properties array";
		LOG(WARNING) << this->buffer_.str();
		syslog(LOG_WARNING, this->buffer_.str().c_str());
	}

	const std::vector<std::string> ret;
	return ret;
}

} // namespace GLogiK

