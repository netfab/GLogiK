/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2018  Fabrice Delliaux <netbox253@gmail.com>
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

#include "lib/utils/utils.h"
#include "lib/shared/glogik.h"

#include "clientsManager.h"

namespace GLogiK
{

using namespace NSGKUtils;

ClientsManager::ClientsManager(NSGKDBus::GKDBus* pDBus)
	:	buffer_("", std::ios_base::app),
		pDBus_(pDBus),
		devicesManager(nullptr),
		enabled_signals_(true)
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

	const NSGKDBus::BusConnection system_bus(NSGKDBus::BusConnection::GKDBUS_SYSTEM);

	/* -- -- -- -- -- -- -- -- -- -- */
	/*  ClientsManager D-Bus object  */
	/* -- -- -- -- -- -- -- -- -- -- */

	this->pDBus_->NSGKDBus::EventGKDBusCallback<StringToBool>::exposeMethod(
		system_bus, CM_object, CM_interf, "RegisterClient",
		{	{"s", "client_session_object_path", "in", "client session object path"},
			{"b", "did_register_succeeded", "out", "did the RegisterClient method succeeded ?"},
			{"s", "failure_reason_or_client_id", "out", "if register success (bool==true), unique client ID, else (bool=false) failure reason"} },
		std::bind(&ClientsManager::registerClient, this, std::placeholders::_1) );

