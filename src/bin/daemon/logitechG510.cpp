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

#include <algorithm>

#include "lib/utils/utils.h"

#include "logitechG510.h"

namespace GLogiK
{

using namespace NSGKUtils;

const std::vector< R_Key > LogitechG510::five_bytes_keys_map_ = {
	{1, 0x01, Keys::GK_KEY_G1,  "G1",  true},
	{1, 0x02, Keys::GK_KEY_G2,  "G2",  true},
	{1, 0x04, Keys::GK_KEY_G3,  "G3",  true},
	{1, 0x08, Keys::GK_KEY_G4,  "G4",  true},
	{1, 0x10, Keys::GK_KEY_G5,  "G5",  true},
	{1, 0x20, Keys::GK_KEY_G6,  "G6",  true},
	{1, 0x40, Keys::GK_KEY_G7,  "G7",  true},
	{1, 0x80, Keys::GK_KEY_G8,  "G8",  true},

	{2, 0x01, Keys::GK_KEY_G9,  "G9",  true},
	{2, 0x02, Keys::GK_KEY_G10, "G10", true},
	{2, 0x04, Keys::GK_KEY_G11, "G11", true},
	{2, 0x08, Keys::GK_KEY_G12, "G12", true},
	{2, 0x10, Keys::GK_KEY_G13, "G13", true},
	{2, 0x20, Keys::GK_KEY_G14, "G14", true},
	{2, 0x40, Keys::GK_KEY_G15, "G15", true},
	{2, 0x80, Keys::GK_KEY_G16, "G16", true},

	{3, 0x01, Keys::GK_KEY_G17, "G17", true},
	{3, 0x02, Keys::GK_KEY_G18, "G18", true},
//	{3, 0x04, Keys::GK_KEY_},
	{3, 0x08, Keys::GK_KEY_LIGHT},
	{3, 0x10, Keys::GK_KEY_M1},
	{3, 0x20, Keys::GK_KEY_M2},
	{3, 0x40, Keys::GK_KEY_M3},
	{3, 0x80, Keys::GK_KEY_MR},

	{4, 0x01, Keys::GK_KEY_L1},
	{4, 0x02, Keys::GK_KEY_L2},
	{4, 0x04, Keys::GK_KEY_L3},
	{4, 0x08, Keys::GK_KEY_L4},
	{4, 0x10, Keys::GK_KEY_L5},
	{4, 0x20, Keys::GK_KEY_MUTE_HEADSET},
	{4, 0x40, Keys::GK_KEY_MUTE_MICRO},
//	{4, 0x80, Keys::GK_KEY_},
};

const std::vector< R_Key > LogitechG510::two_bytes_keys_map_ = {
	{1, 0x01, Keys::GK_KEY_AUDIO_NEXT},				/* XF86AudioNext */
	{1, 0x02, Keys::GK_KEY_AUDIO_PREV},				/* XF86AudioPrev */
	{1, 0x04, Keys::GK_KEY_AUDIO_STOP},				/* XF86AudioStop */
	{1, 0x08, Keys::GK_KEY_AUDIO_PLAY},				/* XF86AudioPlay */
	{1, 0x10, Keys::GK_KEY_AUDIO_MUTE},				/* XF86AudioMute */
	{1, 0x20, Keys::GK_KEY_AUDIO_RAISE_VOLUME},		/* XF86AudioRaiseVolume */
	{1, 0x40, Keys::GK_KEY_AUDIO_LOWER_VOLUME},		/* XF86AudioLowerVolume */
//	{1, 0x80, Keys::GK_KEY_},
};

const std::vector< M_Key_Led_Mask > LogitechG510::leds_mask_ = {
	{Leds::GK_LED_M1, 0x80},
	{Leds::GK_LED_M2, 0x40},
	{Leds::GK_LED_M3, 0x20},
	{Leds::GK_LED_MR, 0x10},
};

const std::vector<DeviceID> LogitechG510::supported_devices_ = {
	// name, vendor_id, product_id, capabilities
	{ "Logitech G510/G510s Gaming Keyboard", VENDOR_LOGITECH, "c22d", to_type(Caps::GK_BACKLIGHT_COLOR|Caps::GK_MACROS_KEYS) },
};

LogitechG510::LogitechG510()
	:	KeyboardDriver(
			8,	/* expected libusb interrupt read max length */
			/* libusb device initialization */
			{	1,		/* bConfigurationValue */
				1,		/* bInterfaceNumber */
				0,		/* bAlternateSetting */
				2	},	/* bNumEndpoints */
			/* EventsLength */
			{	5,		/* MacrosKeys */
				2	}	/* MultimediaKeys */
		)
{
}

LogitechG510::~LogitechG510() {
}

const char* LogitechG510::getDriverName() const {
	return "Logitech G510/G510s driver";
}

const uint16_t LogitechG510::getDriverID() const {
	return GLOGIKD_DRIVER_ID_G510;
}

const std::vector<DeviceID> & LogitechG510::getSupportedDevices(void) const {
	return LogitechG510::supported_devices_;
}

const std::vector<std::string> & LogitechG510::getMacroKeysNames(void) const {
	this->keys_names_.clear();
	for (const auto & k : LogitechG510::five_bytes_keys_map_ ) {
		if( k.macro_key )
			this->keys_names_.push_back(k.name);
	}
	return this->keys_names_;
}

/* return true if the pressed key is a macro key (G1-G18)  */
const bool LogitechG510::checkMacroKey(USBDevice & device) {
	for (const auto & k : LogitechG510::five_bytes_keys_map_ ) {
		if( k.macro_key and (device.pressed_keys & to_type(k.key)) ) {
			device.chosen_macro_key = k.name;
			return true;
		}
	}
	return false;
}

void LogitechG510::processKeyEvent2Bytes(USBDevice & device) {
	if (device.keys_buffer[0] != 0x02) {
		GKSysLog(LOG_WARNING, WARNING, "wrong first byte value on 2 bytes event");
		return;
	}

	for (const auto & k : LogitechG510::two_bytes_keys_map_ ) {
		if( device.keys_buffer[k.index] & k.mask )
			device.pressed_keys |= to_type(k.key);
	}
}

void LogitechG510::processKeyEvent5Bytes(USBDevice & device) {
	if (device.keys_buffer[0] != 0x03) {
		GKSysLog(LOG_WARNING, WARNING, "wrong first byte value on 5 bytes event");
		return;
	}

	for (const auto & k : LogitechG510::five_bytes_keys_map_ ) {
		if( device.keys_buffer[k.index] & k.mask )
			device.pressed_keys |= to_type(k.key);
	}
}

void LogitechG510::processKeyEvent8Bytes(USBDevice & device) {
	if (device.keys_buffer[0] != 0x01) {
		GKSysLog(LOG_WARNING, WARNING, "wrong first byte value on 8 bytes event");
		return;
	}

	this->fillStandardKeysEvents(device);
}

KeyStatus LogitechG510::processKeyEvent(USBDevice & device)
{
	device.pressed_keys = 0;

	switch(device.getLastInterruptTransferLength()) {
		case 2:
#if DEBUGGING_ON
			LOG(DEBUG1) << device.getStrID() << " 2 bytes : " << this->getBytes(device);
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
				LOG(DEBUG1) << device.getStrID() << " 8 bytes : processing standard key event : "
							<< this->getBytes(device);
#endif
				this->processKeyEvent8Bytes(device);
				std::copy(
						std::begin(device.keys_buffer), std::end(device.keys_buffer),
						std::begin(device.previous_keys_buffer));
				return KeyStatus::S_KEY_PROCESSED;
			}
#if DEBUGGING_ON
			LOG(DEBUG1) << device.getStrID() << " 8 bytes : skipping standard key event : "
						<< this->getBytes(device);
#endif
			return KeyStatus::S_KEY_SKIPPED;
			break;
		default:
#if DEBUGGING_ON
			LOG(DEBUG1) << device.getStrID() << " not implemented: " << device.getLastInterruptTransferLength() << " bytes !";
#endif
			return KeyStatus::S_KEY_SKIPPED;
			break;
	}

