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

#include "logitech_g15.h"

#include <libg15.h>

namespace GLogiKd
{

LogitechG15::LogitechG15() : initialized(false), buffer_("", std::ios_base::app) {
	this->supported_devices_ = {
		// name, vendor_id, product_id
		{ "Logitech G510", VENDOR_LOGITECH, "c22d" },
		//{ "Logitech G510", VENDOR_LOGITECH, "c30f" }
		};

#ifdef DEBUGGING_ON
	libg15Debug(G15_LOG_WARN);
#endif
}

LogitechG15::~LogitechG15() {
	LOG(DEBUG3) << "exiting libg15";
	if( this->initialized ) {
		try {
			this->closeDevice();
		}
		catch ( const GLogiKExcept & e ) {
			syslog( LOG_ERR, e.what() );
			LOG(ERROR) << e.what();
		}
	}
}

void LogitechG15::initializeDevice(const char* vendor_id, const char* product_id) {
	LOG(DEBUG3) << "initializing libg15 device Vid:Pid - "
				<< vendor_id << ":" << product_id;

	if( this->initialized )
		throw GLogiKExcept("device already initialized");

	unsigned int vendor = std::stoul(vendor_id, nullptr, 16);
	unsigned int product = std::stoul(product_id, nullptr, 16);

	int ret = setupLibG15(vendor, product, 0);

	if ( ret != G15_NO_ERROR ) {
		this->buffer_.str("libg15 initialization failure. Return code : ");
		this->buffer_ << ret;
		throw GLogiKExcept( this->buffer_.str() );
	}

	this->initialized = true;
}

void LogitechG15::closeDevice() {
	LOG(DEBUG3) << "closing libg15 device";
	int ret = exitLibG15();
	this->initialized = false;
	if ( ret != G15_NO_ERROR ) {
		this->buffer_.str("libg15 exit failure. Return code : ");
		this->buffer_ << ret;
		throw GLogiKExcept( this->buffer_.str() );
	}
}

} // namespace GLogiKd

