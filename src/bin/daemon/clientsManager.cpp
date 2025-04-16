/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2025  Fabrice Delliaux <netbox253@gmail.com>
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

ClientsManager::ClientsManager(
	NSGKDBus::GKDBus* const pDBus,
	DevicesManager* const pDevicesManager,
	GKDepsMap_type* const pDepsMap
)	:	_pDBus(pDBus),
		_pDevicesManager(pDevicesManager),
		_pDepsMap(pDepsMap),
		_active("active"),
		_numActive(0),
		_enabledSignals(true)
{
	GK_LOG_FUNC

	GKLog(trace, "initializing clients manager")

	this->initializeDBusRequests();
}

ClientsManager::~ClientsManager()
{
	GK_LOG_FUNC

	GKLog(trace, "destroying clients manager")

	for( auto & clientPair : _connectedClients ) {
		Client* pClient = clientPair.second;
		if( pClient != nullptr ) { /* sanity check */
			std::ostringstream buffer(std::ios_base::app);
			buffer << "destroying unfreed client : " << clientPair.first;
			GKSysLogWarning(buffer.str());
			delete pClient; pClient = nullptr;
		}
	}
	_connectedClients.clear();

	GKLog(trace, "exiting clients manager")
}

void ClientsManager::initializeDBusRequests(void)
{
	/* clients manager DBus object path and interface */
	const auto & CM_OP = GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH;
	const auto & CM_IF = GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE;

	/* devices manager DBus object path and interface */
	const auto & DM_OP = GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH;
	const auto & DM_IF = GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE;

	const std::string dIN("in");	/* direction in */
	const std::string dOUT("out");	/* direction out */

	const std::string rValueComment("did the method succeeded ?");

	/* -- -- -- -- -- -- -- -- -- -- */
	/*  ClientsManager D-Bus object  */
	/* -- -- -- -- -- -- -- -- -- -- */

	_pDBus->NSGKDBus::Callback<SIGs2b>::exposeMethod(
		_systemBus, CM_OP, CM_IF, "RegisterClient",
		{	{"s", "client_session_object_path", dIN, "client session object path"},
			{"b", "did_register_succeeded", dOUT, rValueComment},
			{"s", "failure_reason_or_client_id", dOUT, "if register success (bool==true), unique client ID, else (bool=false) failure reason"} },
		std::bind(&ClientsManager::registerClient, this, std::placeholders::_1) );

	_pDBus->NSGKDBus::Callback<SIGs2b>::exposeMethod(
		_systemBus, CM_OP, CM_IF, "UnregisterClient",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"b", "did_unregister_succeeded", dOUT, rValueComment} },
		std::bind(&ClientsManager::unregisterClient, this, std::placeholders::_1) );

	_pDBus->NSGKDBus::Callback<SIGss2b>::exposeMethod(
		_systemBus, CM_OP, CM_IF, "UpdateClientState",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "client_new_state", dIN, "client new state"},
			{"b", "did_updateclientstate_succeeded", dOUT, rValueComment} },
		std::bind(&ClientsManager::updateClientState, this, std::placeholders::_1, std::placeholders::_2) );

	_pDBus->NSGKDBus::Callback<SIGs2b>::exposeMethod(
		_systemBus, CM_OP, CM_IF, "SetClientReady",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"b", "did_method_succeeded", dOUT, rValueComment} },
		std::bind(&ClientsManager::setClientReady, this, std::placeholders::_1) );

	_pDBus->NSGKDBus::Callback<SIGss2b>::exposeMethod(
		_systemBus, CM_OP, CM_IF, "DeleteDeviceConfiguration",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStartedDevices or GetStoppedDevices"},
			{"b", "did_deletedeviceconfiguration_succeeded", dOUT, rValueComment} },
		std::bind(&ClientsManager::deleteDeviceConfiguration, this, std::placeholders::_1, std::placeholders::_2) );

	_pDBus->NSGKDBus::Callback<SIGs2D>::exposeMethod(
		_systemBus, CM_OP, CM_IF, "GetDaemonDependenciesMap",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"a(yta(sss))", "dependencies_map", dOUT, "array of executable dependencies"} },
		std::bind(&ClientsManager::getDaemonDependenciesMap, this, std::placeholders::_1) );

	/* -- -- -- -- -- -- -- -- -- -- */
	/*  DevicesManager D-Bus object  */
	/* -- -- -- -- -- -- -- -- -- -- */

	_pDBus->NSGKDBus::Callback<SIGss2b>::exposeMethod(
		_systemBus, DM_OP, DM_IF, "StopDevice",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStartedDevices"},
			{"b", "did_stop_succeeded", dOUT, rValueComment} },
		std::bind(&ClientsManager::stopDevice, this, std::placeholders::_1, std::placeholders::_2) );

	_pDBus->NSGKDBus::Callback<SIGss2b>::exposeMethod(
		_systemBus, DM_OP, DM_IF, "StartDevice",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStoppedDevices"},
			{"b", "did_start_succeeded", dOUT, rValueComment} },
		std::bind(&ClientsManager::startDevice, this, std::placeholders::_1, std::placeholders::_2) );

	_pDBus->NSGKDBus::Callback<SIGss2b>::exposeMethod(
		_systemBus, DM_OP, DM_IF, "RestartDevice",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStartedDevices"},
			{"b", "did_restart_succeeded", dOUT, rValueComment} },
		std::bind(&ClientsManager::restartDevice, this, std::placeholders::_1, std::placeholders::_2) );

		/* -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
		/* methods used to initialize devices on service-side */
		/* -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */

	_pDBus->NSGKDBus::Callback<SIGs2as>::exposeMethod(
		_systemBus, DM_OP, DM_IF, "GetStartedDevices",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"as", "array_of_strings", dOUT, "array of started devices ID strings"} },
		std::bind(&ClientsManager::getStartedDevices, this, std::placeholders::_1) );

	_pDBus->NSGKDBus::Callback<SIGs2as>::exposeMethod(
		_systemBus, DM_OP, DM_IF, "GetStoppedDevices",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"as", "array_of_strings", dOUT, "array of stopped devices ID strings"} },
		std::bind(&ClientsManager::getStoppedDevices, this, std::placeholders::_1) );

	_pDBus->NSGKDBus::Callback<SIGss2s>::exposeMethod(
		_systemBus, DM_OP, DM_IF, "GetDeviceStatus",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID"},
			{"s", "device status", dOUT, "string representing the device status"} },
		std::bind(&ClientsManager::getDeviceStatus, this, std::placeholders::_1, std::placeholders::_2) );

	_pDBus->NSGKDBus::Callback<SIGss2v>::exposeMethod(
		_systemBus, DM_OP, DM_IF, "GetDeviceProperties",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStartedDevices or GetStoppedDevices"},
			{"sst", "get_device_properties", dOUT, "device properties"} },
		std::bind(&ClientsManager::getDeviceProperties, this, std::placeholders::_1, std::placeholders::_2) );

	_pDBus->NSGKDBus::Callback<SIGss2aP>::exposeMethod(
		_systemBus, DM_OP, DM_IF, "GetDeviceLCDPluginsProperties",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStartedDevices or GetStoppedDevices"},
			{"a(tss)", "get_lcd_plugins_properties_array", dOUT, "LCDPluginsProperties array"} },
		std::bind(&ClientsManager::getDeviceLCDPluginsProperties, this, std::placeholders::_1, std::placeholders::_2) );

	_pDBus->NSGKDBus::Callback<SIGss2aG>::exposeMethod(
		_systemBus, DM_OP, DM_IF, "GetDeviceGKeysIDArray",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStartedDevices or GetStoppedDevices"},
			{"ay", "array_of_keys_id", dOUT, "array of G-keys ID for the device"} },
		std::bind(&ClientsManager::getDeviceGKeysIDArray, this, std::placeholders::_1, std::placeholders::_2) );

	_pDBus->NSGKDBus::Callback<SIGss2am>::exposeMethod(
		_systemBus, DM_OP, DM_IF, "GetDeviceMKeysIDArray",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStartedDevices or GetStoppedDevices"},
			{"ay", "array_of_keys_id", dOUT, "array of M-keys ID for the device"} },
		std::bind(&ClientsManager::getDeviceMKeysIDArray, this, std::placeholders::_1, std::placeholders::_2) );

	_pDBus->NSGKDBus::Callback<SIGssyyy2b>::exposeMethod(
		_systemBus, DM_OP, DM_IF, "SetDeviceBacklightColor",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStartedDevices"},
			{"y", "red_byte", dIN, "red byte for the RGB color model"},
			{"y", "green_byte", dIN, "green byte for the RGB color model"},
			{"y", "blue_byte", dIN, "blue byte for the RGB color model"},
			{"b", "did_setcolor_succeeded", dOUT, rValueComment} },
		std::bind(&ClientsManager::setDeviceBacklightColor, this, std::placeholders::_1, std::placeholders::_2,
			std::placeholders::_3, std::placeholders::_4, std::placeholders::_5) );

	_pDBus->NSGKDBus::Callback<SIGssyt2b>::exposeMethod(
		_systemBus, DM_OP, DM_IF, "SetDeviceLCDPluginsMask",
		{	{"s", "client_unique_id", dIN, "must be a valid client ID"},
			{"s", "device_id", dIN, "device ID coming from GetStartedDevices"},
			{"y", "LCD_Plugins_Mask_ID", dIN, "LCD plugins mask ID"},
			{"t", "LCD_Plugins_Mask", dIN, "LCD plugins mask"},
			{"b", "did_setmask_succeeded", dOUT, rValueComment} },
		std::bind(&ClientsManager::setDeviceLCDPluginsMask, this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4) );

	/* -- -- -- -- -- -- -- -- -- -- -- -- -- */
	/*  declaration of introspectable signals */
	/*       potentially sent by daemon       */
	/* -- -- -- -- -- -- -- -- -- -- -- -- -- */

	/*  ClientsManager D-Bus object  */

	_pDBus->declareIntrospectableSignal(_systemBus, CM_OP, CM_IF, "DaemonIsStopping", {});
	_pDBus->declareIntrospectableSignal(_systemBus, CM_OP, CM_IF, "DaemonIsStarting", {});
	_pDBus->declareIntrospectableSignal(_systemBus, CM_OP, CM_IF, "ReportYourself",   {});

	/*  DevicesManager D-Bus object  */

	_pDBus->declareIntrospectableSignal(
		_systemBus, DM_OP, DM_IF, "DevicesStarted",
		{	{"as", "", dOUT, "array of started devices ID strings"} }
	);

	_pDBus->declareIntrospectableSignal(
		_systemBus, DM_OP, DM_IF, "DevicesStopped",
		{	{"as", "", dOUT, "array of stopped devices ID strings"} }
	);

	_pDBus->declareIntrospectableSignal(
		_systemBus, DM_OP, DM_IF, "DevicesUnplugged",
		{	{"as", "", dOUT, "array of unplugged devices ID strings"} }
	);

	_pDBus->declareIntrospectableSignal(
		_systemBus, DM_OP, DM_IF, "DeviceMBankSwitch",
		{	{"s", "device_id", "in", "device ID"},
			{"y", "macro_bankID", "in", "macro bankID"} }
	);

	_pDBus->declareIntrospectableSignal(
		_systemBus, DM_OP, DM_IF, "DeviceMacroRecorded",
		{	{"s", "device_id", dOUT, "device ID"},
			{"y", "macro_bankID", "in", "macro bankID"},
			{"y", "macro_keyID", "in", "macro key ID"} }
	);

	_pDBus->declareIntrospectableSignal(
		_systemBus, DM_OP, DM_IF, "DeviceMacroCleared",
		{	{"s", "device_id", dOUT, "device ID"},
			{"y", "macro_bankID", "in", "macro bankID"},
			{"y", "macro_keyID", "in", "macro key ID"} }
	);

	_pDBus->declareIntrospectableSignal(
		_systemBus, DM_OP, DM_IF, "DeviceMediaEvent",
		{	{"s", "device_id", "in", "device ID"},
			{"s", "media_key_event", "in", "media key event"} }
	);
}

