/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
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
#include <mutex>

#include "lib/shared/glogik.hpp"
#include "lib/utils/utils.hpp"

#include "logitechG510.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

const std::vector<RKey> G510Base::fiveBytesKeysMap = {
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

	{4, 0x01, Keys::GK_KEY_L1, LCD_KEY_L1, false, true},
	{4, 0x02, Keys::GK_KEY_L2, LCD_KEY_L2, false, true},
	{4, 0x04, Keys::GK_KEY_L3, LCD_KEY_L3, false, true},
	{4, 0x08, Keys::GK_KEY_L4, LCD_KEY_L4, false, true},
	{4, 0x10, Keys::GK_KEY_L5, LCD_KEY_L5, false, true},
	{4, 0x20, Keys::GK_KEY_MUTE_HEADSET},
	{4, 0x40, Keys::GK_KEY_MUTE_MICRO},
//	{4, 0x80, Keys::GK_KEY_},
};

const std::vector<RKey> G510Base::twoBytesKeysMap = {
	{1, 0x01, Keys::GK_KEY_AUDIO_NEXT,			XF86_AUDIO_NEXT},			/* XF86AudioNext */
	{1, 0x02, Keys::GK_KEY_AUDIO_PREV,			XF86_AUDIO_PREV},			/* XF86AudioPrev */
	{1, 0x04, Keys::GK_KEY_AUDIO_STOP,			XF86_AUDIO_STOP},			/* XF86AudioStop */
	{1, 0x08, Keys::GK_KEY_AUDIO_PLAY,			XF86_AUDIO_PLAY},			/* XF86AudioPlay */
	{1, 0x10, Keys::GK_KEY_AUDIO_MUTE,			XF86_AUDIO_MUTE},			/* XF86AudioMute */
	{1, 0x20, Keys::GK_KEY_AUDIO_RAISE_VOLUME,	XF86_AUDIO_RAISE_VOLUME},	/* XF86AudioRaiseVolume */
	{1, 0x40, Keys::GK_KEY_AUDIO_LOWER_VOLUME,	XF86_AUDIO_LOWER_VOLUME},	/* XF86AudioLowerVolume */
//	{1, 0x80, Keys::GK_KEY_},
};

const std::vector<MKeyLed> G510Base::ledsMask = {
	{Leds::GK_LED_M1, 0x80},
	{Leds::GK_LED_M2, 0x40},
	{Leds::GK_LED_M3, 0x20},
	{Leds::GK_LED_MR, 0x10},
};

const std::vector<USBDeviceID> G510Base::knownDevices = {
/* -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
										{
/*								vendor */	VENDOR_LOGITECH,
/*								 model */	"G510s",
/*						  product name */	"Gaming Keyboard",
/*							 vendor ID */	VENDOR_ID_LOGITECH,
/*							product ID */	"c22d",
/*						  capabilities */	toEnumType(
												Caps::GK_BACKLIGHT_COLOR |
												Caps::GK_MACROS_KEYS |
												Caps::GK_MEDIA_KEYS |
												Caps::GK_LCD_SCREEN
											),
											1, 1, 0, 2,
/*	(libusb) interrupt read max length */	8,
/*			MacrosKeys transfer length */	5,
/*			 MediaKeys transfer length */	2,
/*			   LCDKeys transfer length */	5
										},
/* -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
										{
/*								vendor */	VENDOR_LOGITECH,
/*								 model */	"G510s",
/*						  product name */	"Gaming Keyboard",
/*							 vendor ID */	VENDOR_ID_LOGITECH,
/*			(onboard audio) product ID */	"c22e",
/*						  capabilities */	toEnumType(
												Caps::GK_BACKLIGHT_COLOR |
												Caps::GK_MACROS_KEYS |
												Caps::GK_MEDIA_KEYS |
												Caps::GK_LCD_SCREEN
											),
											1, 1, 0, 2,
/*	(libusb) interrupt read max length */	8,
/*			MacrosKeys transfer length */	5,
/*			 MediaKeys transfer length */	2,
/*			   LCDKeys transfer length */	5
										},
