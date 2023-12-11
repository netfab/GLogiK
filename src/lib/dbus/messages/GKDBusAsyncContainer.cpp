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

#include <new>

#include "lib/utils/utils.hpp"

#include "GKDBusAsyncContainer.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

/* --- --- --- */
/*    public   */
/* --- --- --- */

void GKDBusMessageAsyncContainer::appendAsyncString(const std::string & value)
{
	this->appendString(value);
}

void GKDBusMessageAsyncContainer::appendAsyncUInt64(const uint64_t value)
{
	this->appendUInt64(value);
}

/* --- --- --- */
/*  protected  */
/* --- --- --- */

GKDBusMessageAsyncContainer::GKDBusMessageAsyncContainer(void)
	:	GKDBusMessage(nullptr, true)
{
	this->newAsyncContainer();
}

GKDBusMessageAsyncContainer::~GKDBusMessageAsyncContainer(void)
{
	this->freeAsyncContainer();
}

void GKDBusMessageAsyncContainer::resetAsyncContainer(void)
{
	this->freeAsyncContainer();
	this->newAsyncContainer();
}

DBusMessage* GKDBusMessageAsyncContainer::getAsyncContainer(void) const
{
	return _message;
}

/* --- --- --- */
/*   private   */
/* --- --- --- */

void GKDBusMessageAsyncContainer::newAsyncContainer(void)
{
	GK_LOG_FUNC

	if(_message != nullptr) {
		LOG(warning) << "async container not NULL";
	}

	/* initialize fake message */
	_message = dbus_message_new(DBUS_MESSAGE_TYPE_ERROR);
	if(_message == nullptr)
		throw GKDBusMessageWrongBuild("can't allocate memory for DBus async container message");

	/* initialize potential arguments iterator */
	dbus_message_iter_init_append(_message, &_itMessage);

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "async container initialized")
#endif
}

void GKDBusMessageAsyncContainer::freeAsyncContainer(void)
{
	GK_LOG_FUNC

	if(_message == nullptr) {
		LOG(warning) << "NULL async container";
		return;
	}

	dbus_message_unref(_message);
	_message = nullptr;

#if DEBUG_GKDBUS_SUBOBJECTS
	GKLog(trace, "async container freed")
#endif
}

} // namespace NSGKDBus