void ClientsManager::cleanDBusRequests(void) noexcept
{
	/* clients manager DBus object path and interface */
	const auto & CM_OP = GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH;
	const auto & CM_IF = GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE;

	/* devices manager DBus object path and interface */
	const auto & DM_OP = GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH;
	const auto & DM_IF = GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE;

	_pDBus->removeMethodsInterface(_systemBus, CM_OP, CM_IF);
	_pDBus->removeMethodsInterface(_systemBus, DM_OP, DM_IF);
}

void ClientsManager::waitForClientsDisconnections(void) noexcept
{
	GK_LOG_FUNC

	if( _connectedClients.empty() ) {
		GKLog(trace, "no client, empty container")
		return;
	}

	this->sendSignalToClients(_connectedClients.size(), _pDBus, "DaemonIsStopping");

	uint16_t c = 0;
	GKLog(trace, "waiting for clients to unregister ...")

	while( c++ < 40 and _connectedClients.size() > 0 ) { /* bonus point */
		_pDevicesManager->checkDBusMessages();
		GKLog(trace, "sleeping for 40 ms ...")
		std::this_thread::sleep_for(std::chrono::milliseconds(40));
	}
}

const bool ClientsManager::registerClient(
	const std::string & clientSessionObjectPath)
{
	GK_LOG_FUNC

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
				GKSysLogWarning(buffer.str());

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
			GKSysLogWarning(buffer.str());
			this->unregisterClient(clientID);
		}
		toUnregister.clear();

		throw std::out_of_range("not found");
	}
	catch (const std::out_of_range& oor) {
		std::string clientID;

		try {
			auto get_random_clientID = [] () -> const std::string
			{
				RandomGenerator rand;
				std::ostringstream ret(std::ios_base::app);
				ret << rand.getString(6) << "-" << rand.getString(4) << "-"
					<< rand.getString(4) << "-" << rand.getString(6);
				return ret.str();
			};

			clientID = get_random_clientID();

			std::ostringstream buffer(std::ios_base::app);
			buffer << "registering new client with ID : " << clientID;
			GKSysLogInfo(buffer.str());

			/* check that clientID not already used */
			if( _connectedClients.find(clientID) != _connectedClients.end() )
				throw GLogiKExcept("already connected clientID");

			_connectedClients[clientID] = new Client(clientSessionObjectPath, _pDevicesManager);
		}
		catch (const std::bad_alloc& e) { /* handle new() failure */
			const std::string s = "new client allocation failure";
			GKSysLogError(s);
			_pDBus->appendAsyncString(s);
			return false;
		}
		catch (const GLogiKExcept & e) {
			GKSysLogError(e.what());
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
	const std::string & clientID)
{
	GK_LOG_FUNC

	try {
		Client* pClient = _connectedClients.at(clientID);

		std::ostringstream buffer(std::ios_base::app);
		buffer << "unregistering client : " << clientID;
		GKSysLogInfo(buffer.str());

		/* resetting devices states first */
		if( pClient->getSessionCurrentState() == _active ) {
			_pDevicesManager->resetDevicesStates();
			GKLog2(trace, "decreasing active users # : ", _numActive)
			_numActive--;
		}

		delete pClient; pClient = nullptr;
		_connectedClients.erase(clientID);
		_pDevicesManager->setNumClients( _connectedClients.size() );

		return true;
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_CLIENT, clientID);
	}
	return false;
}