/* -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
};

const char* G510Base::getDriverName() const
{
	return "Logitech G510/G510s driver";
}

const uint16_t G510Base::getDriverID() const
{
	return GLOGIKD_DRIVER_ID_G510;
}

const std::vector<USBDeviceID> & G510Base::getSupportedDevices(void) const
{
	return G510Base::knownDevices;
}

const std::vector<std::string> & G510Base::getMacroKeysNames(void) const
{
	KeyboardDriver::macrosKeysNames.clear();
	for (const auto & key : G510Base::fiveBytesKeysMap ) {
		if( key.isMacroKey )
			KeyboardDriver::macrosKeysNames.push_back(key.name);
	}
	return KeyboardDriver::macrosKeysNames;
}

/* return true if any macro key (G1-G18) is pressed  */
const bool G510Base::checkMacroKey(USBDevice & device)
{
	for (const auto & key : G510Base::fiveBytesKeysMap ) {
		if( key.isMacroKey and (device._pressedRKeysMask & toEnumType(key.key)) ) {
			device._macroKey = key.name;
			return true;
		}
	}
	return false;
}

/* return true if any media key is pressed */
const bool G510Base::checkMediaKey(USBDevice & device)
{
	for (const auto & key : G510Base::twoBytesKeysMap ) {
		if( device._pressedRKeysMask & toEnumType(key.key) ) {
			device._mediaKey = key.name;
			return true;
		}
	}
	return false;
}