	this->pDBus_->NSGKDBus::EventGKDBusCallback<StringToBool>::exposeMethod(
		system_bus, CM_object, CM_interf, "UnregisterClient",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"b", "did_unregister_succeeded", "out", "did the UnregisterClient method succeeded ?"} },
		std::bind(&ClientsManager::unregisterClient, this, std::placeholders::_1) );

	this->pDBus_->NSGKDBus::EventGKDBusCallback<TwoStringsToBool>::exposeMethod(
		system_bus, CM_object, CM_interf, "UpdateClientState",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "client_new_state", "in", "client new state"},
			{"b", "did_updateclientstate_succeeded", "out", "did the UpdateClientState method succeeded ?"} },
		std::bind(&ClientsManager::updateClientState, this, std::placeholders::_1, std::placeholders::_2) );

	this->pDBus_->NSGKDBus::EventGKDBusCallback<TwoStringsToBool>::exposeMethod(
		system_bus, CM_object, CM_interf, "DeleteDeviceConfiguration",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStartedDevices or GetStoppedDevices"},
			{"b", "did_deletedeviceconfiguration_succeeded", "out", "did the DeleteDeviceConfiguration method succeeded ?"} },
		std::bind(&ClientsManager::deleteDeviceConfiguration, this, std::placeholders::_1, std::placeholders::_2) );

	this->pDBus_->NSGKDBus::EventGKDBusCallback<StringToBool>::exposeMethod(
		system_bus, CM_object, CM_interf, "ToggleClientReadyPropertie",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"b", "did_method_succeeded", "out", "did the method succeeded ?"} },
		std::bind(&ClientsManager::toggleClientReadyPropertie, this, std::placeholders::_1) );

	/* -- -- -- -- -- -- -- -- -- -- */
	/*  DevicesManager D-Bus object  */
	/* -- -- -- -- -- -- -- -- -- -- */

	this->pDBus_->NSGKDBus::EventGKDBusCallback<TwoStringsToBool>::exposeMethod(
		system_bus, DM_object, DM_interf, "StopDevice",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStartedDevices"},
			{"b", "did_stop_succeeded", "out", "did the StopDevice method succeeded ?"} },
		std::bind(&ClientsManager::stopDevice, this, std::placeholders::_1, std::placeholders::_2) );

	this->pDBus_->NSGKDBus::EventGKDBusCallback<TwoStringsToBool>::exposeMethod(
		system_bus, DM_object, DM_interf, "StartDevice",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStoppedDevices"},
			{"b", "did_start_succeeded", "out", "did the StartDevice method succeeded ?"} },
		std::bind(&ClientsManager::startDevice, this, std::placeholders::_1, std::placeholders::_2) );

	this->pDBus_->NSGKDBus::EventGKDBusCallback<TwoStringsToBool>::exposeMethod(
		system_bus, DM_object, DM_interf, "RestartDevice",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStartedDevices"},
			{"b", "did_restart_succeeded", "out", "did the RestartDevice method succeeded ?"} },
		std::bind(&ClientsManager::restartDevice, this, std::placeholders::_1, std::placeholders::_2) );

	this->pDBus_->NSGKDBus::EventGKDBusCallback<ThreeStringsOneByteToMacro>::exposeMethod(
		system_bus, DM_object, DM_interf, "GetDeviceMacro",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStartedDevices"},
			{"s", "macro_key_name", "in", "macro key name"},
			{"y", "macro_profile", "in", "macro profile"},
			{"a(yyq)", "macro_array", "out", "macro array"} },
		std::bind(&ClientsManager::getDeviceMacro, this, std::placeholders::_1, std::placeholders::_2,
			std::placeholders::_3, std::placeholders::_4) );

		/* -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
		/* methods used to initialize devices on service-side */
		/* -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */

	this->pDBus_->NSGKDBus::EventGKDBusCallback<StringToStringsArray>::exposeMethod(
		system_bus, DM_object, DM_interf, "GetStartedDevices",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"as", "array_of_strings", "out", "array of started devices ID strings"} },
		std::bind(&ClientsManager::getStartedDevices, this, std::placeholders::_1) );

	this->pDBus_->NSGKDBus::EventGKDBusCallback<StringToStringsArray>::exposeMethod(
		system_bus, DM_object, DM_interf, "GetStoppedDevices",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"as", "array_of_strings", "out", "array of stopped devices ID strings"} },
		std::bind(&ClientsManager::getStoppedDevices, this, std::placeholders::_1) );

	this->pDBus_->NSGKDBus::EventGKDBusCallback<TwoStringsToString>::exposeMethod(
		system_bus, DM_object, DM_interf, "GetDeviceStatus",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID"},
			{"s", "device status", "out", "string representing the device status"} },
		std::bind(&ClientsManager::getDeviceStatus, this, std::placeholders::_1, std::placeholders::_2) );

	this->pDBus_->NSGKDBus::EventGKDBusCallback<TwoStringsToStringsArray>::exposeMethod(
		system_bus, DM_object, DM_interf, "GetDeviceProperties",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStartedDevices or GetStoppedDevices"},
			{"as", "array_of_strings", "out", "string array of device properties"} },
		std::bind(&ClientsManager::getDeviceProperties, this, std::placeholders::_1, std::placeholders::_2) );

	this->pDBus_->NSGKDBus::EventGKDBusCallback<TwoStringsToStringsArray>::exposeMethod(
		system_bus, DM_object, DM_interf, "GetDeviceMacroKeysNames",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStartedDevices or GetStoppedDevices"},
			{"as", "array_of_strings", "out", "string array of device macro keys names"} },
		std::bind(&ClientsManager::getDeviceMacroKeysNames, this, std::placeholders::_1, std::placeholders::_2) );

	this->pDBus_->NSGKDBus::EventGKDBusCallback<TwoStringsThreeBytesToBool>::exposeMethod(
		system_bus, DM_object, DM_interf, "SetDeviceBacklightColor",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStartedDevices"},
			{"y", "red_byte", "in", "red byte for the RGB color model"},
			{"y", "green_byte", "in", "green byte for the RGB color model"},
			{"y", "blue_byte", "in", "blue byte for the RGB color model"},
			{"b", "did_setcolor_succeeded", "out", "did the SetDeviceBacklightColor method succeeded ?"} },
		std::bind(&ClientsManager::setDeviceBacklightColor, this, std::placeholders::_1, std::placeholders::_2,
			std::placeholders::_3, std::placeholders::_4, std::placeholders::_5) );

	this->pDBus_->NSGKDBus::EventGKDBusCallback<TwoStringsOneByteOneMacrosBankToBool>::exposeMethod(
		system_bus, DM_object, DM_interf, "SetDeviceMacrosBank",
		{	{"s", "client_unique_id", "in", "must be a valid client ID"},
			{"s", "device_id", "in", "device ID coming from GetStartedDevices"},
			{"y", "macro_profile", "in", "macro profile"},
			{"a(sya(yyq))", "macros_bank", "in", "macros bank"},
			{"b", "did_setbank_succeeded", "out", "did the SetDeviceMacrosBank method succeeded ?"} },
		std::bind(&ClientsManager::setDeviceMacrosBank, this, std::placeholders::_1, std::placeholders::_2,
			std::placeholders::_3, std::placeholders::_4) );
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

