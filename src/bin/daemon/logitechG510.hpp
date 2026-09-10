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

#ifndef SRC_BIN_DAEMON_LOGITECH_G510_DRIVER_HPP_
#define SRC_BIN_DAEMON_LOGITECH_G510_DRIVER_HPP_

#include <cstdint>

#include <array>
#include <map>
#include <vector>
#include <string>
#include <string_view>

#include "config.h"

#include "keyboardDriver.hpp"

#include "USBDeviceID.hpp"
#include "USBDevice.hpp"

#include "include/enums.hpp"
#include "include/base.hpp"
#include "include/RKeys.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

namespace D_G510
{

namespace detail
{
	/* RKey - Recognized Keys */
	struct RKey
	{
		constexpr RKey(
			std::string_view name,
			RKeys key,
			std::uint16_t index,
			unsigned char mask
		)	:	name(name),
				key(key),
				index(index),
				mask(static_cast<unsigned char>(mask))
		{}
		std::string_view name;
		RKeys key;
		std::uint16_t index;
		unsigned char mask;
	};

	struct MKeyLed
	{
		constexpr MKeyLed(Leds led, unsigned int mask)
			:	led(led),
				mask(static_cast<unsigned char>(mask))
		{}
		const Leds led;
		const unsigned char mask;
	};

	constexpr std::uint16_t G510_DRIVER_ID = ( 1u << 0 );
	constexpr char G510_VENDOR[] = "Logitech";
	constexpr char G510_VENDOR_ID[] = "046d";

	using R = RKey;

	inline constexpr std::array keys5BytesMap =
	{
	//  R{ "Key",           RKeys::GK_KEY_, 3, 1u << 2 },
		R{ "KeyLight",   RKeys::GK_KEY_LIGHT, 3, 1u << 3 },
		R{ "MR",            RKeys::GK_KEY_MR, 3, 1u << 7 },

		R{ "MuteHeadphones", RKeys::GK_KEY_MUTE_HEADPHONES, 4, 1u << 5 },
		R{ "MuteMicro",           RKeys::GK_KEY_MUTE_MICRO, 4, 1u << 6 },
	//	R{ "Key"            RKeys::GK_KEY_, 4, 1u << 7 },
	};

	inline constexpr std::array MKeys5BytesMap =
	{
		R{ "M1", RKeys::GK_KEY_M1, 3, 1u << 4 },
		R{ "M2", RKeys::GK_KEY_M2, 3, 1u << 5 },
		R{ "M3", RKeys::GK_KEY_M3, 3, 1u << 6 },
	};

	inline constexpr std::array GKeys5BytesMap =
	{
		R{ "G1",  RKeys::GK_KEY_G1 , 1, 1u << 0 },
		R{ "G2",  RKeys::GK_KEY_G2 , 1, 1u << 1 },
		R{ "G3",  RKeys::GK_KEY_G3 , 1, 1u << 2 },
		R{ "G4",  RKeys::GK_KEY_G4 , 1, 1u << 3 },
		R{ "G5",  RKeys::GK_KEY_G5 , 1, 1u << 4 },
		R{ "G6",  RKeys::GK_KEY_G6 , 1, 1u << 5 },
		R{ "G7",  RKeys::GK_KEY_G7 , 1, 1u << 6 },
		R{ "G8",  RKeys::GK_KEY_G8 , 1, 1u << 7 },

		R{ "G9",  RKeys::GK_KEY_G9 , 2, 1u << 0 },
		R{ "G10", RKeys::GK_KEY_G10, 2, 1u << 1 },
		R{ "G11", RKeys::GK_KEY_G11, 2, 1u << 2 },
		R{ "G12", RKeys::GK_KEY_G12, 2, 1u << 3 },
		R{ "G13", RKeys::GK_KEY_G13, 2, 1u << 4 },
		R{ "G14", RKeys::GK_KEY_G14, 2, 1u << 5 },
		R{ "G15", RKeys::GK_KEY_G15, 2, 1u << 6 },
		R{ "G16", RKeys::GK_KEY_G16, 2, 1u << 7 },

		R{ "G17", RKeys::GK_KEY_G17, 3, 1u << 0 },
		R{ "G18", RKeys::GK_KEY_G18, 3, 1u << 1 },
	};

