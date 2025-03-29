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

#ifndef SRC_LIB_SHARED_SESSION_MANAGER_HPP_
#define SRC_LIB_SHARED_SESSION_MANAGER_HPP_

#include <X11/SM/SMlib.h>

#define SM_ERROR_STRING_LENGTH 255

namespace GLogiK
{

class SessionManager
{
	public:
		SessionManager(void);
		virtual ~SessionManager(void);

		const int openConnection(void);
		const bool isSessionAlive(void);
		void processICEMessages(void);

	protected:

	private:
		SmcConn _pSMCConnexion;
		IceConn _pICEConnexion;

		SmcCallbacks _callbacks;

		unsigned long _mask = 0;
		int _ICEfd = -1;

		char* _pPreviousID = nullptr;
		char* _pClientID = nullptr;
		char  _errorString[SM_ERROR_STRING_LENGTH+1] = { 0 };

		void closeConnection(void);

		static bool stillRunning;
		static void handleSignal(int signum);

		static void processICEMessages(
			IceConn ice_conn
		);
		static void ICEConnectionWatchCallback(
			IceConn ice_conn,
			IcePointer client_data,
			Bool opening,
			IcePointer *watch_data
		);

		static void SaveYourselfCallback(
			SmcConn smc_conn,
			SmPointer client_data,
			int save_type,
			Bool shutdown,
			int interact_style,
			Bool fast
		);
		static void DieCallback(
			SmcConn smc_conn,
			SmPointer client_data
		);
		static void SaveCompleteCallback(
			SmcConn smc_conn,
			SmPointer client_data
		);
		static void ShutdownCancelledCallback(
			SmcConn smc_conn,
			SmPointer client_data
		);
};

} // namespace GLogiK

#endif
