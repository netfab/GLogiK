/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2026  Fabrice Delliaux <netbox253@gmail.com>
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

#include <new>
#include <algorithm>
#include <mutex>

#include "lib/shared/glogik.hpp"
#include "lib/utils/utils.hpp"

#include "G510Base.hpp"
#include "G510Detail.hpp"

namespace GLogiK
{

namespace D_G510
{

using namespace NSGKUtils;

const std::vector<USBDeviceID> G510Base::knownDevices =
{
/* -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
	{
		detail::G510_VENDOR,    /* vendor */
		"G510s",                /* model */
		"Gaming Keyboard",      /* product name */
		detail::G510_VENDOR_ID, /* vendor ID */
		"c22d",                 /* product ID */
		toEnumType(             /* device capabilities */
			Caps::GK_BACKLIGHT_COLOR |
			Caps::GK_MACROS_KEYS |
			Caps::GK_MEDIA_KEYS |
			Caps::GK_LCD_SCREEN
		),
		    /* USB_INTERFACE_DESCRIPTOR */
		1,  /* (libusb) bConfigurationValue */
		1,  /* (libusb) bInterfaceNumber */
		0,  /* (libusb) bAlternateSetting */
		2,  /* (libusb) bNumEndpoints */
		    /* -- */
		8,  /* (libusb) interrupt read max length */
		5,  /* MacrosKeys transfer length */
		2,  /* MediaKeys transfer length */
		5   /* LCDKeys transfer length */
	},
/* -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
	{
		detail::G510_VENDOR,    /* vendor */
		"G510s",                /* model */
		"Gaming Keyboard",      /* product name */
		detail::G510_VENDOR_ID, /* vendor ID */
		"c22e",                 /* product ID (onboard audio) */
		toEnumType(             /* device capabilities */
			Caps::GK_BACKLIGHT_COLOR |
			Caps::GK_MACROS_KEYS |
			Caps::GK_MEDIA_KEYS |
			Caps::GK_LCD_SCREEN
		),
		    /* USB_INTERFACE_DESCRIPTOR */
		1,  /* (libusb) bConfigurationValue */
		1,  /* (libusb) bInterfaceNumber */
		0,  /* (libusb) bAlternateSetting */
		2,  /* (libusb) bNumEndpoints */
		    /* -- */
		8,  /* (libusb) interrupt read max length */
		5,  /* MacrosKeys transfer length */
		2,  /* MediaKeys transfer length */
		5   /* LCDKeys transfer length */
	},
/* -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
};

const char* G510Base::getDriverName() const
{
	return "Logitech G510/G510s driver";
}

const std::uint16_t G510Base::getDriverID() const
{
	return detail::G510_DRIVER_ID;
}

const std::vector<USBDeviceID> & G510Base::getSupportedDevices(void) const
{
	return G510Base::knownDevices;
}

const MKeysIDArray_type G510Base::getMKeysIDArray(void) const
{
	MKeysIDArray_type ret;

	try
	{
		ret.reserve(detail::RKeys2MKeysIDMap.size());

		for (const auto & [RKey, MKeyID] : detail::RKeys2MKeysIDMap)
			ret.push_back(MKeyID);
	}
	catch( const std::length_error & e )
	{
		GKSysLogError("reserve length_error failure : ", e.what());
	}
	catch( const std::bad_alloc & e )
	{
		GKSysLogError("reserve bad_alloc failure : ", e.what());
	}

	return ret;
}

const GKeysIDArray_type G510Base::getGKeysIDArray(void) const
{
	GK_LOG_FUNC

	GKeysIDArray_type ret;

	try
	{
		ret.reserve(detail::RKeys2GKeysIDMap.size());

		for (const auto & [RKey, GKeyID] : detail::RKeys2GKeysIDMap)
			ret.push_back(GKeyID);
	}
	catch( const std::length_error & e )
	{
		GKSysLogError("reserve length_error failure : ", e.what());
	}
	catch( const std::bad_alloc & e )
	{
		GKSysLogError("reserve bad_alloc failure : ", e.what());
	}

	return ret;
}

/* return true if any G-Key (G1-G18) was pressed  */
const bool G510Base::checkDevicePressedAnyGKey(USBDevice & device)
{
	GK_LOG_FUNC

	for (const auto & [RKey, GKeyID] : detail::RKeys2GKeysIDMap)
	{
		if( device._pressedRKeysMask & toEnumType(RKey) )
		{
			device._GKeyID = GKeyID;
			return true;
		}
	}
	return false;
}

/* return true if any media key was pressed */
const bool G510Base::checkDevicePressedAnyMediaKey(USBDevice & device)
{
	for( const auto & key : detail::mediaKeys2BytesMap )
	{
		if( device._pressedRKeysMask & toEnumType(key.key) )
		{
			device._mediaKey = key.name;
			return true;
		}
	}
	return false;
}

/* return true if any LCD key was pressed */
const bool G510Base::checkDevicePressedAnyLCDKey(USBDevice & device)
{
	for( const auto & key : detail::LCDKeys5BytesMap )
	{
		if( device._pressedRKeysMask & toEnumType(key.key) )
		{
			std::lock_guard<std::mutex> lock(device._LCDMutex);
			device._LCDKey = key.name;
			return true;
		}
	}
	return false;
}

/* return true if any Mx key was pressed */
const bool G510Base::checkDevicePressedAnyMxKey(USBDevice & device)
{
	// MR and M1,M2,M3 keys are not in the same containers
	if( device._pressedRKeysMask & toEnumType(detail::RKeys::GK_KEY_MR) )
		return true;
	for( const auto & key : detail::MKeys5BytesMap )
		if( device._pressedRKeysMask & toEnumType(key.key) )
			return true;
	return false;
}

/* return true if MR key is enabled */
const bool G510Base::isDeviceMRKeyEnabled(USBDevice & device)
{
	/* is MR key enabled ? */
	return ( device._MxKeysLedsMask & toEnumType(Leds::GK_LED_MR) );
}

/*
 * When pressing backlight key, 2 events are produced :
 *  - one double 5 bytes event : detail::RKeys::GK_KEY_LIGHT
 *  - one 2 bytes event with first byte equal to 0x04
 */
void G510Base::processDeviceKeyEvent2Bytes(USBDevice & device)
{
	GK_LOG_FUNC

	if( device._pressedKeys[0] == 0x02 )
	{
		for( const auto & key : detail::mediaKeys2BytesMap )
		{
			if( device._pressedKeys[key.index] & key.mask )
				device._pressedRKeysMask |= toEnumType(key.key);
		}
	}
	else if( device._pressedKeys[0] == 0x04 )
	{
#if DEBUGGING_ON
		/* continue only when debug is on */
		if( ! GKLogging::GKDebug )
			return;

		if(device._pressedKeys[1] & toEnumType(SpecialKeys::GK_KEY_BACKLIGHT_OFF))
		{
			LOG(trace) << "backlight off";
		}
		else
		{
			LOG(trace) << "backlight on";
		}

		/* continue only when audio onboard is enabled */
		if( ! (device._pressedKeys[1] & toEnumType(SpecialKeys::GK_ONBOARD_AUDIO_ON) ) )
			return;

		if(device._pressedKeys[1] & toEnumType(SpecialKeys::GK_KEY_HEADSET_OFF))
		{
			LOG(trace) << "headset off";
		}
		else
		{
			LOG(trace) << "headset on";
		}

		if(device._pressedKeys[1] & toEnumType(SpecialKeys::GK_KEY_MICRO_OFF))
		{
			LOG(trace) << "micro off";
		}
		else
		{
			LOG(trace) << "micro on";
		}
#endif
	}
	else
	{
		GKSysLogWarning("wrong first byte value on 2 bytes event");
	}
}

void G510Base::processDeviceKeyEvent5Bytes(USBDevice & device)
{
	GK_LOG_FUNC

	if (device._pressedKeys[0] != 0x03)
	{
		GKSysLogWarning("wrong first byte value on 5 bytes event");
		return;
	}

	for(const auto & key : detail::keys5BytesMap)
	{
		if( device._pressedKeys[key.index] & key.mask )
			device._pressedRKeysMask |= toEnumType(key.key);
	}

	/* M Keys */
	for(const auto & key : detail::MKeys5BytesMap)
	{
		if( device._pressedKeys[key.index] & key.mask )
			device._pressedRKeysMask |= toEnumType(key.key);
	}

	/* G Keys */
	for(const auto & key : detail::GKeys5BytesMap)
	{
		if( device._pressedKeys[key.index] & key.mask )
			device._pressedRKeysMask |= toEnumType(key.key);
	}

	/* LCD Keys */
	for(const auto & key : detail::LCDKeys5BytesMap)
	{
		if( device._pressedKeys[key.index] & key.mask )
			device._pressedRKeysMask |= toEnumType(key.key);
	}
}

void G510Base::processDeviceKeyEvent8Bytes(USBDevice & device)
{
	GK_LOG_FUNC

	if (device._pressedKeys[0] != 0x01)
	{
		GKSysLogWarning("wrong first byte value on 8 bytes event");
		return;
	}

	this->fillDeviceStandardKeysEvents(device);
}

KeyStatus G510Base::processDeviceKeyEvent(USBDevice & device)
{
	GK_LOG_FUNC

	device._pressedRKeysMask = 0;

	switch(device.getLastKeysInterruptTransferLength())
	{
		case 2:
#if DEBUGGING_ON && DEBUG_KEYS
			GKLog3(trace, device.getID(), " 2 bytes : ", this->getBytes(device))
#endif
			this->processDeviceKeyEvent2Bytes(device);
			return KeyStatus::S_KEY_PROCESSED;
			break;
		case 5:
#if DEBUGGING_ON && DEBUG_KEYS
			GKLog3(trace, device.getID(), " 5 bytes : ", this->getBytes(device))
#endif
			this->processDeviceKeyEvent5Bytes(device);
			if( device._pressedRKeysMask == 0 ) /* skip release key events */
				return KeyStatus::S_KEY_SKIPPED;
			return KeyStatus::S_KEY_PROCESSED;
			break;
		case 8:
			/* process those events only if Macro Record Mode on */
			if( device._MxKeysLedsMask & toEnumType(Leds::GK_LED_MR) )
			{
#if DEBUGGING_ON && DEBUG_KEYS
				GKLog3(trace, device.getID(), " 8 bytes : ", this->getBytes(device))
#endif
				/* process standard key event */
				this->processDeviceKeyEvent8Bytes(device);
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
			GKLog3(trace, device.getID(), " not implemented : bytes : ",
				device.getLastKeysInterruptTransferLength())
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
		const unsigned char data[] =
		{
			ReportID, 0, 0, 0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 0,
				   0, 0, 0
		};
		this->sendUSBDeviceFeatureReport(device, data, 19);
	}
	{
		const unsigned char ReportID = 0x09;
		const unsigned char data[] =
		{
			ReportID, 0x02, 0, 0, 0, 0, 0, 0
		};

		this->sendUSBDeviceFeatureReport(device, data, 8);
	}
}

void G510Base::setDeviceBacklightColor(
	USBDevice & device,
	const std::uint8_t r,
	const std::uint8_t g,
	const std::uint8_t b)
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
	for (const auto & led : detail::ledsMask )
	{
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

/*
 * return true if leds_mask has been updated (meaning that setDeviceMxKeysLeds should be called)
 */
const bool G510Base::updateDeviceMxKeysLedsMask(USBDevice & device, bool disableMR)
{
	auto & mask = device._MxKeysLedsMask;
	bool mask_updated = false;
	device._MKeyID = MKeysID::MKEY_M0;

	/* was MR key enabled ? */
	const bool MR_ON = mask & toEnumType(Leds::GK_LED_MR);

	// lambda
	auto update_MxKey_mask = [&] (const Leds keyledmask, const MKeysID sMKey) -> void
	{
		/* was this Mx key already enabled */
		const bool Mx_ON = mask & toEnumType(keyledmask);

		/* an Mx key (M1, M2, or M3) was pressed, we must reset
		 * the mask, else two differents Mx keys LEDs could be
		 * on at the same time; this also disables Macro Record
		 * mode if MR LED was on */
		mask = 0;
		if( ! Mx_ON )
		{ /* Mx was off, enable it */
			mask |= toEnumType(keyledmask);
			device._MKeyID = sMKey;
		}
		mask_updated = true;
		device._MBankKeyPressed = true;
	};

	/* M1 key was pressed */
	if( device._pressedRKeysMask & toEnumType(detail::RKeys::GK_KEY_M1) )
		update_MxKey_mask(Leds::GK_LED_M1, MKeysID::MKEY_M1);
	/* M2 key was pressed */
	else if( device._pressedRKeysMask & toEnumType(detail::RKeys::GK_KEY_M2) )
		update_MxKey_mask(Leds::GK_LED_M2, MKeysID::MKEY_M2);
	/* M3 key was pressed */
	else if( device._pressedRKeysMask & toEnumType(detail::RKeys::GK_KEY_M3) )
		update_MxKey_mask(Leds::GK_LED_M3, MKeysID::MKEY_M3);

	/* MR key was pressed */
	if( device._pressedRKeysMask & toEnumType(detail::RKeys::GK_KEY_MR) )
	{
		if(! MR_ON)
		{ /* MR was off, enable it */
			mask |= toEnumType(Leds::GK_LED_MR);
		}
		else
		{ /* MR was on, disable it */
			mask &= ~(toEnumType(Leds::GK_LED_MR));
		}
		mask_updated = true;
	}
	else if(disableMR)
	{ /* force disable MR */
		mask &= ~(toEnumType(Leds::GK_LED_MR));
		mask_updated = true;
	}

	return mask_updated;
}

} // namespace D_G510

} // namespace GLogiK
