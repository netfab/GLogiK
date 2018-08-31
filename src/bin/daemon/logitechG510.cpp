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

#include "lib/shared/glogik.hpp"
#include "lib/utils/utils.hpp"

#include "logitechG510.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

const std::vector<RKey> LogitechG510::fiveBytesKeysMap = {
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

const std::vector<RKey> LogitechG510::twoBytesKeysMap = {
	{1, 0x01, Keys::GK_KEY_AUDIO_NEXT,			XF86_AUDIO_NEXT},			/* XF86AudioNext */
	{1, 0x02, Keys::GK_KEY_AUDIO_PREV,			XF86_AUDIO_PREV},			/* XF86AudioPrev */
	{1, 0x04, Keys::GK_KEY_AUDIO_STOP,			XF86_AUDIO_STOP},			/* XF86AudioStop */
	{1, 0x08, Keys::GK_KEY_AUDIO_PLAY,			XF86_AUDIO_PLAY},			/* XF86AudioPlay */
	{1, 0x10, Keys::GK_KEY_AUDIO_MUTE,			XF86_AUDIO_MUTE},			/* XF86AudioMute */
	{1, 0x20, Keys::GK_KEY_AUDIO_RAISE_VOLUME,	XF86_AUDIO_RAISE_VOLUME},	/* XF86AudioRaiseVolume */
	{1, 0x40, Keys::GK_KEY_AUDIO_LOWER_VOLUME,	XF86_AUDIO_LOWER_VOLUME},	/* XF86AudioLowerVolume */
//	{1, 0x80, Keys::GK_KEY_},
};

const std::vector<MKeyLed> LogitechG510::ledsMask = {
	{Leds::GK_LED_M1, 0x80},
	{Leds::GK_LED_M2, 0x40},
	{Leds::GK_LED_M3, 0x20},
	{Leds::GK_LED_MR, 0x10},
};

const std::vector<DeviceID> LogitechG510::knownDevices = {
	// name, vendor_id, product_id, capabilities
	{	"Logitech G510/G510s Gaming Keyboard",
		VENDOR_LOGITECH, "c22d",
		to_type(	Caps::GK_BACKLIGHT_COLOR |
					Caps::GK_MACROS_KEYS |
					Caps::GK_MEDIA_KEYS |
					Caps::GK_LCD_SCREEN )
	},
};

LogitechG510::LogitechG510()
	:	KeyboardDriver(
			8,	/* expected libusb keys interrupt read max length */
			/* libusb device initialization */
			{	1,		/* bConfigurationValue */
				1,		/* bInterfaceNumber */
				0,		/* bAlternateSetting */
				2	},	/* bNumEndpoints */
			/* KeysEventsLength */
			{	5,		/* MacrosKeys */
				2	}	/* MediaKeys */
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
	return LogitechG510::knownDevices;
}

const std::vector<std::string> & LogitechG510::getMacroKeysNames(void) const {
	KeyboardDriver::macrosKeysNames.clear();
	for (const auto & key : LogitechG510::fiveBytesKeysMap ) {
		if( key.isMacroKey )
			KeyboardDriver::macrosKeysNames.push_back(key.name);
	}
	return KeyboardDriver::macrosKeysNames;
}

/* return true if any macro key (G1-G18) is pressed  */
const bool LogitechG510::checkMacroKey(USBDevice & device) {
	for (const auto & key : LogitechG510::fiveBytesKeysMap ) {
		if( key.isMacroKey and (device._pressedRKeysMask & to_type(key.key)) ) {
			device._macroKey = key.name;
			return true;
		}
	}
	return false;
}

/* return true if any media key is pressed */
const bool LogitechG510::checkMediaKey(USBDevice & device) {
	for (const auto & key : LogitechG510::twoBytesKeysMap ) {
		if( device._pressedRKeysMask & to_type(key.key) ) {
			device._mediaKey = key.name;
			return true;
		}
	}
	return false;
}

/*
 * When pressing backlight key, 2 events are produced :
 *  - one double 5 bytes event : Keys::GK_KEY_LIGHT
 *  - one 2 bytes event with first byte equal to 0x04
 */
void LogitechG510::processKeyEvent2Bytes(USBDevice & device) {
	if (device._pressedKeys[0] == 0x02) {
		for (const auto & key : LogitechG510::twoBytesKeysMap ) {
			if( device._pressedKeys[key.index] & key.mask )
				device._pressedRKeysMask |= to_type(key.key);
		}
	}
	else if (device._pressedKeys[0] == 0x04) {
#if DEBUGGING_ON
		if( device._pressedKeys[1] == 0x04 ) {
			LOG(DEBUG3) << "backlight off";
		}
		else { /* 0x0 */
			LOG(DEBUG3) << "backlight on";
		}
#endif
	}
	else {
		GKSysLog(LOG_WARNING, WARNING, "wrong first byte value on 2 bytes event");
	}
}

void LogitechG510::processKeyEvent5Bytes(USBDevice & device) {
	if (device._pressedKeys[0] != 0x03) {
		GKSysLog(LOG_WARNING, WARNING, "wrong first byte value on 5 bytes event");
		return;
	}

	for (const auto & key : LogitechG510::fiveBytesKeysMap ) {
		if( device._pressedKeys[key.index] & key.mask )
			device._pressedRKeysMask |= to_type(key.key);
	}
}

void LogitechG510::processKeyEvent8Bytes(USBDevice & device) {
	if (device._pressedKeys[0] != 0x01) {
		GKSysLog(LOG_WARNING, WARNING, "wrong first byte value on 8 bytes event");
		return;
	}

	this->fillStandardKeysEvents(device);
}

KeyStatus LogitechG510::processKeyEvent(USBDevice & device)
{
	device._pressedRKeysMask = 0;

	switch(device.getLastKeysInterruptTransferLength()) {
		case 2:
#if DEBUGGING_ON
			LOG(DEBUG1) << device.getID() << " 2 bytes : " << this->getBytes(device);
#endif
			this->processKeyEvent2Bytes(device);
			return KeyStatus::S_KEY_PROCESSED;
			break;
		case 5:
#if DEBUGGING_ON
			LOG(DEBUG1) << "5 bytes : " << this->getBytes(device);
#endif
			this->processKeyEvent5Bytes(device);
			if( device._pressedRKeysMask == 0 ) /* skip release key events */
				return KeyStatus::S_KEY_SKIPPED;
			return KeyStatus::S_KEY_PROCESSED;
			break;
		case 8:
			/* process those events only if Macro Record Mode on */
			if( device._banksLedsMask & to_type(Leds::GK_LED_MR) ) {
#if DEBUGGING_ON
				LOG(DEBUG1) << device.getID() << " 8 bytes : processing standard key event : "
							<< this->getBytes(device);
#endif
				this->processKeyEvent8Bytes(device);
				std::copy(
						std::begin(device._pressedKeys), std::end(device._pressedKeys),
						std::begin(device._previousPressedKeys));
				return KeyStatus::S_KEY_PROCESSED;
			}
#if DEBUGGING_ON
			LOG(DEBUG1) << device.getID() << " 8 bytes : skipping standard key event : "
						<< this->getBytes(device);
#endif
			return KeyStatus::S_KEY_SKIPPED;
			break;
		default:
#if DEBUGGING_ON
			LOG(DEBUG1) << device.getID() << " not implemented: " << device.getLastKeysInterruptTransferLength() << " bytes !";
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
	LOG(DEBUG3) << device.getID() << " setting " << device.getName()
				<< " backlight color with following RGB bytes : " << getHexRGB(r, g, b);
#endif
	unsigned char data[4] = { 5, r, g, b };
	this->sendControlRequest(device, 0x305, 1, data, 4);
}

void LogitechG510::setMxKeysLeds(USBDevice & device) {
	unsigned char mask = 0;
	for (const auto & led : LogitechG510::ledsMask ) {
		if( device._banksLedsMask & to_type(led.led) )
			mask |= led.mask;
	}

#if DEBUGGING_ON
	LOG(DEBUG1) << device.getID() << " setting " << device.getName()
				<< " MxKeys leds using current mask : 0x" << std::hex << to_uint(mask);
#endif
	unsigned char data[2] = { 4, mask };
	this->sendControlRequest(device, 0x304, 1, data, 2);
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
void LogitechG510::sendUSBDeviceInitialization(USBDevice & device) {
#if DEBUGGING_ON
	LOG(INFO) << device.getID() << " sending " << device.getName() << " initialization requests";
#endif
	{
		unsigned char data[] = {
			1, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0
		};
		this->sendControlRequest(device, 0x301, 1, data, 19);
	}
	{
		unsigned char data[] = {
			0x09, 0x02, 0, 0, 0, 0, 0, 0
		};

		this->sendControlRequest(device, 0x309, 1, data, 8);
	}
}

} // namespace GLogiK