	return KeyStatus::S_KEY_UNKNOWN;
}

void LogitechG510::setDeviceBacklightColor(
	USBDevice & device,
	const uint8_t r,
	const uint8_t g,
	const uint8_t b)
{
	device.setRGBBytes( r, g, b );
#if DEBUGGING_ON
	LOG(DEBUG3) << device.getStrID() << " setting " << device.getName()
				<< " backlight color with following RGB bytes : " << getHexRGB(r, g, b);
#endif
	unsigned char usb_data[4] = { 5, r, g, b };
	this->sendControlRequest(device, 0x305, 1, usb_data, 4);
}

void LogitechG510::setMxKeysLeds(const USBDevice & device) {
	unsigned char leds_mask = 0;
	for (const auto & l : LogitechG510::leds_mask_ ) {
		if( device.current_leds_mask & to_type(l.led) )
			leds_mask |= l.mask;
	}

#if DEBUGGING_ON
	LOG(DEBUG1) << device.getStrID() << " setting " << device.getName()
				<< " MxKeys leds using current mask : 0x" << std::hex << to_uint(leds_mask);
#endif
	unsigned char leds_buffer[2] = { 4, leds_mask };
	this->sendControlRequest(device, 0x304, 1, leds_buffer, 2);
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
void LogitechG510::sendUSBDeviceInitialization(const USBDevice & device) {
	unsigned char usb_data[] = {
		1, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0
	};
	unsigned char usb_data_2[] = {
		0x09, 0x02, 0, 0, 0, 0, 0, 0
	};

#if DEBUGGING_ON
	LOG(INFO) << device.getStrID() << " sending " << device.getName() << " initialization requests";
#endif
	this->sendControlRequest(device, 0x301, 1, usb_data, 19);
	this->sendControlRequest(device, 0x309, 1, usb_data_2, 8);
}

} // namespace GLogiK

