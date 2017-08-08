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

LogitechG510::LogitechG510() : KeyboardDriver(INTERRUPT_READ_MAX_LENGTH, { 1, 1, 0, 2 }) {
	this->supported_devices_ = {
		// name, vendor_id, product_id
		{ "Logitech G510/G510s", VENDOR_LOGITECH, "c22d" },
		};
}

LogitechG510::~LogitechG510() {
}

KeyStatus LogitechG510::processKeyEvent(unsigned int * pressed_keys, unsigned int actual_length) {

	switch(actual_length) {
		case 2:
			LOG(DEBUG1) << "2 bytes : " << this->getBytes(actual_length);
			return KeyStatus::S_KEY_PROCESSED;
			break;
		case 5:
			LOG(DEBUG1) << "5 bytes : " << this->getBytes(actual_length);
			return KeyStatus::S_KEY_PROCESSED;
			break;
		case 8:
#if DEBUGGING_ON
			LOG(DEBUG1) << "8 bytes : skipping standard key event : " << this->getBytes(actual_length);
#endif
			return KeyStatus::S_KEY_SKIPPED;
			break;
		default:
			LOG(DEBUG1) << "not implemented: " << actual_length << " bytes !";
			return KeyStatus::S_KEY_SKIPPED;
			break;
	}

	return KeyStatus::S_KEY_UNKNOWN;
}

/*
 *	Send G510/G510s control requests for device initialization
 *	When the keyboard is first plugged-in, the firmware is setting up the device
 *	configuration in a way where :
 *		- G1  - G12 keys produces KEY_F1 - KEY_F12 input events
 *		- G13 - G18 keys produces KEY_1  - KEY_6   input events
 *	This way, those keys are default-binded and are usable even if no driver take
 *	control of the keyboard.
 *	The following initialization disable (at least) this behavior.
 */
void LogitechG510::sendDeviceInitialization(const InitializedDevice & current_device) {
	unsigned char usb_data[] = {
		1, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0
	};
	unsigned char usb_data_2[] = {
		0x09, 0x02, 0, 0, 0, 0, 0, 0
	};

	LOG(INFO) << "sending " << current_device.device.name << " initialization requests";
	this->sendControlRequest(current_device.usb_handle, 0x301, 1, usb_data, 19);
	this->sendControlRequest(current_device.usb_handle, 0x309, 1, usb_data_2, 8);
}

} // namespace GLogiKd

