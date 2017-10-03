/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
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

#include <algorithm>

#include <syslog.h>

#include <config.h>

#include "lib/utils/utils.h"

#include "logitechG510.h"

namespace GLogiK
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

void LogitechG510::initializeMacroKeys(const InitializedDevice & device) {
	for (const auto & k : this->five_bytes_keys_map_ ) {
		if( k.macro_key )
			this->initializeMacroKey(device, k.name);
	}
}

/* return true if the pressed key is a macro key (G1-G18)  */
const bool LogitechG510::checkMacroKey(InitializedDevice & device) {
	for (const auto & k : this->five_bytes_keys_map_ ) {
		if( k.macro_key and (device.pressed_keys & to_type(k.key)) ) {
			device.chosen_macro_key = k.name;
			return true;
		}
	}
	return false;
}

void LogitechG510::processKeyEvent2Bytes(InitializedDevice & device) {
	if (device.keys_buffer[0] != 0x02) {
		this->logWarning("warning : wrong first byte value on 2 bytes event");
		return;
	}

	for (auto k : this->two_bytes_keys_map_ ) {
		if( device.keys_buffer[k.index] & k.mask )
			device.pressed_keys |= to_type(k.key);
	}
}

void LogitechG510::processKeyEvent5Bytes(InitializedDevice & device) {
	if (device.keys_buffer[0] != 0x03) {
		this->logWarning("warning : wrong first byte value on 5 bytes event");
		return;
	}

	for (auto k : this->five_bytes_keys_map_ ) {
		if( device.keys_buffer[k.index] & k.mask )
			device.pressed_keys |= to_type(k.key);
	}
}

void LogitechG510::processKeyEvent8Bytes(InitializedDevice & device) {
	if (device.keys_buffer[0] != 0x01) {
		this->logWarning("warning : wrong first byte value on 8 bytes event");
		return;
	}

	this->fillStandardKeysEvents(device);
}

KeyStatus LogitechG510::processKeyEvent(InitializedDevice & device)
{
	device.pressed_keys = 0;

	switch(device.transfer_length) {
		case 2:
#if DEBUGGING_ON
			LOG(DEBUG1) << "2 bytes : " << this->getBytes(device);
#endif
			this->processKeyEvent2Bytes(device);
			return KeyStatus::S_KEY_PROCESSED;
			break;
		case 5:
#if DEBUGGING_ON
			LOG(DEBUG1) << "5 bytes : " << this->getBytes(device);
#endif
			this->processKeyEvent5Bytes(device);
			if( device.pressed_keys == 0 ) /* skip release key events */
				return KeyStatus::S_KEY_SKIPPED;
			return KeyStatus::S_KEY_PROCESSED;
			break;
		case 8:
			/* process those events only if Macro Record Mode on */
			if( device.current_leds_mask & to_type(Leds::GK_LED_MR) ) {
#if DEBUGGING_ON
				LOG(DEBUG1) << "8 bytes : processing standard key event : " << this->getBytes(device);
#endif
				this->processKeyEvent8Bytes(device);
				std::copy(
						std::begin(device.keys_buffer), std::end(device.keys_buffer),
						std::begin(device.previous_keys_buffer));
				return KeyStatus::S_KEY_PROCESSED;
			}
#if DEBUGGING_ON
			LOG(DEBUG1) << "8 bytes : skipping standard key event : " << this->getBytes(device);
#endif
			return KeyStatus::S_KEY_SKIPPED;
			break;
		default:
#if DEBUGGING_ON
			LOG(DEBUG1) << "not implemented: " << device.transfer_length << " bytes !";
#endif
			return KeyStatus::S_KEY_SKIPPED;
			break;
	}

	return KeyStatus::S_KEY_UNKNOWN;
}

void LogitechG510::setKeyboardColor(const InitializedDevice & device) {
#if DEBUGGING_ON
	LOG(DEBUG1) << "setting " << device.device.name << " keyboard color using rgb : 0x"
				<< std::hex << to_uint(device.rgb[0])
				<< std::hex << to_uint(device.rgb[1])
				<< std::hex << to_uint(device.rgb[2]);
#endif
	unsigned char usb_data[4] = { 5, device.rgb[0], device.rgb[1], device.rgb[2] };
	this->sendControlRequest(device.usb_handle, 0x305, 1, usb_data, 4);
}

void LogitechG510::setMxKeysLeds(const InitializedDevice & device) {
	unsigned char leds_mask = 0;
	for (auto l : this->leds_mask_ ) {
		if( device.current_leds_mask & to_type(l.led) )
			leds_mask |= l.mask;
	}

#if DEBUGGING_ON
	LOG(DEBUG1) << "setting " << device.device.name << " MxKeys leds using current mask : 0x"
				<< std::hex << to_uint(leds_mask);
#endif
	unsigned char leds_buffer[2] = { 4, leds_mask };
	this->sendControlRequest(device.usb_handle, 0x304, 1, leds_buffer, 2);
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
void LogitechG510::sendDeviceInitialization(const InitializedDevice & device) {
	unsigned char usb_data[] = {
		1, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0
	};
	unsigned char usb_data_2[] = {
		0x09, 0x02, 0, 0, 0, 0, 0, 0
	};

	LOG(INFO) << "sending " << device.device.name << " initialization requests";
	this->sendControlRequest(device.usb_handle, 0x301, 1, usb_data, 19);
	this->sendControlRequest(device.usb_handle, 0x309, 1, usb_data_2, 8);
}

} // namespace GLogiK