void ClientsManager::runLoop(void) {
	this->devicesManager->startMonitoring(this->pDBus_);

	if( this->clients_.empty() ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "no client, empty container";
#endif
		return;
	}

	this->sendSignalToClients(this->clients_.size(), this->pDBus_, "DaemonIsStopping");

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

const bool ClientsManager::registerClient(
	const std::string & clientSessionObjectPath
)	{
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
				this->pDBus_->appendExtraStringToMessage(reason);

				/* process to check if registered clients are still alives */
				for(auto & new_pair : this->clients_ ) {
					new_pair.second->uncheck();
				}

				this->sendSignalToClients(this->clients_.size(), this->pDBus_, "ReportYourself");

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
			this->pDBus_->appendExtraStringToMessage(s);
			return false;
		}
		catch (const std::out_of_range& oor) {
			this->buffer_.str("tried to initialize unknown client : ");
			this->buffer_ << clientID;
			GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
			this->pDBus_->appendExtraStringToMessage("internal error");
			return false;
		}
		catch (const GLogiKExcept & e) {
			GKSysLog(LOG_ERR, ERROR, e.what());
			this->pDBus_->appendExtraStringToMessage("internal error");
			return false;
		}

		this->devicesManager->setNumClients( this->clients_.size() );

		/* appending client id to DBus reply */
		this->pDBus_->appendExtraStringToMessage(clientID);

		return true;
	}
	return false;
}

const bool ClientsManager::unregisterClient(
	const std::string & clientID
)	{
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
		GKSysLog_UnknownClient
	}
	return false;
}

const bool ClientsManager::updateClientState(
	const std::string & clientID,
	const std::string & state
)	{
#if DEBUGGING_ON
	LOG(DEBUG2) << s_Client << clientID << " state: " << state;
#endif
	try {
		Client* pClient = this->clients_.at(clientID);
		pClient->updateSessionState(state);

		if(state == "online") {
			this->devicesManager->resetDevicesStates();
		}
		else if(state == "active") {
			pClient->setAllDevicesBacklightColors(this->devicesManager);
			pClient->setAllDevicesMacrosProfiles(this->devicesManager);
		}
		else {
			this->buffer_.str("unhandled state for updating devices : ");
			this->buffer_ << state;
			GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
		}

		return true;
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}

	return false;
}

const bool ClientsManager::toggleClientReadyPropertie(
	const std::string & clientID
)	{
	try {
		Client* pClient = this->clients_.at(clientID);
		pClient->toggleClientReadyPropertie();
		if( pClient->isReady() ) {
			if(pClient->getSessionCurrentState() == "active") {
				pClient->setAllDevicesBacklightColors(this->devicesManager);
				pClient->setAllDevicesMacrosProfiles(this->devicesManager);
			}
		}
		return true;
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}
	return false;
}

const bool ClientsManager::deleteDeviceConfiguration(
	const std::string & clientID,
	const std::string & devID
)	{
#if DEBUGGING_ON
	LOG(DEBUG2) << s_Device << devID << " " << s_Client << clientID;
#endif
	try {
		Client* pClient = this->clients_.at(clientID);
		return pClient->deleteDevice(devID);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}

	return false;
}