/* return true if any LCD key is pressed */
const bool G510Base::checkLCDKey(USBDevice & device)
{
	for (const auto & key : G510Base::fiveBytesKeysMap ) {
		if( key.isLCDKey and device._pressedRKeysMask & toEnumType(key.key) ) {
			std::lock_guard<std::mutex> lock(device._LCDMutex);
			device._LCDKey = key.name;
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
void G510Base::processKeyEvent2Bytes(USBDevice & device)
{
	GK_LOG_FUNC

	if (device._pressedKeys[0] == 0x02) {
		for (const auto & key : G510Base::twoBytesKeysMap ) {
			if( device._pressedKeys[key.index] & key.mask )
				device._pressedRKeysMask |= toEnumType(key.key);
		}
	}
	else if (device._pressedKeys[0] == 0x04) {
#if DEBUGGING_ON
		if(device._pressedKeys[1] & toEnumType(SpecialKeys::GK_KEY_BACKLIGHT_OFF)) {
			LOG(DEBUG3) << "backlight off";
		}
		else {
			LOG(DEBUG3) << "backlight on";
		}

		/* audio onboard disabled */
		if( ! (device._pressedKeys[1] & toEnumType(SpecialKeys::GK_ONBOARD_AUDIO_ON) ) )
			return;

		if(device._pressedKeys[1] & toEnumType(SpecialKeys::GK_KEY_HEADSET_OFF)) {
			LOG(DEBUG3) << "headset off";
		}
		else {
			LOG(DEBUG3) << "headset on";
		}

		if(device._pressedKeys[1] & toEnumType(SpecialKeys::GK_KEY_MICRO_OFF)) {
			LOG(DEBUG3) << "micro off";
		}
		else {
			LOG(DEBUG3) << "micro on";
		}
#endif
	}
	else {
		GKSysLog(LOG_WARNING, WARNING, "wrong first byte value on 2 bytes event");
	}
}

void G510Base::processKeyEvent5Bytes(USBDevice & device)
{
	GK_LOG_FUNC

	if (device._pressedKeys[0] != 0x03) {
		GKSysLog(LOG_WARNING, WARNING, "wrong first byte value on 5 bytes event");
		return;
	}

	for (const auto & key : G510Base::fiveBytesKeysMap ) {
		if( device._pressedKeys[key.index] & key.mask )
			device._pressedRKeysMask |= toEnumType(key.key);
	}
}

void G510Base::processKeyEvent8Bytes(USBDevice & device)
{
	GK_LOG_FUNC

	if (device._pressedKeys[0] != 0x01) {
		GKSysLog(LOG_WARNING, WARNING, "wrong first byte value on 8 bytes event");
		return;
	}

	this->fillStandardKeysEvents(device);
}

KeyStatus G510Base::processKeyEvent(USBDevice & device)
{
	GK_LOG_FUNC

	device._pressedRKeysMask = 0;

	switch(device.getLastKeysInterruptTransferLength()) {
		case 2:
#if DEBUGGING_ON && DEBUG_KEYS
			LOG(DEBUG1) << device.getID() << " 2 bytes : " << this->getBytes(device);
#endif
			this->processKeyEvent2Bytes(device);
			return KeyStatus::S_KEY_PROCESSED;
			break;
		case 5:
#if DEBUGGING_ON && DEBUG_KEYS
			LOG(DEBUG1) << "5 bytes : " << this->getBytes(device);
#endif
			this->processKeyEvent5Bytes(device);
			if( device._pressedRKeysMask == 0 ) /* skip release key events */
				return KeyStatus::S_KEY_SKIPPED;
			return KeyStatus::S_KEY_PROCESSED;
			break;
		case 8:
			/* process those events only if Macro Record Mode on */
			if( device._MxKeysLedsMask & toEnumType(Leds::GK_LED_MR) ) {
#if DEBUGGING_ON && DEBUG_KEYS
				LOG(DEBUG1) << device.getID() << " 8 bytes : processing standard key event : "
							<< this->getBytes(device);
#endif
				this->processKeyEvent8Bytes(device);
				std::copy(
						std::begin(device._pressedKeys), std::end(device._pressedKeys),
						std::begin(device._previousPressedKeys));
				return KeyStatus::S_KEY_PROCESSED;
			}
#if DEBUGGING_ON && DEBUG_KEYS
			LOG(DEBUG1) << device.getID() << " 8 bytes : skipping standard key event : "
						<< this->getBytes(device);
#endif
			return KeyStatus::S_KEY_SKIPPED;
			break;
		default:
#if DEBUGGING_ON && DEBUG_KEYS
			LOG(DEBUG1) << device.getID() << " not implemented: " << device.getLastKeysInterruptTransferLength() << " bytes !";
#endif
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
void G510Base::sendUSBDeviceInitialization(USBDevice & device)
{
	GK_LOG_FUNC

#if DEBUGGING_ON
	LOG(INFO)	<< device.getID() << " sending " << device.getFullName()
				<< " initialization requests";
#endif
	{
		const unsigned char ReportID = 0x01;
		const unsigned char data[] = {
			ReportID, 0, 0, 0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 0,
				   0, 0, 0
		};
		this->sendUSBDeviceFeatureReport(device, data, 19);
	}
	{
		const unsigned char ReportID = 0x09;
		const unsigned char data[] = {
			ReportID, 0x02, 0, 0, 0, 0, 0, 0
		};

		this->sendUSBDeviceFeatureReport(device, data, 8);
	}
}

void G510Base::setDeviceBacklightColor(
	USBDevice & device,
	const uint8_t r,
	const uint8_t g,
	const uint8_t b)
{
	GK_LOG_FUNC

	device.setRGBBytes( r, g, b );
#if DEBUGGING_ON
	LOG(DEBUG1) << device.getID() << " setting " << device.getFullName()
				<< " backlight color with following RGB bytes : " << getHexRGB(r, g, b);
#endif
	const unsigned char ReportID = 0x05;
	const unsigned char data[4] = { ReportID, r, g, b };
	this->sendUSBDeviceFeatureReport(device, data, 4);
}

void G510Base::setDeviceMxKeysLeds(USBDevice & device)
{
	GK_LOG_FUNC

	unsigned char mask = 0;
	for (const auto & led : G510Base::ledsMask ) {
		if( device._MxKeysLedsMask & toEnumType(led.led) )
			mask |= led.mask;
	}

#if DEBUGGING_ON
	LOG(DEBUG1) << device.getID() << " setting " << device.getFullName()
				<< " MxKeys leds using current mask : 0x" << std::hex << toUInt(mask);
#endif
	const unsigned char ReportID = 0x04;
	const unsigned char data[2] = { ReportID, mask };
	this->sendUSBDeviceFeatureReport(device, data, 2);
}

} // namespace GLogiK

