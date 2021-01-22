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

#include <csignal>

#include <string>
#include <iostream>

#include "lib/utils/utils.hpp"

#include "sessionManager.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

bool SessionManager::stillRunning = true;

SessionManager::SessionManager() {
	_mask = (SmcSaveYourselfProcMask|SmcDieProcMask|SmcSaveCompleteProcMask|SmcShutdownCancelledProcMask);

	_callbacks.save_yourself.client_data = nullptr;
	_callbacks.save_yourself.callback = SessionManager::SaveYourselfCallback;

	_callbacks.die.client_data = nullptr;
	_callbacks.die.callback = SessionManager::DieCallback;

	_callbacks.save_complete.client_data = nullptr;
	_callbacks.save_complete.callback = SessionManager::SaveCompleteCallback;

	_callbacks.shutdown_cancelled.client_data = nullptr;
	_callbacks.shutdown_cancelled.callback = SessionManager::ShutdownCancelledCallback;
#if DEBUGGING_ON
	LOG(DEBUG1) << "session manager initialized";
#endif
}

SessionManager::~SessionManager() {
#if DEBUGGING_ON
	LOG(DEBUG1) << "destroying session manager";
#endif
	IceRemoveConnectionWatch(SessionManager::ICEConnectionWatchCallback, nullptr);

	this->closeConnection();

	if(_pClientID != nullptr)
		free(_pClientID);

#if DEBUGGING_ON
	LOG(DEBUG1) << "session manager destroyed";
#endif
}

/* -- */
/*
 *	public
 */

const int SessionManager::openConnection(void) {
#if DEBUGGING_ON
	LOG(DEBUG) << "opening session manager connection";
#endif
	std::signal(SIGINT, SessionManager::handleSignal);
	std::signal(SIGTERM, SessionManager::handleSignal);

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

#if DEBUGGING_ON
	LOG(DEBUG) << "Session manager client ID : " << _pClientID;
#endif

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
	if(_pSMCConnexion == nullptr)
		return;

	SmcCloseStatus status = SmcCloseConnection(_pSMCConnexion, 0, nullptr);
	switch( status ) {
		case SmcClosedNow:
#if DEBUGGING_ON
			LOG(DEBUG2) << "ICE connection was closed";
#endif
			break;
		case SmcClosedASAP:
#if DEBUGGING_ON
			LOG(DEBUG2) << "ICE connection will be freed ASAP";
#endif
			break;
		case SmcConnectionInUse:
#if DEBUGGING_ON
			LOG(DEBUG2) << "ICE connection not closed because in used";
#endif
			break;
	}

}

void SessionManager::handleSignal(int sig) {
	std::ostringstream buff("caught signal : ", std::ios_base::app);
	switch( sig ) {
		case SIGINT:
		case SIGTERM:
			buff << sig << " --> bye bye";
			LOG(INFO) << buff.str();
			std::signal(SIGINT, SIG_DFL);
			std::signal(SIGTERM, SIG_DFL);
			SessionManager::stillRunning = false;
			break;
		default:
			buff << sig << " --> unhandled";
			LOG(WARNING) << buff.str();
			break;
	}
}

void SessionManager::processICEMessages(IceConn ice_conn) {
	int ret = IceProcessMessages(ice_conn, nullptr, nullptr);

	switch(ret) {
		case IceProcessMessagesSuccess:
#if DEBUGGING_ON
			LOG(DEBUG3) << "ICE messages process success";
#endif
			break;
		case IceProcessMessagesIOError:
			LOG(WARNING) << "IO error, closing ICE connection";
			IceCloseConnection(ice_conn);
			// TODO throw ?
			break;
		case IceProcessMessagesConnectionClosed:
#if DEBUGGING_ON
			LOG(DEBUG3) << "ICE connection has been closed";
#endif
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
#if DEBUGGING_ON
	LOG(DEBUG2) << "SM save yourself call";
#endif
	SmcSaveYourselfDone(smc_conn, true);
}

void SessionManager::DieCallback(SmcConn smc_conn, SmPointer client_data)
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "SM die call, behaves like we received SIGTERM";
#endif
	SessionManager::handleSignal(SIGTERM);
}

void SessionManager::SaveCompleteCallback(SmcConn smc_conn, SmPointer client_data)
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "SM save complete call";
#endif
}

void SessionManager::ShutdownCancelledCallback(SmcConn smc_conn, SmPointer client_data)
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "SM shutdown cancelled call";
#endif
}

} // namespace GLogiK