	inline constexpr std::array LCDKeys5BytesMap =
	{
		R{ "L1", RKeys::GK_KEY_L1, 4, 1u << 0 },
		R{ "L2", RKeys::GK_KEY_L2, 4, 1u << 1 },
		R{ "L3", RKeys::GK_KEY_L3, 4, 1u << 2 },
		R{ "L4", RKeys::GK_KEY_L4, 4, 1u << 3 },
		R{ "L5", RKeys::GK_KEY_L5, 4, 1u << 4 },
	};

	inline constexpr std::array mediaKeys2BytesMap =
	{
		R{ "XF86AudioNext",        RKeys::GK_KEY_AUDIO_NEXT        , 1, 1u << 0 },
		R{ "XF86AudioPrev",        RKeys::GK_KEY_AUDIO_PREV        , 1, 1u << 1 },
		R{ "XF86AudioStop",        RKeys::GK_KEY_AUDIO_STOP        , 1, 1u << 2 },
		R{ "XF86AudioPlay",        RKeys::GK_KEY_AUDIO_PLAY        , 1, 1u << 3 },
		R{ "XF86AudioMute",        RKeys::GK_KEY_AUDIO_MUTE        , 1, 1u << 4 },
		R{ "XF86AudioRaiseVolume", RKeys::GK_KEY_AUDIO_RAISE_VOLUME, 1, 1u << 5 },
		R{ "XF86AudioLowerVolume", RKeys::GK_KEY_AUDIO_LOWER_VOLUME, 1, 1u << 6 },
	//	R{ "",                     RKeys::GK_KEY_, 1, 1u << 7 },
	};

	inline constexpr std::array<MKeyLed, 4> ledsMask =
	{
		{
			{ Leds::GK_LED_M1, 1u << 7 },
			{ Leds::GK_LED_M2, 1u << 6 },
			{ Leds::GK_LED_M3, 1u << 5 },
			{ Leds::GK_LED_MR, 1u << 4 },
		}
	};

} // namespace detail

class G510Base
{
	public:

	protected:
		G510Base(void) = default;
		virtual ~G510Base(void) = default;

		const char* getDriverName(void) const;
		const std::uint16_t getDriverID(void) const;
		const std::vector<USBDeviceID> & getSupportedDevices(void) const;
		const MKeysIDArray_type getMKeysIDArray(void) const;
		const GKeysIDArray_type getGKeysIDArray(void) const;

		/* return true if any G-Key (G1-G18) was pressed  */
		virtual const bool checkPressedAnyGKey(USBDevice & device);
		/* return true if any media key was pressed */
		virtual const bool checkPressedAnyMediaKey(USBDevice & device);
		/* return true if any LCD key was pressed */
		virtual const bool checkPressedAnyLCDKey(USBDevice & device);
		/* return true if any Mx key was pressed */
		virtual const bool checkPressedAnyMxKey(USBDevice & device);

		virtual KeyStatus processKeyEvent(USBDevice & device);

		virtual void sendUSBDeviceInitialization(USBDevice & device);

		virtual void setDeviceBacklightColor(
			USBDevice & device,
			const std::uint8_t r=0xFF,
			const std::uint8_t g=0xFF,
			const std::uint8_t b=0xFF
		);

		virtual void setDeviceMxKeysLeds(USBDevice & device);

		virtual const bool updateDeviceMxKeysLedsMask(
			USBDevice & device,
			bool disableMR=false
		);

	private:
		static const std::map<RKeys, GKeysID>  RKeys2GKeysIDMap;
		static const std::vector<USBDeviceID> knownDevices;

