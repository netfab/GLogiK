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

#include <map>
#include <vector>
#include <string>

#include "config.h"

#include "keyboardDriver.hpp"

#include "USBDeviceID.hpp"
#include "USBDevice.hpp"

#include "include/enums.hpp"
#include "include/base.hpp"

namespace GLogiK
{

namespace D_G510
{

namespace detail
{
	/* RKey - Recognized Keys */
	struct RKey
	{
		const std::string name;
		const RKeys key;
		const std::uint16_t index;
		const unsigned char mask;
	};

	struct MKeyLed
	{
		const Leds led;
		const unsigned char mask;
	};

	constexpr std::uint16_t G510_DRIVER_ID = ( 1 << 0 );
	constexpr char G510_VENDOR[] = "Logitech";
	constexpr char G510_VENDOR_ID[] = "046d";
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

		static const std::vector<detail::MKeyLed> ledsMask;

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
		static const std::vector<detail::RKey>    keys5BytesMap;
		static const std::vector<detail::RKey>   MKeys5BytesMap;
		static const std::vector<detail::RKey>   GKeys5BytesMap;
		static const std::vector<detail::RKey> LCDKeys5BytesMap;
		static const std::vector<detail::RKey> mediaKeys2BytesMap;
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
