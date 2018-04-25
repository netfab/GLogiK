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

#ifndef __GLOGIK_VOLUME_NOTIFICATION_H__
#define __GLOGIK_VOLUME_NOTIFICATION_H__

#include <atomic>
#include <string>

#include <libnotify/notify.h>

namespace GLogiK
{

class VolumeNotification
{
	public:
		VolumeNotification(void);
		~VolumeNotification(void);

		void init(
			const std::string & app_name,
			int timeout
		);
		void updateProperties(
			const std::string & summary,
			const std::string & body = "",
			const std::string & icon = "");
		const bool show(void);
		const bool close(void);

	protected:

	private:
		NotifyNotification* pNotification_;
		bool is_initted_;
		int timeout_;

		static std::atomic<unsigned int> cnt_;

		void setTimeout(int timeout);
		void maybeClose(void);
};

} // namespace GLogiK

#endif

