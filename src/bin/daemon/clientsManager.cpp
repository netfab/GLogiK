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
#include <functional>
#include <sstream>
#include <thread>
#include <chrono>
#include <stdexcept>

#include <config.h>

#include "lib/utils/utils.h"
#include "lib/shared/glogik.h"

#include "clientsManager.h"

namespace GLogiK
{

ClientsManager::ClientsManager(GKDBus* pDBus) : buffer_("", std::ios_base::app), DBus(pDBus),
	devicesManager(nullptr)
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "initializing clients manager";
#endif

	try {
		this->devicesManager = new DevicesManager();
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		throw GLogiKBadAlloc("devices manager allocation failure");
	}

	/* clients manager DBus object and interface */
	const auto & CM_object = GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT;
	const auto & CM_interf = GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE;

	/* devices manager DBus object and interface */
	const auto & DM_object = GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT;
	const auto & DM_interf = GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE;

	/* ClientsManager D-Bus object */

	this->DBus->addEvent_StringToBool_Callback(
		CM_object, CM_interf, "RegisterClient",
		{	{"s", "client_session_object_path", "in", "client session object path"},
			{"b", "did_register_succeeded", "out", "did the RegisterClient method succeeded ?"},
			{"s", "failure_reason_or_client_id", "out", "if register success (bool==true), unique client ID, else (bool=false) failure reason"} },
		std::bind(&ClientsManager::registerClient, this, std::placeholders::_1) );

	this->DBus->addEvent_StringToBool_Callback(
		CM_object, CM_interf, "UnregisterClient",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"b", "did_unregister_succeeded", "out", "did the UnregisterClient method succeeded ?"} },
		std::bind(&ClientsManager::unregisterClient, this, std::placeholders::_1) );

	this->DBus->addEvent_TwoStringsToBool_Callback(
		CM_object, CM_interf, "UpdateClientState",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "client_new_state", "in", "client new state"},
			{"b", "did_updateclientstate_succeeded", "out", "did the UpdateClientState method succeeded ?"} },
		std::bind(&ClientsManager::updateClientState, this, std::placeholders::_1, std::placeholders::_2) );

	this->DBus->addEvent_TwoStringsToBool_Callback(
		CM_object, CM_interf, "DeleteDeviceConfiguration",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStartedDevices or GetStoppedDevices"},
			{"b", "did_deletedeviceconfiguration_succeeded", "out", "did the DeleteDeviceConfiguration method succeeded ?"} },
		std::bind(&ClientsManager::deleteDeviceConfiguration, this, std::placeholders::_1, std::placeholders::_2) );

	/* DevicesManager D-Bus object */

	this->DBus->addEvent_TwoStringsToBool_Callback(
		DM_object, DM_interf, "StopDevice",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStartedDevices"},
			{"b", "did_stop_succeeded", "out", "did the StopDevice method succeeded ?"} },
		std::bind(&ClientsManager::stopDevice, this, std::placeholders::_1, std::placeholders::_2) );

	this->DBus->addEvent_TwoStringsToBool_Callback(
		DM_object, DM_interf, "StartDevice",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStoppedDevices"},
			{"b", "did_start_succeeded", "out", "did the StartDevice method succeeded ?"} },
		std::bind(&ClientsManager::startDevice, this, std::placeholders::_1, std::placeholders::_2) );

	this->DBus->addEvent_TwoStringsToBool_Callback(
		DM_object, DM_interf, "RestartDevice",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStartedDevices"},
			{"b", "did_restart_succeeded", "out", "did the RestartDevice method succeeded ?"} },
		std::bind(&ClientsManager::restartDevice, this, std::placeholders::_1, std::placeholders::_2) );

	this->DBus->addEvent_StringToStringsArray_Callback(
		DM_object, DM_interf, "GetStartedDevices",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"as", "array_of_strings", "out", "array of started devices ID strings"} },
		std::bind(&ClientsManager::getStartedDevices, this, std::placeholders::_1) );

	this->DBus->addEvent_StringToStringsArray_Callback(
		DM_object, DM_interf, "GetStoppedDevices",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"as", "array_of_strings", "out", "array of stopped devices ID strings"} },
		std::bind(&ClientsManager::getStoppedDevices, this, std::placeholders::_1) );

	this->DBus->addEvent_TwoStringsToStringsArray_Callback(
		DM_object, DM_interf, "GetDeviceProperties",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStartedDevices or GetStoppedDevices"},
			{"as", "array_of_strings", "out", "string array of device properties"} },
		std::bind(&ClientsManager::getDeviceProperties, this, std::placeholders::_1, std::placeholders::_2) );

	this->DBus->addEvent_TwoStringsThreeBytesToBool_Callback(
		DM_object, DM_interf, "SetDeviceBacklightColor",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStartedDevices"},
			{"y", "red_byte", "in", "red byte for the RGB color model"},
			{"y", "green_byte", "in", "green byte for the RGB color model"},
			{"y", "blue_byte", "in", "blue byte for the RGB color model"},
			{"b", "did_setcolor_succeeded", "out", "did the SetDeviceBacklightColor method succeeded ?"} },
		std::bind(&ClientsManager::setDeviceBacklightColor, this, std::placeholders::_1, std::placeholders::_2,
			std::placeholders::_3, std::placeholders::_4, std::placeholders::_5) );

	this->DBus->addEvent_ThreeStringsOneByteToMacro_Callback(
		DM_object, DM_interf, "GetMacro",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStartedDevices"},
			{"s", "macro_key_name", "in", "macro key name"},
			{"y", "macro_profile", "in", "macro profile"},
			{"a(yyq)", "macro_array", "out", "macro array"} },
		std::bind(&ClientsManager::getDeviceMacro, this, std::placeholders::_1, std::placeholders::_2,
			std::placeholders::_3, std::placeholders::_4) );

	this->DBus->addEvent_TwoStringsToStringsArray_Callback(
		DM_object, DM_interf, "GetDeviceMacroKeysNames",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStartedDevices or GetStoppedDevices"},
			{"as", "array_of_strings", "out", "string array of device macro keys names"} },
		std::bind(&ClientsManager::getDeviceMacroKeysNames, this, std::placeholders::_1, std::placeholders::_2) );
}

