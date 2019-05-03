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

#include "lib/utils/utils.hpp"

#include "GKDBusAsyncContainer.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

GKDBusAsyncContainer::GKDBusAsyncContainer(void)
	:	GKDBusMessage(nullptr, true), _numArgs(0)
{
	/* initialize fake message */
	_message = dbus_message_new(DBUS_MESSAGE_TYPE_ERROR);
	if(_message == nullptr)
		throw GKDBusMessageWrongBuild("can't allocate memory for DBus Async Container message");

	/* initialize potential arguments iterator */
	dbus_message_iter_init_append(_message, &_itMessage);
#if DEBUG_GKDBUS_SUBOBJECTS
	LOG(DEBUG2) << "DBus Async Container initialized";
#endif
}

GKDBusAsyncContainer::~GKDBusAsyncContainer()
{
	dbus_message_unref(_message);
#if DEBUG_GKDBUS_SUBOBJECTS
	LOG(DEBUG2) << "DBus Async Container destroyed";
#endif
}

DBusMessage* GKDBusAsyncContainer::getAsyncContainerPointer(void) const
{
	return _message;
}

void GKDBusAsyncContainer::incArgs(void) {
	_numArgs++;
}

const bool GKDBusAsyncContainer::isAsyncContainerEmpty(void) const
{
	return (_numArgs == 0);
}

/* --- --- --- */
/* --- --- --- */
/* --- --- --- */

thread_local GKDBusAsyncContainer* GKDBusMessageAsyncContainer::_asyncContainer(nullptr);

GKDBusMessageAsyncContainer::GKDBusMessageAsyncContainer()
{
}

GKDBusMessageAsyncContainer::~GKDBusMessageAsyncContainer()
{
}

void GKDBusMessageAsyncContainer::initializeAsyncContainer(void)
{
	if(_asyncContainer) /* sanity check */
		throw GKDBusMessageWrongBuild("DBus AsyncContainer already allocated");

	try {
		_asyncContainer = new GKDBusAsyncContainer();
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		LOG(ERROR) << "GKDBus AsyncContainer allocation failure : " << e.what();
		throw GKDBusMessageWrongBuild("allocation error");
	}
}

void GKDBusMessageAsyncContainer::destroyAsyncContainer(void)
{
	if(_asyncContainer) {
		delete _asyncContainer;
		_asyncContainer = nullptr;
	}
}

void GKDBusMessageAsyncContainer::appendAsyncString(const std::string & value)
{
	if(_asyncContainer == nullptr) { /* sanity check */
		this->initializeAsyncContainer();
	}

	_asyncContainer->appendString(value);
	_asyncContainer->incArgs();
}

void GKDBusMessageAsyncContainer::appendAsyncUInt64(const uint64_t value)
{
	if(_asyncContainer == nullptr) { /* sanity check */
		this->initializeAsyncContainer();
	}

	_asyncContainer->appendUInt64(value);
	_asyncContainer->incArgs();
}

const bool GKDBusMessageAsyncContainer::isAsyncContainerEmpty(void) const
{
	if(_asyncContainer == nullptr)
		return true;
	return _asyncContainer->isAsyncContainerEmpty();
}

DBusMessage* GKDBusMessageAsyncContainer::getAsyncContainerPointer(void) const
{
	if(_asyncContainer == nullptr)
		return nullptr;
	return _asyncContainer->getAsyncContainerPointer();
}

} // namespace NSGKDBus

