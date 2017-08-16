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

LogitechG510::LogitechG510() :
	KeyboardDriver(INTERRUPT_READ_MAX_LENGTH, TRANSFER_LENGTH_FOR_LEDS_UPDATE, { 1, 1, 0, 2 })
{
	this->supported_devices_ = {
		// name, vendor_id, product_id
		{ "Logitech G510/G510s", VENDOR_LOGITECH, "c22d" },
		};
}

LogitechG510::~LogitechG510() {
}

/* return true if the pressed key is a macro key (G1-G18)  */
const bool LogitechG510::checkMacroKey(const uint64_t pressed_keys) const {
	for (auto k : this->five_bytes_keys_map_ ) {
		if( k.macro_key and (pressed_keys & to_type(k.key)) )
			return true;
	}
	return false;
}

void LogitechG510::processKeyEvent2Bytes(uint64_t * pressed_keys) {
	*pressed_keys = 0;
	if (this->keys_buffer_[0] != 0x02) {
		this->buffer_.str("warning : wrong first byte value on 2 bytes event");
		LOG(WARNING) << this->buffer_.str();
		syslog(LOG_WARNING, this->buffer_.str().c_str());
		return;
	}

	for (auto k : this->two_bytes_keys_map_ ) {
		if( this->keys_buffer_[k.index] & k.mask )
			*pressed_keys |= to_type(k.key);
	}
}

void LogitechG510::processKeyEvent5Bytes(uint64_t * pressed_keys) {
	if (this->keys_buffer_[0] != 0x03) {
		this->buffer_.str("warning : wrong first byte value on 5 bytes event");
		LOG(WARNING) << this->buffer_.str();
		syslog(LOG_WARNING, this->buffer_.str().c_str());
		return;
	}

	for (auto k : this->five_bytes_keys_map_ ) {
		if( this->keys_buffer_[k.index] & k.mask )
			*pressed_keys |= to_type(k.key);
	}
}

void LogitechG510::processKeyEvent8Bytes(uint64_t * pressed_keys) {
	if (this->keys_buffer_[0] != 0x01) {
		this->buffer_.str("warning : wrong first byte value on 8 bytes event");
		LOG(WARNING) << this->buffer_.str();
		syslog(LOG_WARNING, this->buffer_.str().c_str());
		return;
	}

	this->fillStandardKeyEvents();
}

KeyStatus LogitechG510::processKeyEvent(uint64_t * pressed_keys, unsigned int actual_length) {
	*pressed_keys = 0;

	switch(actual_length) {
		case 2:
#if DEBUGGING_ON
			LOG(DEBUG1) << "2 bytes : " << this->getBytes(actual_length);
#endif
			this->processKeyEvent2Bytes(pressed_keys);
			return KeyStatus::S_KEY_PROCESSED;
			break;
		case 5:
#if DEBUGGING_ON
			LOG(DEBUG1) << "5 bytes : " << this->getBytes(actual_length);
#endif
			this->processKeyEvent5Bytes(pressed_keys);
			if( *pressed_keys == 0 ) /* skip release key events */
				return KeyStatus::S_KEY_SKIPPED;
			return KeyStatus::S_KEY_PROCESSED;
			break;
		case 8:
			/* process those events only if Macro Record Mode on */
			if( this->current_leds_mask_ & to_type(Leds::GK_LED_MR) ) {
#if DEBUGGING_ON
				LOG(DEBUG1) << "8 bytes : processing standard key event : " << this->getBytes(actual_length);
#endif
				this->processKeyEvent8Bytes(pressed_keys);
				std::copy(
						std::begin(this->keys_buffer_), std::end(this->keys_buffer_),
						std::begin(this->previous_keys_buffer_));
				return KeyStatus::S_KEY_PROCESSED;
			}
#if DEBUGGING_ON
			LOG(DEBUG1) << "8 bytes : skipping standard key event : " << this->getBytes(actual_length);
#endif
			return KeyStatus::S_KEY_SKIPPED;
			break;
		default:
#if DEBUGGING_ON
			LOG(DEBUG1) << "not implemented: " << actual_length << " bytes !";
#endif
			return KeyStatus::S_KEY_SKIPPED;
			break;
	}

	return KeyStatus::S_KEY_UNKNOWN;
}

void LogitechG510::setLeds(const InitializedDevice & current_device) {
	unsigned char leds_mask = 0;
	for (auto l : this->leds_mask_ ) {
		if( this->current_leds_mask_ & to_type(l.led) )
			leds_mask |= l.mask;
	}

	LOG(DEBUG1) << "setting " << current_device.device.name << " M-Keys leds using current mask : 0x"
				<< std::hex << to_uint(leds_mask);
	unsigned char leds_buffer[2] = { 4, leds_mask };
	this->sendControlRequest(current_device.usb_handle, 0x304, 1, leds_buffer, 2);
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