const bool ClientsManager::stopDevice(
	const std::string & clientID,
	const std::string & devID
)	{
#if DEBUGGING_ON
	LOG(DEBUG2) << s_Device << devID << " " << s_Client << clientID;
#endif
	try {
		Client* pClient = this->clients_.at(clientID);

		if( ! pClient->isReady() ) {
			GKSysLog(LOG_WARNING, WARNING, "device state change not allowed while client not ready");
			return false;
		}

		if( ! pClient->isAlive() ) {
			GKSysLog(LOG_WARNING, WARNING, "device state change not allowed because client not alive");
			return false;
		}

		const bool ret = this->devicesManager->stopDevice(devID);
		if(ret and this->enabled_signals_) {
			const std::vector<std::string> array = {devID};
			this->sendStatusSignalArrayToClients(this->clients_.size(), this->pDBus_, "DevicesStopped", array);
		}
		return ret;
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}
	return false;
}

const bool ClientsManager::startDevice(
	const std::string & clientID,
	const std::string & devID
)	{
#if DEBUGGING_ON
	LOG(DEBUG2) << s_Device << devID << " " << s_Client << clientID;
#endif
	try {
		Client* pClient = this->clients_.at(clientID);

		if( ! pClient->isReady() ) {
			GKSysLog(LOG_WARNING, WARNING, "device state change not allowed while client not ready");
			return false;
		}

		if( ! pClient->isAlive() ) {
			GKSysLog(LOG_WARNING, WARNING, "device state change not allowed because client not alive");
			return false;
		}

		const bool ret = this->devicesManager->startDevice(devID);
		if(ret and this->enabled_signals_) {
			const std::vector<std::string> array = {devID};
			this->sendStatusSignalArrayToClients(this->clients_.size(), this->pDBus_, "DevicesStarted", array);
		}
		return ret;
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}
	return false;
}

const bool ClientsManager::restartDevice(
	const std::string & clientID,
	const std::string & devID
)	{
#if DEBUGGING_ON
	LOG(DEBUG2) << s_Device << devID << " " << s_Client << clientID;
#endif

	this->enabled_signals_ = false;
	const std::vector<std::string> array = {devID};

	if( this->stopDevice(clientID, devID) ) {

#if DEBUGGING_ON
		LOG(DEBUG) << "sleeping for 1000 ms";
#endif
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		if( this->startDevice(clientID, devID) ) {
			this->enabled_signals_ = true;
			this->sendStatusSignalArrayToClients(this->clients_.size(), this->pDBus_, "DevicesStarted", array);
			return true;
		}

		this->enabled_signals_ = true;
		this->sendStatusSignalArrayToClients(this->clients_.size(), this->pDBus_, "DevicesStopped", array);
		GKSysLog(LOG_ERR, ERROR, "device restarting failure : start failed");
		return false;
	}

	this->enabled_signals_ = true;
	// FIXME
	// device could fail to stop for following reasons :
	//  * unknown clientID
	//  * client not allowed to stop device (not ready)
	//  * client not allowed to stop device (not alive)
	//  * devID not found by deviceManager in container
	//  * device found but driver not found (unlikely)
	// should send signal to clients to tell them to check device status
	GKSysLog(LOG_ERR, ERROR, "device restarting failure : stop failed");
	return false;
}

const std::vector<std::string> ClientsManager::getStartedDevices(
	const std::string & clientID
)	{
#if DEBUGGING_ON
	LOG(DEBUG2) << s_Client << clientID;
#endif
	try {
		Client* pClient = this->clients_.at(clientID);
		pClient->isAlive(); /* to avoid warning */
		return this->devicesManager->getStartedDevices();
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}

	const std::vector<std::string> ret;
	return ret;
}

