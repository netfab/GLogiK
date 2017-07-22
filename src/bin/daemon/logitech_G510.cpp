/*
 *
 *	This file is part of GLogiK project.
 *	GLogiKd, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2017  Fabrice Delliaux <netbox253@gmail.com>
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

#include <config.h>

#include "exception.h"
#include "include/log.h"
#include <syslog.h>

#include "logitech_G510.h"

namespace GLogiKd
{

LogitechG510::LogitechG510() {
	this->supported_devices_ = {
		// name, vendor_id, product_id
		{ "Logitech G510/G510s", VENDOR_LOGITECH, "c22d" },
		};
}

LogitechG510::~LogitechG510() {
}

void LogitechG510::initializeDevice(const KeyboardDevice & device, const unsigned int bus, const unsigned int num) {
	LOG(DEBUG3) << "Trying to initialize " << device.name << "("
				<< device.vendor_id << ":" << device.product_id << "), device "
				<< num << " on bus " << bus;
	this->initializeLibusb();
}

void LogitechG510::closeDevice(const KeyboardDevice &device, const unsigned int bus, const unsigned int num) {
	LOG(DEBUG3) << "Trying to close " << device.name << "("
				<< device.vendor_id << ":" << device.product_id << "), device "
				<< num << " on bus " << bus;
}

} // namespace GLogiKd

