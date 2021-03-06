/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2020  Fabrice Delliaux <netbox253@gmail.com>
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

#include "lib/utils/utils.hpp"
#include "lib/shared/glogik.hpp"

#include "clientsManager.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

ClientsManager::ClientsManager(DevicesManager* pDevicesManager)
	:	_pDBus(nullptr),
		_pDevicesManager(pDevicesManager),
		_active("active"),
		_numActive(0),
		_enabledSignals(true)
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "initializing clients manager";
#endif
}

ClientsManager::~ClientsManager() {
#if DEBUGGING_ON
	LOG(DEBUG2) << "destroying clients manager";
#endif

	for( auto & clientPair : _connectedClients ) {
		Client* pClient = clientPair.second;
		if( pClient != nullptr ) { /* sanity check */
			std::ostringstream buffer(std::ios_base::app);
			buffer << "destroying unfreed client : " << clientPair.first;
			GKSysLog(LOG_WARNING, WARNING, buffer.str());
			delete pClient; pClient = nullptr;
		}
	}
	_connectedClients.clear();

#if DEBUGGING_ON
	LOG(DEBUG2) << "exiting clients manager";
#endif
}

void ClientsManager::initializeDBusRequests(NSGKDBus::GKDBus* pDBus)
{
	_pDBus = pDBus;

	/* clients manager DBus object and interface */
	const auto & CM_object = GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT;
	const auto & CM_interf = GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE;

	/* devices manager DBus object and interface */
	const auto & DM_object = GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT;
	const auto & DM_interf = GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE;

	const NSGKDBus::BusConnection system_bus(NSGKDBus::BusConnection::GKDBUS_SYSTEM);

	const std::string dIN("in");	/* direction in */
	const std::string dOUT("out");	/* direction out */

	/* -- -- -- -- -- -- -- -- -- -- */
	/*  ClientsManager D-Bus object  */
	/* -- -- -- -- -- -- -- -- -- -- */

	_pDBus->NSGKDBus::EventGKDBusCallback<StringToBool>::exposeMethod(
		system_bus, CM_object, CM_interf, "RegisterClient",
		{	{"s", "client_session_object_path", dIN, "client session object path"},
			{"b", "did_register_succeeded", dOUT, "did the RegisterClient method succeeded ?"},
			{"s", "failure_reason_or_client_id", dOUT, "if register success (bool==true), unique client ID, else (bool=false) failure reason"} },
		std::bind(&ClientsManager::registerClient, this, std::placeholders::_1) );

	_pDBus->NSGKDBus::EventGKDBusCallback<StringToBool>::exposeMethod(
		system_bus, CM_object, CM_interf, "UnregisterClient",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"b", "did_unregister_succeeded", dOUT, "did the UnregisterClient method succeeded ?"} },
		std::bind(&ClientsManager::unregisterClient, this, std::placeholders::_1) );

	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsToBool>::exposeMethod(
		system_bus, CM_object, CM_interf, "UpdateClientState",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "client_new_state", dIN, "client new state"},
			{"b", "did_updateclientstate_succeeded", dOUT, "did the UpdateClientState method succeeded ?"} },
		std::bind(&ClientsManager::updateClientState, this, std::placeholders::_1, std::placeholders::_2) );

	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsToBool>::exposeMethod(
		system_bus, CM_object, CM_interf, "DeleteDeviceConfiguration",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStartedDevices or GetStoppedDevices"},
			{"b", "did_deletedeviceconfiguration_succeeded", dOUT, "did the DeleteDeviceConfiguration method succeeded ?"} },
		std::bind(&ClientsManager::deleteDeviceConfiguration, this, std::placeholders::_1, std::placeholders::_2) );

	_pDBus->NSGKDBus::EventGKDBusCallback<StringToBool>::exposeMethod(
		system_bus, CM_object, CM_interf, "ToggleClientReadyPropertie",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"b", "did_method_succeeded", dOUT, "did the method succeeded ?"} },
		std::bind(&ClientsManager::toggleClientReadyPropertie, this, std::placeholders::_1) );

	/* -- -- -- -- -- -- -- -- -- -- */
	/*  DevicesManager D-Bus object  */
	/* -- -- -- -- -- -- -- -- -- -- */

	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsToBool>::exposeMethod(
		system_bus, DM_object, DM_interf, "StopDevice",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStartedDevices"},
			{"b", "did_stop_succeeded", dOUT, "did the StopDevice method succeeded ?"} },
		std::bind(&ClientsManager::stopDevice, this, std::placeholders::_1, std::placeholders::_2) );

	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsToBool>::exposeMethod(
		system_bus, DM_object, DM_interf, "StartDevice",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStoppedDevices"},
			{"b", "did_start_succeeded", dOUT, "did the StartDevice method succeeded ?"} },
		std::bind(&ClientsManager::startDevice, this, std::placeholders::_1, std::placeholders::_2) );

	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsToBool>::exposeMethod(
		system_bus, DM_object, DM_interf, "RestartDevice",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStartedDevices"},
			{"b", "did_restart_succeeded", dOUT, "did the RestartDevice method succeeded ?"} },
		std::bind(&ClientsManager::restartDevice, this, std::placeholders::_1, std::placeholders::_2) );

	_pDBus->NSGKDBus::EventGKDBusCallback<ThreeStringsOneByteToMacro>::exposeMethod(
		system_bus, DM_object, DM_interf, "GetDeviceMacro",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStartedDevices"},
			{"s", "macro_key_name", dIN, "macro key name"},
			{"y", "macro_bankID", dIN, "macro bankID"},
			{"a(yyq)", "macro_array", dOUT, "macro array"} },
		std::bind(&ClientsManager::getDeviceMacro, this, std::placeholders::_1, std::placeholders::_2,
			std::placeholders::_3, std::placeholders::_4) );

		/* -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
		/* methods used to initialize devices on service-side */
		/* -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */

	_pDBus->NSGKDBus::EventGKDBusCallback<StringToStringsArray>::exposeMethod(
		system_bus, DM_object, DM_interf, "GetStartedDevices",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"as", "array_of_strings", dOUT, "array of started devices ID strings"} },
		std::bind(&ClientsManager::getStartedDevices, this, std::placeholders::_1) );

	_pDBus->NSGKDBus::EventGKDBusCallback<StringToStringsArray>::exposeMethod(
		system_bus, DM_object, DM_interf, "GetStoppedDevices",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"as", "array_of_strings", dOUT, "array of stopped devices ID strings"} },
		std::bind(&ClientsManager::getStoppedDevices, this, std::placeholders::_1) );

	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsToString>::exposeMethod(
		system_bus, DM_object, DM_interf, "GetDeviceStatus",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID"},
			{"s", "device status", dOUT, "string representing the device status"} },
		std::bind(&ClientsManager::getDeviceStatus, this, std::placeholders::_1, std::placeholders::_2) );

	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsToVoid>::exposeMethod(
		system_bus, DM_object, DM_interf, "GetDeviceProperties",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStartedDevices or GetStoppedDevices"},
			{"sst", "get_device_properties", dOUT, "device properties"} },
		std::bind(&ClientsManager::getDeviceProperties, this, std::placeholders::_1, std::placeholders::_2) );

	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsToLCDPluginsPropertiesArray>::exposeMethod(
		system_bus, DM_object, DM_interf, "GetDeviceLCDPluginsProperties",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStartedDevices or GetStoppedDevices"},
			{"a(tss)", "get_lcd_plugins_properties_array", dOUT, "LCDPluginsProperties array"} },
		std::bind(&ClientsManager::getDeviceLCDPluginsProperties, this, std::placeholders::_1, std::placeholders::_2) );

	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsToStringsArray>::exposeMethod(
		system_bus, DM_object, DM_interf, "GetDeviceMacroKeysNames",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStartedDevices or GetStoppedDevices"},
			{"as", "array_of_strings", dOUT, "string array of device macro keys names"} },
		std::bind(&ClientsManager::getDeviceMacroKeysNames, this, std::placeholders::_1, std::placeholders::_2) );

	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsThreeBytesToBool>::exposeMethod(
		system_bus, DM_object, DM_interf, "SetDeviceBacklightColor",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStartedDevices"},
			{"y", "red_byte", dIN, "red byte for the RGB color model"},
			{"y", "green_byte", dIN, "green byte for the RGB color model"},
			{"y", "blue_byte", dIN, "blue byte for the RGB color model"},
			{"b", "did_setcolor_succeeded", dOUT, "did the SetDeviceBacklightColor method succeeded ?"} },
		std::bind(&ClientsManager::setDeviceBacklightColor, this, std::placeholders::_1, std::placeholders::_2,
			std::placeholders::_3, std::placeholders::_4, std::placeholders::_5) );

	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsOneByteOneMacrosBankToBool>::exposeMethod(
		system_bus, DM_object, DM_interf, "SetDeviceMacrosBank",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStartedDevices"},
			{"y", "macro_bankID", dIN, "macro bankID"},
			{"a(sya(yyq))", "macros_bank", dIN, "macros bank"},
			{"b", "did_setbank_succeeded", dOUT, "did the SetDeviceMacrosBank method succeeded ?"} },
		std::bind(&ClientsManager::setDeviceMacrosBank, this, std::placeholders::_1, std::placeholders::_2,
			std::placeholders::_3, std::placeholders::_4) );

	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsOneByteToBool>::exposeMethod(
		system_bus, DM_object, DM_interf, "ResetDeviceMacrosBank",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStartedDevices"},
			{"y", "macro_bankID", dIN, "macro bankID"},
			{"b", "did_resetbank_succeeded", dOUT, "did the ResetDeviceMacrosBank method succeeded ?"} },
		std::bind(&ClientsManager::resetDeviceMacrosBank, this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) );

	_pDBus->NSGKDBus::EventGKDBusCallback<TwoStringsOneByteOneUInt64ToBool>::exposeMethod(
		system_bus, DM_object, DM_interf, "SetDeviceLCDPluginsMask",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStartedDevices"},
			{"y", "LCD_Plugins_Mask_ID", dIN, "LCD plugins mask ID"},
			{"t", "LCD_Plugins_Mask", dIN, "LCD plugins mask"},
			{"b", "did_setmask_succeeded", dOUT, "did the SetDeviceLCDPluginsMask method succeeded ?"} },
		std::bind(&ClientsManager::setDeviceLCDPluginsMask, this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4) );

	/* -- -- -- -- -- -- -- -- -- -- -- -- -- */
	/*  declaration of introspectable signals */
	/*       potentially sent by daemon       */
	/* -- -- -- -- -- -- -- -- -- -- -- -- -- */

	/*  ClientsManager D-Bus object  */

	_pDBus->declareIntrospectableSignal(system_bus, CM_object, CM_interf, "DaemonIsStopping", {});
	_pDBus->declareIntrospectableSignal(system_bus, CM_object, CM_interf, "DaemonIsStarting", {});
	_pDBus->declareIntrospectableSignal(system_bus, CM_object, CM_interf, "ReportYourself",   {});

	/*  DevicesManager D-Bus object  */

	_pDBus->declareIntrospectableSignal(
		system_bus, DM_object, DM_interf, "DevicesStarted",
		{	{"as", "", dOUT, "array of started devices ID strings"} }
	);

	_pDBus->declareIntrospectableSignal(
		system_bus, DM_object, DM_interf, "DevicesStopped",
		{	{"as", "", dOUT, "array of stopped devices ID strings"} }
	);

	_pDBus->declareIntrospectableSignal(
		system_bus, DM_object, DM_interf, "DevicesUnplugged",
		{	{"as", "", dOUT, "array of unplugged devices ID strings"} }
	);

	_pDBus->declareIntrospectableSignal(
		system_bus, DM_object, DM_interf, "MacroRecorded",
		{	{"s", "device_id", dOUT, "device ID"},
			{"s", "macro_key_name", dOUT, "macro key name"},
			{"y", "macro_bankID", dOUT, "macro bankID"} }
	);

	_pDBus->declareIntrospectableSignal(
		system_bus, DM_object, DM_interf, "MacroCleared",
		{	{"s", "device_id", dOUT, "device ID"},
			{"s", "macro_key_name", dOUT, "macro key name"},
			{"y", "macro_bankID", dOUT, "macro bankID"} }
	);

	_pDBus->declareIntrospectableSignal(
		system_bus, DM_object, DM_interf, "deviceMediaEvent",
		{	{"s", "device_id", "in", "device ID"},
			{"s", "media_key_event", "in", "media key event"} }
	);
}