const bool ClientsManager::updateClientState(
	const std::string & clientID,
	const std::string & state)
{
	GK_LOG_FUNC

	GKLog4(trace,
		CONST_STRING_CLIENT, clientID,
		"state : ", state
	)

	if( (state != _active) and (state != "online") ) {
		std::ostringstream buffer(std::ios_base::app);
		buffer << "unhandled state for updating devices : " << state;
		GKSysLogWarning(buffer.str());
		return false;
	}

	try {
		Client* pClient = _connectedClients.at(clientID);
		const std::string oldState( pClient->getSessionCurrentState() );
		pClient->updateSessionState(state);

		if( (oldState == _active) and (state != _active) ) {
			GKLog2(trace, "decreasing active users # : ", _numActive)
			_numActive--;
		}

		if(state == _active) {
			if(oldState != _active) {
				GKLog2(trace, "increasing active users # : ", _numActive)
				_numActive++;
			}

			if( pClient->isReady() ) {
				GKLog(trace, "setting active user's parameters for all started devices")
				for(const auto & devID : _pDevicesManager->getStartedDevices()) {
					pClient->setDeviceActiveUser(devID, _pDevicesManager);
				}
			}
		}

		GKLog2(trace, "active users # : ", _numActive)
		if(_numActive == 0) {
			_pDevicesManager->resetDevicesStates();
		}

		return true;
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_CLIENT, clientID);
	}

	return false;
}

