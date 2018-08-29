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

#ifndef SRC_BIN_SERVICE_VOLUME_NOTIFICATION_HPP_
#define SRC_BIN_SERVICE_VOLUME_NOTIFICATION_HPP_

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
			const std::string & appName,
			int timeout
		);
		const bool updateProperties(
			const std::string & summary,
			const std::string & body = "",
			const std::string & icon = "");
		const bool show(void);

	protected:

	private:
		NotifyNotification* _pNotification;
		bool _isInitted;
		int _timeout;

		static std::atomic<unsigned int> counter;

		void setTimeout(int timeout);
		void maybeClose(void);
		const bool close(void);
};

} // namespace GLogiK

#endif

