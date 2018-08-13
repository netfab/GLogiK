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

#include <thread>
#include <chrono>

#include "volumeNotification.h"

namespace GLogiK
{

std::atomic<unsigned int> VolumeNotification::cnt_(0);

VolumeNotification::VolumeNotification(void)
	:	pNotification_(nullptr),
		is_initted_(false),
		timeout_(5000)
{
}

VolumeNotification::~VolumeNotification(void)
{
	if( this->is_initted_ )
		notify_uninit();
}

void VolumeNotification::init(
	const std::string & app_name,
	int timeout)
{
	if( ! this->is_initted_ ) {
		this->is_initted_ = notify_init( app_name.c_str() );
		this->pNotification_ = notify_notification_new("hello", nullptr, nullptr);
		this->timeout_ = timeout;
		this->setTimeout(timeout);
	}
}

const bool VolumeNotification::updateProperties(
	const std::string & summary,
	const std::string & body,
	const std::string & icon)
{

	const char* b = nullptr;
	if( ! body.empty() )
		b = body.c_str();

	const char* i = nullptr;
	if( ! icon.empty() )
		i = icon.c_str();

	return notify_notification_update(this->pNotification_, summary.c_str(), b, i);
}

const bool VolumeNotification::show() {
	const bool ret = notify_notification_show(this->pNotification_, 0);
	if(ret) {
		VolumeNotification::cnt_++;
		std::thread closing_thread(&VolumeNotification::maybeClose, this);
		closing_thread.detach();
	}
	return ret;
}

const bool VolumeNotification::close() {
	return notify_notification_close(this->pNotification_, 0);
}

void VolumeNotification::maybeClose(void) {
	std::this_thread::sleep_for(std::chrono::milliseconds(this->timeout_ + 200));
	VolumeNotification::cnt_--;
	if( VolumeNotification::cnt_ == 0 )
		this->close();
}

void VolumeNotification::setTimeout(int timeout)
{
	/* timeout in milliseconds, may be ignored by some
	 * notifications daemons implementations */
	notify_notification_set_timeout(this->pNotification_, timeout);
}


} // namespace GLogiK