const bool ClientsManager::setClientReady(const std::string & clientID)
{
	GK_LOG_FUNC

	try {
		Client* pClient = _connectedClients.at(clientID);
		pClient->setReady();
		if( pClient->isReady() ) {
			if(pClient->getSessionCurrentState() == _active) {
				GKLog(trace, "setting active user's parameters for all started devices")
				for(const auto & devID : _pDevicesManager->getStartedDevices()) {
					pClient->setDeviceActiveUser(devID, _pDevicesManager);
				}
			}
		}
		return true;
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_CLIENT, clientID);
	}
	return false;
}

const bool ClientsManager::deleteDeviceConfiguration(
	const std::string & clientID,
	const std::string & devID)
{
	GK_LOG_FUNC

	GKLog4(trace,
		CONST_STRING_DEVICE, devID,
		CONST_STRING_CLIENT, clientID
	)

	try {
		Client* pClient = _connectedClients.at(clientID);
		return pClient->deleteDevice(devID);
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_CLIENT, clientID);
	}

	return false;
}

const GKDepsMap_type &
	ClientsManager::getDaemonDependenciesMap(const std::string & clientID)
{
	GK_LOG_FUNC

	GKLog2(trace, CONST_STRING_CLIENT, clientID)

	try {
		/* just checking that provided ID is known or log error */
		Client* pClient = _connectedClients.at(clientID);
		pClient->isAlive(); /* to avoid warning */
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_CLIENT, clientID);
	}

	return (*_pDepsMap);
}

