/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2022  Fabrice Delliaux <netbox253@gmail.com>
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
#include "lib/shared/glogik.hpp"

#include "desktopNotification.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

#if HAVE_LIBNOTIFY

desktopNotification::desktopNotification(void)
	:	_pNotification(nullptr)
{
	notify_init(GLOGIKS_DESKTOP_SERVICE_NAME);
}

desktopNotification::desktopNotification(
	const std::string & summary,
	const std::string & body,
	const std::string & icon) : desktopNotification()
{
	this->setParameters(summary, body, icon);
}

desktopNotification::~desktopNotification(void)
{
	if(_pNotification)
		this->show();
	notify_uninit();
}

void desktopNotification::setParameters(
	const std::string & summary,
	const std::string & body,
	const std::string & icon)
{
	if(_pNotification)
		notify_notification_update(_pNotification, summary.c_str(), body.c_str(), icon.c_str());
	else
		_pNotification = notify_notification_new(summary.c_str(), body.c_str(), icon.c_str());
	//notify_notification_set_timeout(_pNotification, 10000); // 10 seconds
}

void desktopNotification::show(void)
{
	GError *notifyError = nullptr;

	if( ! notify_notification_show(_pNotification, &notifyError) ) {
		LOG(error) << "failed to send notification : " << notifyError->message;
		g_error_free(notifyError);
	}

	g_object_unref(G_OBJECT(_pNotification));
	_pNotification = nullptr;
}

#endif

} // namespace GLogiK