ClientsManager::~ClientsManager() {
#if DEBUGGING_ON
	LOG(DEBUG2) << "destroying clients manager";
#endif

	delete this->devicesManager;
	this->devicesManager = nullptr;

	for( auto & client_pair : this->clients_ ) {
		Client* pClient = client_pair.second;
		if( pClient != nullptr ) { /* sanity check */
			this->buffer_.str("destroying unfreed client : ");
			this->buffer_ << client_pair.first;
			GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
			delete pClient;
			pClient = nullptr;
		}
	}
	this->clients_.clear();

#if DEBUGGING_ON
	LOG(DEBUG2) << "exiting clients manager";
#endif
}

void ClientsManager::sendSignalToClients(const std::string & signal) {
	/* don't try to send signal if we know that there is no clients */
	if( this->clients_.size() == 0 )
		return;

#if DEBUGGING_ON
	LOG(DEBUG2) << "sending clients " << signal << " signal";
#endif
	try {
		this->DBus->initializeTargetsSignal(
			BusConnection::GKDBUS_SYSTEM,
			GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT_PATH,
			GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
			signal.c_str()
		);
		this->DBus->sendTargetsSignal();
	}
	catch (const GLogiKExcept & e) {
		this->buffer_.str("DBus targets signal failure : ");
		this->buffer_ << e.what();
		GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
	}
}

