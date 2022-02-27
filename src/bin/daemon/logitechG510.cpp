/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2022  Fabrice Delliaux <netbox253@gmail.com>
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

const std::vector<RKey> G510Base::keys5BytesMap = {
//	{ Keys::GK_KEY_,				nullptr,	3,	1 << 2 },
	{ Keys::GK_KEY_LIGHT,			nullptr,	3,	1 << 3 },
	{ Keys::GK_KEY_MR,				nullptr,	3,	1 << 7 },

	{ Keys::GK_KEY_MUTE_HEADSET,	nullptr,	4,	1 << 5 },
	{ Keys::GK_KEY_MUTE_MICRO,		nullptr,	4,	1 << 6 },
//	{ Keys::GK_KEY_,				nullptr,	4,	1 << 7 },
};

const std::vector<RKey> G510Base::MKeys5BytesMap = {
	{ Keys::GK_KEY_M1,	M_KEY_M1,	3,	1 << 4 },
	{ Keys::GK_KEY_M2,	M_KEY_M2,	3,	1 << 5 },
	{ Keys::GK_KEY_M3,	M_KEY_M3,	3,	1 << 6 },
};

const std::vector<RKey> G510Base::GKeys5BytesMap = {
	{ Keys::GK_KEY_G1,	G_KEY_G1,	1,	1 << 0 },
	{ Keys::GK_KEY_G2,	G_KEY_G2,	1,	1 << 1 },
	{ Keys::GK_KEY_G3,	G_KEY_G3,	1,	1 << 2 },
	{ Keys::GK_KEY_G4,	G_KEY_G4,	1,	1 << 3 },
	{ Keys::GK_KEY_G5,	G_KEY_G5,	1,	1 << 4 },
	{ Keys::GK_KEY_G6,	G_KEY_G6,	1,	1 << 5 },
	{ Keys::GK_KEY_G7,	G_KEY_G7,	1,	1 << 6 },
	{ Keys::GK_KEY_G8,	G_KEY_G8,	1,	1 << 7 },

	{ Keys::GK_KEY_G9,	G_KEY_G9,	2,	1 << 0 },
	{ Keys::GK_KEY_G10,	G_KEY_G10,	2,	1 << 1 },
	{ Keys::GK_KEY_G11,	G_KEY_G11,	2,	1 << 2 },
	{ Keys::GK_KEY_G12,	G_KEY_G12,	2,	1 << 3 },
	{ Keys::GK_KEY_G13,	G_KEY_G13,	2,	1 << 4 },
	{ Keys::GK_KEY_G14,	G_KEY_G14,	2,	1 << 5 },
	{ Keys::GK_KEY_G15,	G_KEY_G15,	2,	1 << 6 },
	{ Keys::GK_KEY_G16,	G_KEY_G16,	2,	1 << 7 },

	{ Keys::GK_KEY_G17,	G_KEY_G17,	3,	1 << 0 },
	{ Keys::GK_KEY_G18,	G_KEY_G18,	3,	1 << 1 },
};

const std::vector<RKey> G510Base::LCDKeys5BytesMap = {
	{ Keys::GK_KEY_L1,	LCD_KEY_L1,	4,	1 << 0 },
	{ Keys::GK_KEY_L2,	LCD_KEY_L2,	4,	1 << 1 },
	{ Keys::GK_KEY_L3,	LCD_KEY_L3,	4,	1 << 2 },
	{ Keys::GK_KEY_L4,	LCD_KEY_L4,	4,	1 << 3 },
	{ Keys::GK_KEY_L5,	LCD_KEY_L5,	4,	1 << 4 },
};

const std::vector<RKey> G510Base::twoBytesKeysMap = {
	{ Keys::GK_KEY_AUDIO_NEXT,			XF86_AUDIO_NEXT,			1,	1 << 0 },
	{ Keys::GK_KEY_AUDIO_PREV,			XF86_AUDIO_PREV,			1,	1 << 1 },
	{ Keys::GK_KEY_AUDIO_STOP,			XF86_AUDIO_STOP,			1,	1 << 2 },
	{ Keys::GK_KEY_AUDIO_PLAY,			XF86_AUDIO_PLAY,			1,	1 << 3 },
	{ Keys::GK_KEY_AUDIO_MUTE,			XF86_AUDIO_MUTE,			1,	1 << 4 },
	{ Keys::GK_KEY_AUDIO_RAISE_VOLUME,	XF86_AUDIO_RAISE_VOLUME,	1,	1 << 5 },
	{ Keys::GK_KEY_AUDIO_LOWER_VOLUME,	XF86_AUDIO_LOWER_VOLUME,	1,	1 << 6 },
//	{ Keys::GK_KEY_,					nullptr,					1,	1 << 7 },
};

