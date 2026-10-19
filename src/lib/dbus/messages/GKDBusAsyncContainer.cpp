/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2026  Fabrice Delliaux <netbox253@gmail.com>
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

#include "config.h"

#include <dbus/dbus-protocol.h> // DBUS_MESSAGE_TYPE_ERROR

#include "lib/utils/utils.hpp"

#include "GKDBusMessage.hpp"
#include "GKDBusAsyncContainer.hpp"

namespace GKDBUS::Messages
{

using namespace NSGKUtils;

/* --- --- --- */
/*    public   */
/* --- --- --- */

void GKDBusMessageAsyncContainer::appendAsyncString(const std::string & value)
{
	this->appendString(value);
}

void GKDBusMessageAsyncContainer::appendAsyncUInt64(const std::uint64_t value)
{
	this->appendUInt64(value);
}

/* --- --- --- */
/*  protected  */
/* --- --- --- */

GKDBusMessageAsyncContainer::GKDBusMessageAsyncContainer()
	:	GKDBusMessage(nullptr, true)
{
	this->newAsyncContainer();
}

GKDBusMessageAsyncContainer::~GKDBusMessageAsyncContainer()
{
	this->freeAsyncContainer();
}

void GKDBusMessageAsyncContainer::resetAsyncContainer()
{
	this->freeAsyncContainer();
	this->newAsyncContainer();
}

DBusMessage* GKDBusMessageAsyncContainer::getAsyncContainer() const
{
	return _message;
}

/* --- --- --- */
/*   private   */
/* --- --- --- */

void GKDBusMessageAsyncContainer::newAsyncContainer()
{
	GK_LOG_FUNC

	if(_message != nullptr)
	{
		LOG(warning) << "async container not NULL";
	}

	/* initialize fake message */
	_message = dbus_message_new(DBUS_MESSAGE_TYPE_ERROR);
	if(_message == nullptr)
		throw GKDBusMessageWrongBuild("can't allocate memory for DBus async container message");

	/* initialize potential arguments iterator */
	dbus_message_iter_init_append(_message, &_itMessage);

#if DEBUG_GKDBUS
	GKLog(trace, "async container initialized")
#endif
}

void GKDBusMessageAsyncContainer::freeAsyncContainer()
{
	GK_LOG_FUNC

	if(_message == nullptr)
	{
		LOG(warning) << "NULL async container";
		return;
	}

	dbus_message_unref(_message);
	_message = nullptr;

#if DEBUG_GKDBUS
	GKLog(trace, "async container freed")
#endif
}

} // namespace GKDBUS::Messages