const std::vector<std::string> ClientsManager::getStoppedDevices(
	const std::string & clientID
)	{
#if DEBUGGING_ON
	LOG(DEBUG2) << s_Client << clientID;
#endif
	try {
		Client* pClient = this->clients_.at(clientID);
		pClient->isAlive(); /* to avoid warning */
		return this->devicesManager->getStoppedDevices();
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}

	const std::vector<std::string> ret;
	return ret;
}

const std::string ClientsManager::getDeviceStatus(
	const std::string & clientID,
	const std::string & devID
)	{
#if DEBUGGING_ON
	LOG(DEBUG2) << s_Device << devID << " " << s_Client << clientID;
#endif
	try {
		this->clients_.at(clientID);
		return this->devicesManager->getDeviceStatus(devID);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}

	return "unknown";
}

const std::vector<std::string> ClientsManager::getDeviceProperties(
	const std::string & clientID,
	const std::string & devID
)	{
#if DEBUGGING_ON
	LOG(DEBUG2) << s_Device << devID << " " << s_Client << clientID;
#endif
	try {
		Client* pClient = this->clients_.at(clientID);
		if( pClient->isAlive() ) {
			return pClient->getDeviceProperties(devID, this->devicesManager);
		}
		GKSysLog(LOG_WARNING, WARNING, "getting device properties not allowed because client not alive");
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}

	const std::vector<std::string> ret;
	return ret;
}

const bool ClientsManager::setDeviceBacklightColor(
	const std::string & clientID,
	const std::string & devID,
	const uint8_t r,
	const uint8_t g,
	const uint8_t b
)	{
#if DEBUGGING_ON
	LOG(DEBUG2) << s_Device << devID << " " << s_Client << clientID;
	LOG(DEBUG3) << "with following RGB bytes : " << std::hex << to_uint(r)
				<< " " << std::hex << to_uint(g)
				<< " " << std::hex << to_uint(b);
#endif
	try {
		Client* pClient = this->clients_.at(clientID);
		return pClient->setDeviceBacklightColor(devID, r, g, b);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}
	return false;
}

const macro_t & ClientsManager::getDeviceMacro(
	const std::string & clientID,
	const std::string & devID,
	const std::string & keyName,
	const uint8_t profile
)	{
#if DEBUGGING_ON
	LOG(DEBUG2) << s_Device << devID << " " << s_Client << clientID;
	LOG(DEBUG3) << "    key : " << keyName;
	LOG(DEBUG3) << "profile : " << to_uint(profile);
#endif
	try {
		Client* pClient = this->clients_.at(clientID);

		if( pClient->isReady() ) {
			pClient->syncDeviceMacrosProfiles(devID, this->devicesManager->getDeviceMacrosProfiles(devID));
			return pClient->getDeviceMacro(devID, keyName, profile);
		}

		GKSysLog(LOG_WARNING, WARNING, "getting device macro not allowed while client not ready");
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}

	return MacrosBanks::empty_macro_;
}

const std::vector<std::string> & ClientsManager::getDeviceMacroKeysNames(
	const std::string & clientID,
	const std::string & devID
)	{
#if DEBUGGING_ON
	LOG(DEBUG2) << s_Device << devID << " " << s_Client << clientID;
#endif
	try {
		this->clients_.at(clientID);
		return this->devicesManager->getDeviceMacroKeysNames(devID);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}

	return KeyboardDriver::getEmptyStringVector();
}

const bool ClientsManager::setDeviceMacrosBank(
	const std::string & clientID,
	const std::string & devID,
	const uint8_t profile,
	const macros_bank_t & bank
)	{
#if DEBUGGING_ON
	LOG(DEBUG2) << s_Device << devID << " " << s_Client << clientID;
	LOG(DEBUG3) << "profile : " << to_uint(profile);
#endif
	try {
		Client* pClient = this->clients_.at(clientID);
		return pClient->setDeviceMacrosBank(devID, profile, bank);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}

	return false;
}

} // namespace GLogiK