void ClientsManager::runLoop(void) {
	this->devicesManager->startMonitoring(this->DBus);

	if( this->clients_.empty() ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "no client, empty container";
#endif
		return;
	}

	this->sendSignalToClients("DaemonIsStopping");

	unsigned int c = 0;
#if DEBUGGING_ON
	LOG(DEBUG2) << "waiting for clients to unregister ...";
#endif
	while( c++ < 10 and this->clients_.size() > 0 ) { /* bonus point */
		this->devicesManager->checkDBusMessages();
#if DEBUGGING_ON
		LOG(DEBUG3) << "sleeping for 400 ms ...";
#endif
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
				GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
				this->unregisterClient(clientID);
				continue;
			}

			if( pClient->getSessionObjectPath() == clientSessionObjectPath ) {
				this->buffer_.str("client already registered : ");
				this->buffer_ << clientSessionObjectPath;
				GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
				const std::string reason("already registered");
				/* appending failure reason to DBus reply */
				this->DBus->appendExtraStringToMethodCallReply(reason);

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
		std::string clientID;

		try {
			clientID = this->generateRandomClientID();

			this->buffer_.str("registering new client with ID : ");
			this->buffer_ << clientID;
			GKSysLog(LOG_INFO, DEBUG2, this->buffer_.str());

			this->clients_[clientID] = new Client(clientSessionObjectPath, this->devicesManager);
		}
		catch (const std::bad_alloc& e) { /* handle new() failure */
			const std::string s = "new client allocation failure";
			GKSysLog(LOG_ERR, ERROR, s);
			this->DBus->appendExtraStringToMethodCallReply(s);
			return false;
		}
		catch (const std::out_of_range& oor) {
			this->buffer_.str("tried to initialize unknown client : ");
			this->buffer_ << clientID;
			GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
			this->DBus->appendExtraStringToMethodCallReply("internal error");
			return false;
		}
		catch (const GLogiKExcept & e) {
			GKSysLog(LOG_ERR, ERROR, e.what());
			this->DBus->appendExtraStringToMethodCallReply("internal error");
			return false;
		}

		this->devicesManager->setNumClients( this->clients_.size() );

		/* appending client id to DBus reply */
		this->DBus->appendExtraStringToMethodCallReply(clientID);

		return true;
	}
	return false;
}

const bool ClientsManager::unregisterClient(const std::string & clientID) {
	try {
		Client* pClient = this->clients_.at(clientID);

		this->buffer_.str("unregistering client : ");
		this->buffer_ << clientID;
		GKSysLog(LOG_INFO, DEBUG2, this->buffer_.str());

		/* resetting devices states first */
		if( pClient->getSessionCurrentState() == "active" )
			this->devicesManager->resetDevicesStates();

		delete pClient; pClient = nullptr;
		this->clients_.erase(clientID);
		this->devicesManager->setNumClients( this->clients_.size() );

		return true;
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("tried to unregister unknown client : ");
		this->buffer_ << clientID;
		GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
	}
	return false;
}

const bool ClientsManager::updateClientState(const std::string & clientID, const std::string & state) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "updating client state : " << clientID << " - " << state;
#endif
	try {
		Client* pClient = this->clients_.at(clientID);
		pClient->updateSessionState(state);

		if(state == "online") {
			this->devicesManager->resetDevicesStates();
		}
		else if(state == "active") {
			pClient->setAllDevicesBacklightColors(this->devicesManager);
		}
		else {
			this->buffer_.str("unhandled state for updating devices : ");
			this->buffer_ << state;
			GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
		}

		return true;
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("client not registered : ");
		this->buffer_ << clientID;
		GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
	}

	return false;
}

const bool ClientsManager::deleteDeviceConfiguration(const std::string & clientID, const std::string & devID) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "deleting device " << devID << " configuration for client : " << clientID;
#endif
	try {
		Client* pClient = this->clients_.at(clientID);
		return pClient->deleteDevice(devID);
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("client not registered : ");
		this->buffer_ << clientID;
		GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
	}

	return false;
}

const bool ClientsManager::stopDevice(const std::string & clientID, const std::string & devID) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "stopDevice " << devID << " called by client " << clientID;
#endif
	try {
		Client* pClient = this->clients_.at(clientID);
		if( pClient->isAlive() ) {
			const bool ret = this->devicesManager->stopDevice(devID);
			if( ret )
				this->sendSignalToClients("SomethingChanged");
			return ret;
		}
#if DEBUGGING_ON
		LOG(DEBUG3) << "stopDevice failure because client is not alive";
#endif
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("unknown client ");
		this->buffer_ << clientID << " tried to stop device " << devID;
		GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
	}
	return false;
}

const bool ClientsManager::startDevice(const std::string & clientID, const std::string & devID) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "startDevice " << devID << " called by client " << clientID;
#endif
	try {
		Client* pClient = this->clients_.at(clientID);
		if( pClient->isAlive() ) {
			const bool ret = this->devicesManager->startDevice(devID);
			if( ret )
				this->sendSignalToClients("SomethingChanged");
			return ret;
		}
#if DEBUGGING_ON
		LOG(DEBUG3) << "startDevice failure because client is not alive";
#endif
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("unknown client ");
		this->buffer_ << clientID << " tried to start device " << devID;
		GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
	}
	return false;
}

