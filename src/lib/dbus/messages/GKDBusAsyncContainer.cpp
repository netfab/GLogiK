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

#include "lib/utils/utils.h"

#include "GKDBusAsyncContainer.h"

namespace NSGKDBus
{

using namespace NSGKUtils;

GKDBusAsyncContainer::GKDBusAsyncContainer(void)
	:	GKDBusMessage(nullptr, false, true), num(0)
{
	/* initialize fake message */
	this->message_ = dbus_message_new(DBUS_MESSAGE_TYPE_INVALID);
	if(this->message_ == nullptr)
		throw GKDBusMessageWrongBuild("can't allocate memory for DBus Async Container message");

	/* initialize potential arguments iterator */
	dbus_message_iter_init_append(this->message_, &this->args_it_);
#if DEBUG_GKDBUS_SUBOBJECTS
	LOG(DEBUG2) << "DBus Async Container initialized";
#endif
}

GKDBusAsyncContainer::~GKDBusAsyncContainer()
{
	dbus_message_unref(this->message_);
#if DEBUG_GKDBUS_SUBOBJECTS
	LOG(DEBUG2) << "DBus Async Container destroyed";
#endif
}

DBusMessage* GKDBusAsyncContainer::getAsyncContainerPointer(void) const
{
	return this->message_;
}

void GKDBusAsyncContainer::incArgs(void) {
	this->num++;
}

const bool GKDBusAsyncContainer::isAsyncContainerEmpty(void) const
{
	return (this->num == 0);
}

/* --- --- --- */
/* --- --- --- */
/* --- --- --- */

GKDBusAsyncContainer* GKDBusMessageAsyncContainer::container_(nullptr);

GKDBusMessageAsyncContainer::GKDBusMessageAsyncContainer()
{
}

GKDBusMessageAsyncContainer::~GKDBusMessageAsyncContainer()
{
}

void GKDBusMessageAsyncContainer::initializeAsyncContainer(void)
{
	if(this->container_) /* sanity check */
		throw GKDBusMessageWrongBuild("DBus AsyncContainer already allocated");

	try {
		this->container_ = new GKDBusAsyncContainer();
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		LOG(ERROR) << "GKDBus AsyncContainer allocation failure : " << e.what();
		throw GKDBusMessageWrongBuild("allocation error");
	}
}

void GKDBusMessageAsyncContainer::destroyAsyncContainer(void)
{
	if(this->container_) {
		delete this->container_;
		this->container_ = nullptr;
	}
}

void GKDBusMessageAsyncContainer::appendAsyncString(const std::string & value)
{
	if(this->container_ == nullptr) { /* sanity check */
		this->initializeAsyncContainer();
	}

	this->container_->appendString(value);
	this->container_->incArgs();
}

void GKDBusMessageAsyncContainer::appendAsyncUInt64(const uint64_t value)
{
	if(this->container_ == nullptr) { /* sanity check */
		this->initializeAsyncContainer();
	}

	this->container_->appendUInt64(value);
	this->container_->incArgs();
}

const bool GKDBusMessageAsyncContainer::isAsyncContainerEmpty(void) const
{
	if(this->container_ == nullptr)
		return true;
	return this->container_->isAsyncContainerEmpty();
}

DBusMessage* GKDBusMessageAsyncContainer::getAsyncContainerPointer(void) const
{
	if(this->container_ == nullptr)
		return nullptr;
	return this->container_->getAsyncContainerPointer();
}

} // namespace NSGKDBus