void ClientsManager::cleanDBusRequests(void) noexcept
{
	/* clients manager DBus object and interface */
	const auto & CM_object = GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT;
	const auto & CM_interf = GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE;

	/* devices manager DBus object and interface */
	const auto & DM_object = GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT;
	const auto & DM_interf = GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE;

	const NSGKDBus::BusConnection system_bus(NSGKDBus::BusConnection::GKDBUS_SYSTEM);

	_pDBus->removeMethodsInterface(system_bus, CM_object, CM_interf);
	_pDBus->removeMethodsInterface(system_bus, DM_object, DM_interf);
}

void ClientsManager::waitForClientsDisconnections(void) noexcept {
	if( _connectedClients.empty() ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "no client, empty container";
#endif
		return;
	}

	this->sendSignalToClients(_connectedClients.size(), _pDBus, "DaemonIsStopping");

	uint16_t c = 0;
#if DEBUGGING_ON
	LOG(DEBUG2) << "waiting for clients to unregister ...";
#endif
	while( c++ < 40 and _connectedClients.size() > 0 ) { /* bonus point */
		_pDevicesManager->checkDBusMessages();
#if DEBUGGING_ON
		LOG(DEBUG3) << "sleeping for 40 ms ...";
#endif
		std::this_thread::sleep_for(std::chrono::milliseconds(40));
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
		std::vector<std::string> toUnregister;

		for(const auto & clientPair : _connectedClients ) {
			const std::string & clientID = clientPair.first;
			Client* pClient = clientPair.second;

			/* unresponsive client */
			if( ! pClient->isAlive() ) {
				toUnregister.push_back(clientID);
				continue;
			}

			/* this client is already registered, sending signal to check all clients */
			if( pClient->getSessionObjectPath() == clientSessionObjectPath ) {
				std::ostringstream buffer(std::ios_base::app);
				buffer << "client already registered : " << clientSessionObjectPath;
				GKSysLog(LOG_WARNING, WARNING, buffer.str());

				/* appending failure reason to DBus reply */
				_pDBus->appendAsyncString("already registered");

				/* process to check if registered clients are still alives, registered
				 * clients will have 5 seconds to update their session state, else they
				 * will be considered as crashed on next registerClient call */
				for(auto & newPair : _connectedClients ) {
					newPair.second->uncheck();
				}
				this->sendSignalToClients(_connectedClients.size(), _pDBus, "ReportYourself");

				/* register failure, sender should wait and retry */
				return false;
			}
		}

		/* unregister crashed clients */
		for(const auto & clientID : toUnregister ) {
			std::ostringstream buffer(std::ios_base::app);
			buffer << "unregistering lost client (maybe crashed) with ID : " << clientID;
			GKSysLog(LOG_WARNING, WARNING, buffer.str());
			this->unregisterClient(clientID);
		}
		toUnregister.clear();

		throw std::out_of_range("not found");
	}
	catch (const std::out_of_range& oor) {
		std::string clientID;

		try {
			clientID = this->generateRandomClientID();

			std::ostringstream buffer(std::ios_base::app);
			buffer << "registering new client with ID : " << clientID;
			GKSysLog(LOG_INFO, DEBUG2, buffer.str());

			_connectedClients[clientID] = new Client(clientSessionObjectPath, _pDevicesManager);
		}
		catch (const std::bad_alloc& e) { /* handle new() failure */
			const std::string s = "new client allocation failure";
			GKSysLog(LOG_ERR, ERROR, s);
			_pDBus->appendAsyncString(s);
			return false;
		}
		catch (const std::out_of_range& oor) {
			std::ostringstream buffer(std::ios_base::app);
			buffer << "tried to initialize unknown client : " << clientID;
			GKSysLog(LOG_ERR, ERROR, buffer.str());

			_pDBus->appendAsyncString("internal error");
			return false;
		}
		catch (const GLogiKExcept & e) {
			GKSysLog(LOG_ERR, ERROR, e.what());
			_pDBus->appendAsyncString("internal error");
			return false;
		}

		_pDevicesManager->setNumClients( _connectedClients.size() );

		/* appending client id to DBus reply */
		_pDBus->appendAsyncString(clientID);
		_pDBus->appendAsyncString(VERSION);

		return true;
	}
	return false;
}

