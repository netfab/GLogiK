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

#include "lib/utils/utils.hpp"

#include "SIGss2s.hpp"


namespace NSGKDBus
{

using namespace NSGKUtils;

template <>
	void callbackEvent<SIGss2s>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message,
		DBusMessage* asyncContainer
	)
{
	ArgBase::fillInArguments(message);

	std::string ret;

	try {
	 const std::string arg1( ArgString::getNextStringArgument() );
	 const std::string arg2( ArgString::getNextStringArgument() );

		/* call two strings to string callback */
		ret = this->callback(arg1, arg2);
	}
	catch ( const GLogiKExcept & e ) {
		/* send error if necessary when something was wrong */
		this->sendCallbackError(connection, message, e.what());
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendStringToReply(ret);

		this->appendAsyncArgsToReply(asyncContainer);
	}
	catch ( const GLogiKExcept & e ) {
		/* delete reply object if allocated and send error reply */
		this->sendReplyError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

} // namespace NSGKDBus


