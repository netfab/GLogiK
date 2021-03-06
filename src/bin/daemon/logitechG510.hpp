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

#ifndef SRC_BIN_DAEMON_LOGITECH_G510_DRIVER_HPP_
#define SRC_BIN_DAEMON_LOGITECH_G510_DRIVER_HPP_

#include <cstdint>

#include <vector>
#include <string>

#include <config.h>

#include "keyboardDriver.hpp"

#include "USBDeviceID.hpp"
#include "USBDevice.hpp"

#include "include/enums.hpp"

namespace GLogiK
{

#define GLOGIKD_DRIVER_ID_G510 ( 1 << 0 )

#define	   VENDOR_LOGITECH "Logitech"
#define	VENDOR_ID_LOGITECH "046d"

/* RKey - Recognized Keys */
struct RKey
{
	const uint16_t index;
	const unsigned char mask;
	const Keys key;
	const char* const name;
	const bool isMacroKey;
	const bool isLCDKey;

	RKey(	const uint16_t i,
			const unsigned char m,
			const Keys k,
			const char* const n=nullptr,
			const bool bMacro=false,
			const bool bLCD=false )
		:	index(i),
			mask(m),
			key(k),
			name(n),
			isMacroKey(bMacro),
			isLCDKey(bLCD)	{}
};

struct MKeyLed
{
	Leds led;
	unsigned char mask;
};



class G510Base
{
	public:

	protected:
		G510Base(void) = default;
		virtual ~G510Base(void) = default;

		const char* getDriverName(void) const;
		const uint16_t getDriverID(void) const;
		const std::vector<USBDeviceID> & getSupportedDevices(void) const;
		const std::vector<std::string> & getMacroKeysNames(void) const;

		static const std::vector<MKeyLed> ledsMask;

		virtual const bool checkMacroKey(USBDevice & device);
		virtual const bool checkMediaKey(USBDevice & device);
		virtual const bool checkLCDKey(USBDevice & device);

		virtual KeyStatus processKeyEvent(USBDevice & device);

		virtual void sendUSBDeviceInitialization(USBDevice & device);

		virtual void setDeviceBacklightColor(
			USBDevice & device,
			const uint8_t r=0xFF,
			const uint8_t g=0xFF,
			const uint8_t b=0xFF
		);

		virtual void setDeviceMxKeysLeds(USBDevice & device);

	private:
		static const std::vector<RKey> fiveBytesKeysMap;
		static const std::vector<RKey> twoBytesKeysMap;
		static const std::vector<USBDeviceID> knownDevices;

		void processKeyEvent2Bytes(USBDevice & device);
		void processKeyEvent5Bytes(USBDevice & device);
		void processKeyEvent8Bytes(USBDevice & device);

		virtual void sendControlRequest(
			USBDevice & device,
			const unsigned char * data,
			uint16_t wLength
		) = 0;

		virtual void fillStandardKeysEvents(USBDevice & device) = 0;

#if DEBUGGING_ON && DEBUG_KEYS
		virtual const std::string getBytes(const USBDevice & device) const = 0;
#endif

};


template <typename USBAPI>
class LogitechG510
	:	public USBKeyboardDriver<USBAPI>,
		public G510Base
{
	public:
		LogitechG510();
		~LogitechG510();

	protected:
		const char* getDriverName(void) const override {
			return G510Base::getDriverName();
		}
		const uint16_t getDriverID(void) const override {
			return G510Base::getDriverID();
		}
		const std::vector<USBDeviceID> & getSupportedDevices(void) const override {
			return G510Base::getSupportedDevices();
		}
		const std::vector<std::string> & getMacroKeysNames(void) const override {
			return G510Base::getMacroKeysNames();
		}

	private:
		void sendControlRequest(
			USBDevice & device,
			const unsigned char * data,
			uint16_t wLength
		) override {
			USBKeyboardDriver<USBAPI>::sendControlRequest(device, data, wLength);
		}

		void setDeviceBacklightColor(
			USBDevice & device,
			const uint8_t r=0xFF,
			const uint8_t g=0xFF,
			const uint8_t b=0xFF
		) override {
			G510Base::setDeviceBacklightColor(device, r, g,  b);
		}

		void setDeviceMxKeysLeds(USBDevice & device) override {
			G510Base::setDeviceMxKeysLeds(device);
		}

		void sendUSBDeviceInitialization(USBDevice & device) override {
			return G510Base::sendUSBDeviceInitialization(device);
		}

		const bool checkMacroKey(USBDevice & device) override {
			return G510Base::checkMacroKey(device);
		}
		const bool checkMediaKey(USBDevice & device) override {
			return G510Base::checkMediaKey(device);
		}
		const bool checkLCDKey(USBDevice & device) override {
			return G510Base::checkLCDKey(device);
		}

		void fillStandardKeysEvents(USBDevice & device) override {
			USBKeyboardDriver<USBAPI>::fillStandardKeysEvents(device);
		}

		KeyStatus processKeyEvent(USBDevice & device) override {
			return G510Base::processKeyEvent(device);
		}

#if DEBUGGING_ON && DEBUG_KEYS
		const std::string getBytes(const USBDevice & device) const override {
			return USBKeyboardDriver<USBAPI>::getBytes(device);
		}
#endif

};


template <typename USBAPI>
LogitechG510<USBAPI>::LogitechG510()
	:	USBKeyboardDriver<USBAPI>() {
}

template <typename USBAPI>
LogitechG510<USBAPI>::~LogitechG510() {
}



} // namespace GLogiK

#endif