const bool ClientsManager::stopDevice(
	const std::string & clientID,
	const std::string & devID)
{
	GK_LOG_FUNC

	GKLog4(trace,
		CONST_STRING_DEVICE, devID,
		CONST_STRING_CLIENT, clientID
	)

	try {
		Client* pClient = _connectedClients.at(clientID);

		if( ! pClient->isReady() ) {
			GKSysLogWarning("device state change not allowed while client not ready");
			return false;
		}

		if( ! pClient->isAlive() ) {
			GKSysLogWarning("device state change not allowed because client not alive");
			return false;
		}

		if(pClient->getSessionCurrentState() != _active) {
			GKSysLogWarning("only active user can change device state");
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
		GKSysLogError(CONST_STRING_UNKNOWN_CLIENT, clientID);
	}
	return false;
}

const bool ClientsManager::startDevice(
	const std::string & clientID,
	const std::string & devID)
{
	GK_LOG_FUNC

	GKLog4(trace,
		CONST_STRING_DEVICE, devID,
		CONST_STRING_CLIENT, clientID
	)

	try {
		Client* pClient = _connectedClients.at(clientID);

		if( ! pClient->isReady() ) {
			GKSysLogWarning("device state change not allowed while client not ready");
			return false;
		}

		if( ! pClient->isAlive() ) {
			GKSysLogWarning("device state change not allowed because client not alive");
			return false;
		}

		if(pClient->getSessionCurrentState() != _active) {
			GKSysLogWarning("only active user can change device state");
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
		GKSysLogError(CONST_STRING_UNKNOWN_CLIENT, clientID);
	}
	return false;
}

const bool ClientsManager::restartDevice(
	const std::string & clientID,
	const std::string & devID)
{
	GK_LOG_FUNC

	GKLog4(trace,
		CONST_STRING_DEVICE, devID,
		CONST_STRING_CLIENT, clientID
	)

	_enabledSignals = false;
	const std::vector<std::string> array = {devID};

	if( this->stopDevice(clientID, devID) ) {

		GKLog(trace, "sleeping for 1000 ms")
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		if( this->startDevice(clientID, devID) ) {
			_enabledSignals = true;
			this->sendStatusSignalArrayToClients(_connectedClients.size(), _pDBus, "DevicesStarted", array);
			return true;
		}

		_enabledSignals = true;
		this->sendStatusSignalArrayToClients(_connectedClients.size(), _pDBus, "DevicesStopped", array);
		GKSysLogError("device restarting failure : start failed");
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
	GKSysLogError("device restarting failure : stop failed");
	return false;
}

const std::vector<std::string>
	ClientsManager::getStartedDevices(const std::string & clientID)
{
	GK_LOG_FUNC

	GKLog2(trace, CONST_STRING_CLIENT, clientID)

	try {
		Client* pClient = _connectedClients.at(clientID);
		pClient->isAlive(); /* to avoid warning */
		return _pDevicesManager->getStartedDevices();
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_CLIENT, clientID);
	}

	const std::vector<std::string> ret;
	return ret;
}

const std::vector<std::string>
	ClientsManager::getStoppedDevices(const std::string & clientID)
{
	GK_LOG_FUNC

	GKLog2(trace, CONST_STRING_CLIENT, clientID)

	try {
		Client* pClient = _connectedClients.at(clientID);
		pClient->isAlive(); /* to avoid warning */
		return _pDevicesManager->getStoppedDevices();
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_CLIENT, clientID);
	}

	const std::vector<std::string> ret;
	return ret;
}

const std::string ClientsManager::getDeviceStatus(
	const std::string & clientID,
	const std::string & devID)
{
	GK_LOG_FUNC

	GKLog4(trace,
		CONST_STRING_DEVICE, devID,
		CONST_STRING_CLIENT, clientID
	)

	try {
		_connectedClients.at(clientID);
		return _pDevicesManager->getDeviceStatus(devID);
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_CLIENT, clientID);
	}

	return "unknown";
}