const bool ClientsManager::unregisterClient(
	const std::string & clientID
)	{
	try {
		Client* pClient = _connectedClients.at(clientID);

		std::ostringstream buffer(std::ios_base::app);
		buffer << "unregistering client : " << clientID;
		GKSysLog(LOG_INFO, DEBUG2, buffer.str());

		/* resetting devices states first */
		if( pClient->getSessionCurrentState() == _active ) {
			_pDevicesManager->resetDevicesStates();
#if DEBUGGING_ON
			LOG(DEBUG3) << "decreasing active users # : " << _numActive;
#endif
			_numActive--;
		}

		delete pClient; pClient = nullptr;
		_connectedClients.erase(clientID);
		_pDevicesManager->setNumClients( _connectedClients.size() );

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
	LOG(DEBUG2) << CONST_STRING_CLIENT << clientID << " state: " << state;
#endif

	if( (state != _active) and (state != "online") ) {
		std::ostringstream buffer(std::ios_base::app);
		buffer << "unhandled state for updating devices : " << state;
		GKSysLog(LOG_WARNING, WARNING, buffer.str());
		return false;
	}

	try {
		Client* pClient = _connectedClients.at(clientID);
		const std::string oldState( pClient->getSessionCurrentState() );
		pClient->updateSessionState(state);

		if( (oldState == _active) and (state != _active) ) {
#if DEBUGGING_ON
			LOG(DEBUG3) << "decreasing active users # : " << _numActive;
#endif
			_numActive--;
		}

		if(state == _active) {
			if(oldState != _active) {
#if DEBUGGING_ON
				LOG(DEBUG3) << "increasing active users # : " << _numActive;
#endif
				_numActive++;
			}

			if( pClient->isReady() ) {
#if DEBUGGING_ON
				LOG(DEBUG1) << "setting active user's parameters for all started devices";
#endif
				for(const auto & devID : _pDevicesManager->getStartedDevices()) {
					pClient->setDeviceActiveUser(devID, _pDevicesManager);
				}
			}
		}

#if DEBUGGING_ON
		LOG(DEBUG3) << "active users # : " << _numActive;
#endif

		if(_numActive == 0) {
			_pDevicesManager->resetDevicesStates();
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
		Client* pClient = _connectedClients.at(clientID);
		pClient->toggleClientReadyPropertie();
		if( pClient->isReady() ) {
			if(pClient->getSessionCurrentState() == _active) {
#if DEBUGGING_ON
				LOG(DEBUG1) << "setting active user's parameters for all started devices";
#endif
				for(const auto & devID : _pDevicesManager->getStartedDevices()) {
					pClient->setDeviceActiveUser(devID, _pDevicesManager);
				}
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
	LOG(DEBUG2)	<< CONST_STRING_DEVICE << devID << " "
				<< CONST_STRING_CLIENT << clientID;
#endif
	try {
		Client* pClient = _connectedClients.at(clientID);
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
	LOG(DEBUG2)	<< CONST_STRING_DEVICE << devID << " "
				<< CONST_STRING_CLIENT << clientID;
#endif
	try {
		Client* pClient = _connectedClients.at(clientID);

		if( ! pClient->isReady() ) {
			GKSysLog(LOG_WARNING, WARNING, "device state change not allowed while client not ready");
			return false;
		}

		if( ! pClient->isAlive() ) {
			GKSysLog(LOG_WARNING, WARNING, "device state change not allowed because client not alive");
			return false;
		}

		if(pClient->getSessionCurrentState() != _active) {
			GKSysLog(LOG_WARNING, WARNING, "only active user can change device state");
			return false;
		}

		const bool ret = _pDevicesManager->stopDevice(devID);
		if(ret and _enabledSignals) {
			const std::vector<std::string> array = {devID};
			this->sendStatusSignalArrayToClients(_connectedClients.size(), _pDBus, "DevicesStopped", array);
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
	LOG(DEBUG2)	<< CONST_STRING_DEVICE << devID << " "
				<< CONST_STRING_CLIENT << clientID;
#endif
	try {
		Client* pClient = _connectedClients.at(clientID);

		if( ! pClient->isReady() ) {
			GKSysLog(LOG_WARNING, WARNING, "device state change not allowed while client not ready");
			return false;
		}

		if( ! pClient->isAlive() ) {
			GKSysLog(LOG_WARNING, WARNING, "device state change not allowed because client not alive");
			return false;
		}

		if(pClient->getSessionCurrentState() != _active) {
			GKSysLog(LOG_WARNING, WARNING, "only active user can change device state");
			return false;
		}

		const bool ret = _pDevicesManager->startDevice(devID);
		if( ret ) {
			/* enable user configuration */
			pClient->setDeviceActiveUser(devID, _pDevicesManager);

			if( _enabledSignals ) {
				const std::vector<std::string> array = {devID};
				this->sendStatusSignalArrayToClients(_connectedClients.size(), _pDBus, "DevicesStarted", array);
			}
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
	LOG(DEBUG2)	<< CONST_STRING_DEVICE << devID << " "
				<< CONST_STRING_CLIENT << clientID;
#endif

	_enabledSignals = false;
	const std::vector<std::string> array = {devID};

	if( this->stopDevice(clientID, devID) ) {

#if DEBUGGING_ON
		LOG(DEBUG) << "sleeping for 1000 ms";
#endif
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		if( this->startDevice(clientID, devID) ) {
			_enabledSignals = true;
			this->sendStatusSignalArrayToClients(_connectedClients.size(), _pDBus, "DevicesStarted", array);
			return true;
		}

		_enabledSignals = true;
		this->sendStatusSignalArrayToClients(_connectedClients.size(), _pDBus, "DevicesStopped", array);
		GKSysLog(LOG_ERR, ERROR, "device restarting failure : start failed");
		return false;
	}

	_enabledSignals = true;
	// TODO device could fail to stop for following reasons :
	//  * unknown clientID
	//  * client not allowed to stop device (not ready)
	//  * client not allowed to stop device (not alive)
	//  * client not allowed to stop device (not active)
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
	LOG(DEBUG2) << CONST_STRING_CLIENT << clientID;
#endif
	try {
		Client* pClient = _connectedClients.at(clientID);
		pClient->isAlive(); /* to avoid warning */
		return _pDevicesManager->getStartedDevices();
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
	LOG(DEBUG2) << CONST_STRING_CLIENT << clientID;
#endif
	try {
		Client* pClient = _connectedClients.at(clientID);
		pClient->isAlive(); /* to avoid warning */
		return _pDevicesManager->getStoppedDevices();
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
	LOG(DEBUG2)	<< CONST_STRING_DEVICE << devID << " "
				<< CONST_STRING_CLIENT << clientID;
#endif
	try {
		_connectedClients.at(clientID);
		return _pDevicesManager->getDeviceStatus(devID);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}

	return "unknown";
}

void ClientsManager::getDeviceProperties(
	const std::string & clientID,
	const std::string & devID
)	{
#if DEBUGGING_ON
	LOG(DEBUG2)	<< CONST_STRING_DEVICE << devID << " "
				<< CONST_STRING_CLIENT << clientID;
#endif
	try {
		Client* pClient = _connectedClients.at(clientID);
		if( pClient->isAlive() ) {
			/* initialize client device object which will be used to
			 * store properties in upcoming setDevice{Foo,Bar} calls */
			pClient->initializeDevice(_pDevicesManager, devID);
			_pDBus->appendAsyncString( _pDevicesManager->getDeviceVendor(devID) );
			_pDBus->appendAsyncString( _pDevicesManager->getDeviceProduct(devID) );
			_pDBus->appendAsyncString( _pDevicesManager->getDeviceName(devID) );
			_pDBus->appendAsyncUInt64( _pDevicesManager->getDeviceCapabilities(devID) );
			return;
		}
		GKSysLog(LOG_WARNING, WARNING, "getting device properties not allowed because client not alive");
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}
}

const LCDPluginsPropertiesArray_type & ClientsManager::getDeviceLCDPluginsProperties(
	const std::string & clientID,
	const std::string & devID)
{
#if DEBUGGING_ON
	LOG(DEBUG2)	<< CONST_STRING_DEVICE << devID << " "
				<< CONST_STRING_CLIENT << clientID;
#endif
	try {
		Client* pClient = _connectedClients.at(clientID);
		if( pClient->isAlive() ) {
			return _pDevicesManager->getDeviceLCDPluginsProperties(devID);
		}
		GKSysLog(LOG_WARNING, WARNING, "getting device LCDPluginsProperties not allowed because client not alive");
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}

	return LCDScreenPluginsManager::_LCDPluginsPropertiesEmptyArray;
}

const bool ClientsManager::setDeviceBacklightColor(
	const std::string & clientID,
	const std::string & devID,
	const uint8_t r,
	const uint8_t g,
	const uint8_t b
)	{
#if DEBUGGING_ON
	LOG(DEBUG2)	<< CONST_STRING_DEVICE << devID << " "
				<< CONST_STRING_CLIENT << clientID;
	LOG(DEBUG3) << "with following RGB bytes : "
				<< getHexRGB(r, g, b);
#endif
	try {
		Client* pClient = _connectedClients.at(clientID);
		return pClient->setDeviceBacklightColor(devID, r, g, b);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}
	return false;
}

const macro_type & ClientsManager::getDeviceMacro(
	const std::string & clientID,
	const std::string & devID,
	const std::string & keyName,
	const uint8_t bankID
)	{
#if DEBUGGING_ON
	LOG(DEBUG2)	<< CONST_STRING_DEVICE << devID << " "
				<< CONST_STRING_CLIENT << clientID;
	LOG(DEBUG3) << "   key : " << keyName;
	LOG(DEBUG3) << "bankID : " << toUInt(bankID);
#endif
	try {
		Client* pClient = _connectedClients.at(clientID);

		if(pClient->getSessionCurrentState() == _active) {
			if( pClient->isReady() ) {
				pClient->syncDeviceMacrosBanks(devID, _pDevicesManager->getDeviceMacrosBanks(devID));
				return pClient->getDeviceMacro(devID, keyName, bankID);
			}
			else {
				GKSysLog(LOG_WARNING, WARNING, "getting device macro not allowed while client not ready");
			}
		}
		else {
			GKSysLog(LOG_WARNING, WARNING, "only active user can get device macro");
		}
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}

	return MacrosBanks::emptyMacro;
}

const std::vector<std::string> & ClientsManager::getDeviceMacroKeysNames(
	const std::string & clientID,
	const std::string & devID
)	{
#if DEBUGGING_ON
	LOG(DEBUG2)	<< CONST_STRING_DEVICE << devID << " "
				<< CONST_STRING_CLIENT << clientID;
#endif
	try {
		_connectedClients.at(clientID);
		return _pDevicesManager->getDeviceMacroKeysNames(devID);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}

	return KeyboardDriver::getEmptyStringVector();
}

const bool ClientsManager::setDeviceMacrosBank(
	const std::string & clientID,
	const std::string & devID,
	const uint8_t bankID,
	const mBank_type & bank
)	{
#if DEBUGGING_ON
	LOG(DEBUG2)	<< CONST_STRING_DEVICE << devID << " "
				<< CONST_STRING_CLIENT << clientID;
	LOG(DEBUG3) << "bankID : " << toUInt(bankID);
#endif
	try {
		Client* pClient = _connectedClients.at(clientID);
		return pClient->setDeviceMacrosBank(devID, bankID, bank);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}

	return false;
}

const bool ClientsManager::resetDeviceMacrosBank(
	const std::string & clientID,
	const std::string & devID,
	const uint8_t bankID)
{
#if DEBUGGING_ON
	LOG(DEBUG2)	<< CONST_STRING_DEVICE << devID << " "
				<< CONST_STRING_CLIENT << clientID;
	LOG(DEBUG3) << "bankID : " << toUInt(bankID);
#endif
	try {
		Client* pClient = _connectedClients.at(clientID);
		return pClient->resetDeviceMacrosBank(devID, bankID);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}

	return false;
}

const bool ClientsManager::setDeviceLCDPluginsMask(
	const std::string & clientID,
	const std::string & devID,
	const uint8_t maskID,
	const uint64_t mask)
{
#if DEBUGGING_ON
	LOG(DEBUG2)	<< CONST_STRING_DEVICE << devID << " "
				<< CONST_STRING_CLIENT << clientID;
	LOG(DEBUG3) << "maskID : " << toUInt(maskID);
#endif

	try {
		Client* pClient = _connectedClients.at(clientID);
		return pClient->setDeviceLCDPluginsMask(devID, maskID, mask);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownClient
	}
	return false;
}

} // namespace GLogiK