const std::vector<MKeyLed> G510Base::ledsMask = {
	{ Leds::GK_LED_M1, 1 << 7 },
	{ Leds::GK_LED_M2, 1 << 6 },
	{ Leds::GK_LED_M3, 1 << 5 },
	{ Leds::GK_LED_MR, 1 << 4 },
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
	KeyboardDriver::keysNames.clear();
	for( const auto & key : G510Base::GKeys5BytesMap ) {
		KeyboardDriver::keysNames.push_back(key.name);
	}
	return KeyboardDriver::keysNames;
}

/* return true if any macro key (G1-G18) is pressed  */
const bool G510Base::checkMacroKey(USBDevice & device)
{
	for( const auto & key : G510Base::GKeys5BytesMap ) {
		if( device._pressedRKeysMask & toEnumType(key.key) ) {
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
	for( const auto & key : G510Base::LCDKeys5BytesMap ) {
		if( device._pressedRKeysMask & toEnumType(key.key) ) {
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
		/* continue only when debug is on */
		if( ! GKLogging::GKDebug )
			return;

		if(device._pressedKeys[1] & toEnumType(SpecialKeys::GK_KEY_BACKLIGHT_OFF)) {
			LOG(trace) << "backlight off";
		}
		else {
			LOG(trace) << "backlight on";
		}

		/* continue only when audio onboard is enabled */
		if( ! (device._pressedKeys[1] & toEnumType(SpecialKeys::GK_ONBOARD_AUDIO_ON) ) )
			return;

		if(device._pressedKeys[1] & toEnumType(SpecialKeys::GK_KEY_HEADSET_OFF)) {
			LOG(trace) << "headset off";
		}
		else {
			LOG(trace) << "headset on";
		}

		if(device._pressedKeys[1] & toEnumType(SpecialKeys::GK_KEY_MICRO_OFF)) {
			LOG(trace) << "micro off";
		}
		else {
			LOG(trace) << "micro on";
		}
#endif
	}
	else {
		GKSysLogWarning("wrong first byte value on 2 bytes event");
	}
}

void G510Base::processKeyEvent5Bytes(USBDevice & device)
{
	GK_LOG_FUNC

	if (device._pressedKeys[0] != 0x03) {
		GKSysLogWarning("wrong first byte value on 5 bytes event");
		return;
	}

	for(const auto & key : G510Base::keys5BytesMap) {
		if( device._pressedKeys[key.index] & key.mask )
			device._pressedRKeysMask |= toEnumType(key.key);
	}

	/* M Keys */
	for(const auto & key : G510Base::MKeys5BytesMap) {
		if( device._pressedKeys[key.index] & key.mask )
			device._pressedRKeysMask |= toEnumType(key.key);
	}

	/* G Keys */
	for(const auto & key : G510Base::GKeys5BytesMap) {
		if( device._pressedKeys[key.index] & key.mask )
			device._pressedRKeysMask |= toEnumType(key.key);
	}

	/* LCD Keys */
	for(const auto & key : G510Base::LCDKeys5BytesMap) {
		if( device._pressedKeys[key.index] & key.mask )
			device._pressedRKeysMask |= toEnumType(key.key);
	}
}

void G510Base::processKeyEvent8Bytes(USBDevice & device)
{
	GK_LOG_FUNC

	if (device._pressedKeys[0] != 0x01) {
		GKSysLogWarning("wrong first byte value on 8 bytes event");
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
			GKLog3(trace, device.getID(), " 2 bytes : ", this->getBytes(device))
#endif
			this->processKeyEvent2Bytes(device);
			return KeyStatus::S_KEY_PROCESSED;
			break;
		case 5:
#if DEBUGGING_ON && DEBUG_KEYS
			GKLog3(trace, device.getID(), " 5 bytes : ", this->getBytes(device))
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
				GKLog3(trace, device.getID(), " 8 bytes : ", this->getBytes(device))
#endif
				/* process standard key event */
				this->processKeyEvent8Bytes(device);
				std::copy(
						std::begin(device._pressedKeys), std::end(device._pressedKeys),
						std::begin(device._previousPressedKeys));
				return KeyStatus::S_KEY_PROCESSED;
			}
#if DEBUGGING_ON && DEBUG_KEYS
			GKLog3(trace, device.getID(), " 8 bytes (skipped) : ", this->getBytes(device))
#endif
			return KeyStatus::S_KEY_SKIPPED;
			break;
		default:
#if DEBUGGING_ON && DEBUG_KEYS
			GKLog3(trace, device.getID(), " not implemented : bytes : ", device.getLastKeysInterruptTransferLength())
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

	GKLog2(info, device.getID(), " sending device initialization requests")

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

	GKLog3(info, device.getID(),
		" setting device backlight color with RGB bytes : ",
		getHexRGB(r, g, b)
	)

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

	GKLog3(info, device.getID(),
		" setting MxKeys leds with mask : ", toUInt(mask)
	)

	const unsigned char ReportID = 0x04;
	const unsigned char data[2] = { ReportID, mask };
	this->sendUSBDeviceFeatureReport(device, data, 2);
}

} // namespace GLogiK