const bool ClientsManager::restartDevice(const std::string & clientID, const std::string & devID) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "restartDevice " << devID << " called by client " << clientID;
#endif

	if( this->stopDevice(clientID, devID) ) {

#if DEBUGGING_ON
		LOG(DEBUG) << "sleeping for 1000 ms";
#endif
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		if( this->startDevice(clientID, devID) ) {
			return true;
		}

		GKSysLog(LOG_ERR, ERROR, "device restarting failure : start failed");
		return false;
	}

	GKSysLog(LOG_ERR, ERROR, "device restarting failure : stop failed");
	return false;
}

const std::vector<std::string> ClientsManager::getStartedDevices(const std::string & clientID) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "getStartedDevices called by client " << clientID;
#endif
	try {
		Client* pClient = this->clients_.at(clientID);
		pClient->isAlive(); /* to avoid warning */
		return this->devicesManager->getStartedDevices();
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("unknown client ");
		this->buffer_ << clientID << " tried to get started devices array";
		GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
	}

	const std::vector<std::string> ret;
	return ret;
}

const std::vector<std::string> ClientsManager::getStoppedDevices(const std::string & clientID) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "getStoppedDevices called by client " << clientID;
#endif
	try {
		Client* pClient = this->clients_.at(clientID);
		pClient->isAlive(); /* to avoid warning */
		return this->devicesManager->getStoppedDevices();
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("unknown client ");
		this->buffer_ << clientID << " tried to get stopped devices array";
		GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
	}

	const std::vector<std::string> ret;
	return ret;
}

const std::vector<std::string> ClientsManager::getDeviceProperties(const std::string & clientID, const std::string & devID) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "getDeviceProperties " << devID << " called by client " << clientID;
#endif
	try {
		Client* pClient = this->clients_.at(clientID);
		if( pClient->isAlive() ) {
			return pClient->getDeviceProperties(devID, this->devicesManager);
		}
#if DEBUGGING_ON
		LOG(DEBUG3) << "getDeviceProperties failure because client is not alive";
#endif
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("unknown client ");
		this->buffer_ << clientID << " tried to get devices properties array";
		GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
	}

	const std::vector<std::string> ret;
	return ret;
}

const bool ClientsManager::setDeviceBacklightColor(const std::string & clientID, const std::string & devID,
	const uint8_t r, const uint8_t g, const uint8_t b)
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "setDeviceBacklightColor " << devID << " called by client " << clientID;
	LOG(DEBUG3) << "with following RGB bytes : " << std::hex << to_uint(r)
				<< " " << std::hex << to_uint(g)
				<< " " << std::hex << to_uint(b);
#endif
	try {
		Client* pClient = this->clients_.at(clientID);
		const bool ret = pClient->setDeviceBacklightColor(devID, r, g, b);
		if( pClient->getSessionCurrentState() == "active" )
			this->devicesManager->setDeviceBacklightColor(devID, r, g, b);
		return ret;
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("unknown client ");
		this->buffer_ << clientID << " tried to set device backlight color";
		GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
	}
	return false;
}

const macro_t & ClientsManager::getDeviceMacro(
	const std::string & clientID, const std::string & devID,
	const std::string & keyName, const uint8_t profile)
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "getDeviceMacro called by client " << clientID;
	LOG(DEBUG3) << " device : " << devID;
	LOG(DEBUG3) << "    key : " << keyName;
	LOG(DEBUG3) << "profile : " << to_uint(profile);
#endif
	try {
		Client* pClient = this->clients_.at(clientID);
		pClient->syncDeviceMacrosProfiles(devID, this->devicesManager->getDeviceMacrosProfiles(devID));
		return pClient->getDeviceMacro(devID, keyName, profile);
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("unknown client ");
		this->buffer_ << clientID << " tried to get macro";
		GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
	}

	return MacrosBanks::empty_macro_;
}

const std::vector<std::string> & ClientsManager::getDeviceMacroKeysNames(
	const std::string & clientID,
	const std::string & devID
) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "device " << devID << " - client " << clientID;
#endif
	try {
		this->clients_.at(clientID);
		return this->devicesManager->getDeviceMacroKeysNames(devID);
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("unknown client ");
		this->buffer_ << clientID;
		GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
	}

	return KeyboardDriver::getEmptyStringVector();
}

} // namespace GLogiK