void ClientsManager::getDeviceProperties(
	const std::string & clientID,
	const std::string & devID)
{
	GK_LOG_FUNC

	GKLog4(trace,
		CONST_STRING_DEVICE, devID,
		CONST_STRING_CLIENT, clientID
	)

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
		GKSysLogWarning("getting device properties not allowed because client not alive");
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_CLIENT, clientID);
	}
}

const LCDPPArray_type & ClientsManager::getDeviceLCDPluginsProperties(
		const std::string & clientID,
		const std::string & devID)
{
	GK_LOG_FUNC

	GKLog4(trace,
		CONST_STRING_DEVICE, devID,
		CONST_STRING_CLIENT, clientID
	)

	try {
		Client* pClient = _connectedClients.at(clientID);
		if( pClient->isAlive() ) {
			return _pDevicesManager->getDeviceLCDPluginsProperties(devID);
		}
		GKSysLogWarning("getting device LCDPluginsProperties not allowed because client not alive");
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_CLIENT, clientID);
	}

	return LCDScreenPluginsManager::_LCDPluginsPropertiesEmptyArray;
}

const bool ClientsManager::setDeviceBacklightColor(
	const std::string & clientID,
	const std::string & devID,
	const uint8_t r,
	const uint8_t g,
	const uint8_t b)
{
	GK_LOG_FUNC

	GKLog6(trace,
		CONST_STRING_DEVICE, devID,
		CONST_STRING_CLIENT, clientID,
		"RGB bytes : ", getHexRGB(r, g, b)
	)

	try {
		Client* pClient = _connectedClients.at(clientID);
		return pClient->setDeviceBacklightColor(devID, r, g, b);
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_CLIENT, clientID);
	}
	return false;
}

const MKeysIDArray_type
	ClientsManager::getDeviceMKeysIDArray(
		const std::string & clientID,
		const std::string & devID)
{
	GK_LOG_FUNC

	GKLog4(trace,
		CONST_STRING_DEVICE, devID,
		CONST_STRING_CLIENT, clientID
	)

	try {
		_connectedClients.at(clientID);
		return _pDevicesManager->getDeviceMKeysIDArray(devID);
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_CLIENT, clientID);
	}

	MKeysIDArray_type ret;
	return ret;
}

const GKeysIDArray_type
	ClientsManager::getDeviceGKeysIDArray(
		const std::string & clientID,
		const std::string & devID)
{
	GK_LOG_FUNC

	GKLog4(trace,
		CONST_STRING_DEVICE, devID,
		CONST_STRING_CLIENT, clientID
	)

	try {
		_connectedClients.at(clientID);
		return _pDevicesManager->getDeviceGKeysIDArray(devID);
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_CLIENT, clientID);
	}

	GKeysIDArray_type ret;
	return ret;
}

const bool ClientsManager::setDeviceLCDPluginsMask(
	const std::string & clientID,
	const std::string & devID,
	const uint8_t maskID,
	const uint64_t mask)
{
	GK_LOG_FUNC

	GKLog6(trace,
		CONST_STRING_DEVICE, devID,
		CONST_STRING_CLIENT, clientID,
		"maskID : ", toUInt(maskID)
	)

	try {
		Client* pClient = _connectedClients.at(clientID);
		return pClient->setDeviceLCDPluginsMask(devID, maskID, mask);
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_CLIENT, clientID);
	}
	return false;
}

} // namespace GLogiK

