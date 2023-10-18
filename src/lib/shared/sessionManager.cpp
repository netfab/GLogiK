/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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

#include <csignal>

#include <string>
#include <iostream>
#include <sstream>

#include "lib/utils/utils.hpp"

#include "sessionManager.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

bool SessionManager::stillRunning = true;

SessionManager::SessionManager() {
	GK_LOG_FUNC

	GKLog(trace, "initializing session manager")

	_mask = (SmcSaveYourselfProcMask|SmcDieProcMask|SmcSaveCompleteProcMask|SmcShutdownCancelledProcMask);

	_callbacks.save_yourself.client_data = nullptr;
	_callbacks.save_yourself.callback = SessionManager::SaveYourselfCallback;

	_callbacks.die.client_data = nullptr;
	_callbacks.die.callback = SessionManager::DieCallback;

	_callbacks.save_complete.client_data = nullptr;
	_callbacks.save_complete.callback = SessionManager::SaveCompleteCallback;

	_callbacks.shutdown_cancelled.client_data = nullptr;
	_callbacks.shutdown_cancelled.callback = SessionManager::ShutdownCancelledCallback;
}

SessionManager::~SessionManager() {
	GK_LOG_FUNC

	GKLog(trace, "destroying session manager")

	IceRemoveConnectionWatch(SessionManager::ICEConnectionWatchCallback, nullptr);

	this->closeConnection();

	if(_pClientID != nullptr)
		free(_pClientID);

	GKLog(trace, "session manager destroyed")
}

/* -- */
/*
 *	public
 */

const int SessionManager::openConnection(void) {
	GK_LOG_FUNC

	GKLog(trace, "opening session manager connection")

	process::setSignalHandler( SIGINT, SessionManager::handleSignal);
	process::setSignalHandler(SIGTERM, SessionManager::handleSignal);

	_pSMCConnexion = SmcOpenConnection(
							nullptr,				/* network_ids_list */
							nullptr,				/* context */
							1,						/* xsmp_major_rev */
							0,						/* xsmp_minor_rev */
							_mask,					/* mask */
							&_callbacks,			/* callbacks */
							_pPreviousID,			/* previous_id */
							&_pClientID,			/* client_id_ret */
							SM_ERROR_STRING_LENGTH, /* error_string_ret max length */
							_errorString			/* error_string_ret */
						);

	if(_pSMCConnexion == nullptr) {
		char * s = _errorString;
		std::string failure("Unable to open connection to session manager : ");
		failure += s;
		throw GLogiKExcept(failure);
	}

	Status ret = IceAddConnectionWatch(SessionManager::ICEConnectionWatchCallback, nullptr);
	if( ret == 0 ) {
		this->closeConnection();
		throw GLogiKExcept("IceAddConnectionWatch failure");
	}

	_pICEConnexion = SmcGetIceConnection(_pSMCConnexion);
	_ICEfd = IceConnectionNumber(_pICEConnexion);

	GKLog2(trace, "Session manager client ID : ", _pClientID)

	return _ICEfd;
}

const bool SessionManager::isSessionAlive(void) {
	return SessionManager::stillRunning;
}

void SessionManager::processICEMessages(void) {
	SessionManager::processICEMessages(_pICEConnexion);
}

/* -- */
/*
 *	private
 */

void SessionManager::closeConnection(void) {
	GK_LOG_FUNC

	if(_pSMCConnexion == nullptr)
		return;

	SmcCloseStatus status = SmcCloseConnection(_pSMCConnexion, 0, nullptr);
	switch( status ) {
		case SmcClosedNow:
			GKLog(trace, "ICE connection was closed")
			break;
		case SmcClosedASAP:
			GKLog(trace, "ICE connection will be freed ASAP")
			break;
		case SmcConnectionInUse:
			GKLog(trace, "ICE connection not closed because in used")
			break;
	}

}

void SessionManager::handleSignal(int sig) {
	GK_LOG_FUNC

	std::ostringstream buff("caught signal : ", std::ios_base::app);
	switch( sig ) {
		case SIGINT:
		case SIGTERM:
			buff << sig << " --> bye bye";
			LOG(info) << buff.str();

			process::resetSignalHandler(SIGINT);
			process::resetSignalHandler(SIGTERM);

			SessionManager::stillRunning = false;
			break;
		default:
			buff << sig << " --> unhandled";
			LOG(warning) << buff.str();
			break;
	}
}

void SessionManager::processICEMessages(IceConn ice_conn) {
	GK_LOG_FUNC

	int ret = IceProcessMessages(ice_conn, nullptr, nullptr);

	switch(ret) {
		case IceProcessMessagesSuccess:
			GKLog(trace, "ICE messages process success")
			break;
		case IceProcessMessagesIOError:
			LOG(warning) << "IO error, closing ICE connection";
			IceCloseConnection(ice_conn);
			// TODO throw ?
			break;
		case IceProcessMessagesConnectionClosed:
			GKLog(trace, "ICE connection has been closed")
			break;
	}
}

/*
 * callback called when ICE connections are created or destroyed
 */
void SessionManager::ICEConnectionWatchCallback(IceConn ice_conn, IcePointer client_data,
	Bool opening, IcePointer *watch_data)
{
/*
	if( ! opening ) {
		return;
	}
*/
	SessionManager::processICEMessages(ice_conn);
}

void SessionManager::SaveYourselfCallback(
	SmcConn smc_conn,
	SmPointer client_data,
	int save_type,
	Bool shutdown,
	int interact_style,
	Bool fast)
{
	GK_LOG_FUNC

	GKLog(trace, "SM save yourself call")

	SmcSaveYourselfDone(smc_conn, true);
}

void SessionManager::DieCallback(SmcConn smc_conn, SmPointer client_data)
{
	GK_LOG_FUNC

	GKLog(trace, "SM die call, behaves like we received SIGTERM")

	SessionManager::handleSignal(SIGTERM);
}

void SessionManager::SaveCompleteCallback(SmcConn smc_conn, SmPointer client_data)
{
	GK_LOG_FUNC

	GKLog(trace, "SM save complete call")
}

void SessionManager::ShutdownCancelledCallback(SmcConn smc_conn, SmPointer client_data)
{
	GK_LOG_FUNC

	GKLog(trace, "SM shutdown cancelled call")
}

} // namespace GLogiK

