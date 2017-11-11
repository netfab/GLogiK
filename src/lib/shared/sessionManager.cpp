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

#include <csignal>

#include <string>
#include <iostream>

#include "lib/utils/utils.h"

#include "sessionManager.h"

namespace GLogiK
{

bool SessionManager::still_running_ = true;

SessionManager::SessionManager() {
	this->mask_ = (SmcSaveYourselfProcMask|SmcDieProcMask|SmcSaveCompleteProcMask|SmcShutdownCancelledProcMask);

	this->callbacks_.save_yourself.client_data = nullptr;
	this->callbacks_.save_yourself.callback = SessionManager::SaveYourselfCallback;

	this->callbacks_.die.client_data = nullptr;
	this->callbacks_.die.callback = SessionManager::DieCallback;

	this->callbacks_.save_complete.client_data = nullptr;
	this->callbacks_.save_complete.callback = SessionManager::SaveCompleteCallback;

	this->callbacks_.shutdown_cancelled.client_data = nullptr;
	this->callbacks_.shutdown_cancelled.callback = SessionManager::ShutdownCancelledCallback;
	LOG(DEBUG1) << "session manager initialized";
}

SessionManager::~SessionManager() {
	LOG(DEBUG1) << "destroying session manager";
	IceRemoveConnectionWatch(SessionManager::ICEConnectionWatchCallback, nullptr);

	this->closeConnection();

	if(this->client_id_ != nullptr)
		free(this->client_id_);

	LOG(DEBUG1) << "session manager destroyed";
}

/* -- */
/*
 *	public
 */

const int SessionManager::openConnection(void) {
	LOG(DEBUG) << "opening session manager connection";
	std::signal(SIGINT, SessionManager::handle_signal);
	std::signal(SIGTERM, SessionManager::handle_signal);

	this->smc_conn_ = SmcOpenConnection(
							NULL,					/* network_ids_list */
							NULL,					/* context */
							1,						/* xsmp_major_rev */
							0,						/* xsmp_minor_rev */
							this->mask_,			/* mask */
							&this->callbacks_,		/* callbacks */
							this->previous_id_,		/* previous_id */
							&this->client_id_,		/* client_id_ret */
							SM_ERROR_STRING_LENGTH, /* error_string_ret max length */
							this->error_string_		/* error_string_ret */
						);

	if(this->smc_conn_ == nullptr) {
		char * s = this->error_string_;
		std::string failure("Unable to open connection to session manager : ");
		failure += s;
		throw GLogiKExcept(failure);
	}

	Status ret = IceAddConnectionWatch(SessionManager::ICEConnectionWatchCallback, nullptr);
	if( ret == 0 ) {
		this->closeConnection();
		throw GLogiKExcept("IceAddConnectionWatch failure");
	}

	this->ice_conn_ = SmcGetIceConnection(this->smc_conn_);
	this->ice_fd_ = IceConnectionNumber(this->ice_conn_);

	LOG(DEBUG) << "Session manager client ID : " << this->client_id_;

	return this->ice_fd_;
}

const bool SessionManager::isSessionAlive(void) {
	return SessionManager::still_running_;
}

void SessionManager::processICEMessages(void) {
	SessionManager::processICEMessages(this->ice_conn_);
}

/* -- */
/*
 *	private
 */

void SessionManager::closeConnection(void) {
	if(this->smc_conn_ == nullptr)
		return;

	SmcCloseStatus status = SmcCloseConnection(this->smc_conn_, 0, nullptr);
	switch( status ) {
		case SmcClosedNow:
			LOG(DEBUG2) << "ICE connection was closed";
			break;
		case SmcClosedASAP:
			LOG(DEBUG2) << "ICE connection will be freed ASAP";
			break;
		case SmcConnectionInUse:
			LOG(DEBUG2) << "ICE connection not closed because in used";
			break;
	}

}

void SessionManager::handle_signal(int sig) {
	std::ostringstream buff("caught signal : ", std::ios_base::app);
	switch( sig ) {
		case SIGINT:
		case SIGTERM:
			buff << sig << " --> bye bye";
			LOG(INFO) << buff.str();
			std::signal(SIGINT, SIG_DFL);
			std::signal(SIGTERM, SIG_DFL);
			SessionManager::still_running_ = false;
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
			LOG(DEBUG3) << "ICE messages process success";
			break;
		case IceProcessMessagesIOError:
			LOG(WARNING) << "IO error, closing ICE connection";
			IceCloseConnection(ice_conn);
			// FIXME throw ?
			break;
		case IceProcessMessagesConnectionClosed:
			LOG(DEBUG3) << "ICE connection has been closed";
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
	// FIXME
	SessionManager::processICEMessages(ice_conn);
}

void SessionManager::SaveYourselfCallback(SmcConn smc_conn, SmPointer client_data, int save_type,
	Bool shutdown, int interact_style, Bool fast)
{
	LOG(DEBUG2) << "SM save yourself call";
	// FIXME
	SmcSaveYourselfDone(smc_conn, true);
}

void SessionManager::DieCallback(SmcConn smc_conn, SmPointer client_data)
{
	LOG(DEBUG2) << "SM die call, behaves like we received SIGTERM";
	SessionManager::handle_signal(SIGTERM);
}

void SessionManager::SaveCompleteCallback(SmcConn smc_conn, SmPointer client_data)
{
	LOG(DEBUG2) << "SM save complete call";
}

void SessionManager::ShutdownCancelledCallback(SmcConn smc_conn, SmPointer client_data)
{
	LOG(DEBUG2) << "SM shutdown cancelled call";
}

} // namespace GLogiK