		void processKeyEvent2Bytes(USBDevice & device);
		void processKeyEvent5Bytes(USBDevice & device);
		void processKeyEvent8Bytes(USBDevice & device);

		virtual void sendUSBDeviceFeatureReport(
			USBDevice & device,
			const unsigned char * data,
			std::uint16_t wLength
		) = 0;

		virtual void fillStandardKeysEvents(USBDevice & device) = 0;

#if DEBUGGING_ON && DEBUG_KEYS
		virtual const std::string getBytes(const USBDevice & device) const = 0;
#endif

};

} // namespace D_G510

template <typename USBAPI>
class LogitechG510
	:	public USBKeyboardDriver<USBAPI>,
		public D_G510::G510Base
{
	public:
		LogitechG510();
		~LogitechG510();

	protected:
		const char* getDriverName(void) const override
		{
			return G510Base::getDriverName();
		}
		const std::uint16_t getDriverID(void) const override
		{
			return G510Base::getDriverID();
		}
		const std::vector<USBDeviceID> & getSupportedDevices(void) const override
		{
			return G510Base::getSupportedDevices();
		}
		const MKeysIDArray_type getMKeysIDArray(void) const override
		{
			return G510Base::getMKeysIDArray();
		}
		const GKeysIDArray_type getGKeysIDArray(void) const override
		{
			return G510Base::getGKeysIDArray();
		}

	private:
		void sendUSBDeviceFeatureReport(
			USBDevice & device,
			const unsigned char * data,
			std::uint16_t wLength
		) override
		{
			USBKeyboardDriver<USBAPI>::sendUSBDeviceFeatureReport(device, data, wLength);
		}

		void setDeviceBacklightColor(
			USBDevice & device,
			const std::uint8_t r=0xFF,
			const std::uint8_t g=0xFF,
			const std::uint8_t b=0xFF
		) override
		{
			G510Base::setDeviceBacklightColor(device, r, g,  b);
		}

		void setDeviceMxKeysLeds(USBDevice & device) override
		{
			G510Base::setDeviceMxKeysLeds(device);
		}

		const bool updateDeviceMxKeysLedsMask(
			USBDevice & device,
			bool disableMR=false
		) override
		{
			return G510Base::updateDeviceMxKeysLedsMask(device, disableMR);
		}

		void sendUSBDeviceInitialization(USBDevice & device) override
		{
			return G510Base::sendUSBDeviceInitialization(device);
		}

		/* return true if any G-Key (G1-G18) was pressed  */
		const bool checkPressedAnyGKey(USBDevice & device) override
		{
			return G510Base::checkPressedAnyGKey(device);
		}

		/* return true if any media key was pressed */
		const bool checkPressedAnyMediaKey(USBDevice & device) override
		{
			return G510Base::checkPressedAnyMediaKey(device);
		}

		/* return true if any LCD key was pressed */
		const bool checkPressedAnyLCDKey(USBDevice & device) override
		{
			return G510Base::checkPressedAnyLCDKey(device);
		}

		/* return true if any Mx key was pressed */
		virtual const bool checkPressedAnyMxKey(USBDevice & device) override
		{
			return G510Base::checkPressedAnyMxKey(device);
		}

		void fillStandardKeysEvents(USBDevice & device) override
		{
			USBKeyboardDriver<USBAPI>::fillStandardKeysEvents(device);
		}

		KeyStatus processKeyEvent(USBDevice & device) override
		{
			return G510Base::processKeyEvent(device);
		}

#if DEBUGGING_ON && DEBUG_KEYS
		const std::string getBytes(const USBDevice & device) const override
		{
			return USBKeyboardDriver<USBAPI>::getBytes(device);
		}
#endif

};


template <typename USBAPI>
LogitechG510<USBAPI>::LogitechG510()
	:	USBKeyboardDriver<USBAPI>()
{
}

template <typename USBAPI>
LogitechG510<USBAPI>::~LogitechG510()
{
}

} // namespace GLogiK

#endif
