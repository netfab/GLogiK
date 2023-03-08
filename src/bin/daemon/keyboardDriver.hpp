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

#ifndef SRC_BIN_DAEMON_KEYBOARD_DRIVER_HPP_
#define SRC_BIN_DAEMON_KEYBOARD_DRIVER_HPP_

#include <cstdint>

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>

#include <linux/input-event-codes.h>

#include <config.h>

#if GKDBUS
#include "lib/dbus/GKDBus.hpp"
#include "lib/shared/GKeysMacro.hpp"
#endif

#include "USBDeviceID.hpp"
#include "USBDevice.hpp"

#include "include/enums.hpp"
#include "include/base.hpp"
#include "include/LCDPP.hpp"

#define DEVICE_LISTENING_THREAD_MAX_ERRORS 3
#define unk	KEY_UNKNOWN

namespace GLogiK
{

enum class KeyStatus : uint8_t
{
	S_KEY_PROCESSED = 0,
	S_KEY_TIMEDOUT,
	S_KEY_SKIPPED,
	S_KEY_UNKNOWN
};

enum class GKModifierKeys : uint8_t
{
	GK_KEY_LEFT_CTRL	= 1 << 0,
	GK_KEY_LEFT_SHIFT	= 1 << 1,
	GK_KEY_LEFT_ALT		= 1 << 2,
	GK_KEY_LEFT_META	= 1 << 3,
	GK_KEY_RIGHT_CTRL	= 1 << 4,
	GK_KEY_RIGHT_SHIFT	= 1 << 5,
	GK_KEY_RIGHT_ALT	= 1 << 6,
	GK_KEY_RIGHT_META	= 1 << 7
};

struct ModifierKey {
	const uint8_t code;			/* event code */
	const GKModifierKeys key;	/* modifier key */
};


class KeyboardDriver
#if GKDBUS
	:	private GKeysMacro
#endif
{
	public:
		virtual ~KeyboardDriver(void) = default;

#if GKDBUS
		void setDBus(NSGKDBus::GKDBus* pDBus);
#endif

		static const bool checkDeviceCapability(const USBDeviceID & device, Caps toCheck);

		/* --- */
		const bool getDeviceThreadsStatus(const std::string & devID) const;

		void resetDeviceState(const USBDeviceID & det);

		void setDeviceActiveConfiguration(
			const std::string & devID,
			const uint8_t r,
			const uint8_t g,
			const uint8_t b,
			const uint64_t LCDPluginsMask1
		);
		const LCDPPArray_type & getDeviceLCDPluginsProperties(
			const std::string & devID
		) const;

		/* --- */
		virtual const uint16_t getDriverID() const = 0;

		virtual void initializeDevice(const USBDeviceID & det);
		virtual void openDevice(const USBDeviceID & det);
		virtual void closeDevice(
			const USBDeviceID & det,
			const bool skipUSBRequests = false
		) noexcept;

		virtual const std::vector<USBDeviceID> & getSupportedDevices(void) const = 0;
		virtual const MKeysIDArray_type getMKeysIDArray(void) const = 0;
		virtual const GKeysIDArray_type getGKeysIDArray(void) const = 0;

	protected:
		KeyboardDriver(void) = default;

		/* USB HID Usage Tables as defined in USB specification,
		 *        Chapter 10 "Keyboard/Keypad Page (0x07)"
		 *
		 * https://www.usb.org/document-library/hid-usage-tables-112
		 *      -> https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf
		 *
		 * See linux/drivers/hid/hid-input.c
		 * and linux/input-event-codes.h
		 */
		static constexpr unsigned char hidKeyboard[256] = {
			  0,  0,  0,  0, 30, 48, 46, 32, 18, 33, 34, 35, 23, 36, 37, 38,
			 50, 49, 24, 25, 16, 19, 31, 20, 22, 47, 17, 45, 21, 44,  2,  3,
			  4,  5,  6,  7,  8,  9, 10, 11, 28,  1, 14, 15, 57, 12, 13, 26,
			 27, 43, 43, 39, 40, 41, 51, 52, 53, 58, 59, 60, 61, 62, 63, 64,
			 65, 66, 67, 68, 87, 88, 99, 70,119,110,102,104,111,107,109,106,
			105,108,103, 69, 98, 55, 74, 78, 96, 79, 80, 81, 75, 76, 77, 71,
			 72, 73, 82, 83, 86,127,116,117,183,184,185,186,187,188,189,190,
			191,192,193,194,134,138,130,132,128,129,131,137,133,135,136,113,
			115,114,unk,unk,unk,121,unk, 89, 93,124, 92, 94, 95,unk,unk,unk,
			122,123, 90, 91, 85,unk,unk,unk,unk,unk,unk,unk,111,unk,unk,unk,
			unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,
			unk,unk,unk,unk,unk,unk,179,180,unk,unk,unk,unk,unk,unk,unk,unk,
			unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,unk,
			unk,unk,unk,unk,unk,unk,unk,unk,111,unk,unk,unk,unk,unk,unk,unk,
			 29, 42, 56,125, 97, 54,100,126,164,166,165,163,161,115,114,113,
			150,158,159,128,136,177,178,176,142,152,173,140,unk,unk,unk,unk
		};

		std::mutex _threadsMutex;

		std::vector<std::thread> _threads;
		std::map<std::string, USBDevice> _initializedDevices;

#if DEBUGGING_ON && DEBUG_KEYS
		const std::string getBytes(const USBDevice & device) const;
#endif

		void fillStandardKeysEvents(USBDevice & device);

	private:
#if GKDBUS
		NSGKDBus::GKDBus* _pDBus;
#endif

		/* USBAPI */
		virtual int performUSBDeviceKeysInterruptTransfer(
			USBDevice & device,
			unsigned int timeout
		) = 0;
		virtual int performUSBDeviceLCDScreenInterruptTransfer(
			USBDevice & device,
			const unsigned char * buffer,
			int bufferLength,
			unsigned int timeout
		) = 0;

		virtual void openUSBDevice(USBDevice & device) = 0;
		virtual void closeUSBDevice(USBDevice & device) = 0;
		/* --- */

		static const std::vector< ModifierKey > modifierKeys;

#if GKDBUS
		void enterMacroRecordMode(USBDevice & device);
#endif

		void LCDScreenLoop(const std::string & devID);
		void listenLoop(const std::string & devID);

		/* internal */
		void notImplemented(const char* func) const;

		KeyStatus getPressedKeys(USBDevice & device);

		const bool updateDeviceMxKeysLedsMask(USBDevice & device, bool disableMR=false);
		void setDeviceLCDPluginsMask(USBDevice & device, uint64_t mask = 0);

		uint16_t getTimeLapse(USBDevice & device);
		const uint8_t handleModifierKeys(USBDevice & device, const uint16_t interval);

		void checkDeviceFatalErrors(USBDevice & device, const std::string & place) const;

		void resetDeviceState(USBDevice & device);
		void joinDeviceThreads(USBDevice & device);
		/* --- */

		/* driver instantiation */
		virtual KeyStatus processKeyEvent(USBDevice & device) = 0;
		virtual void setDeviceMxKeysLeds(USBDevice & device);
		virtual void setDeviceBacklightColor(
			USBDevice & device,
			const uint8_t r=0xFF,
			const uint8_t g=0xFF,
			const uint8_t b=0xFF
		);
		virtual const bool checkGKey(USBDevice & device) = 0;
		virtual const bool checkMediaKey(USBDevice & device) = 0;
		virtual const bool checkLCDKey(USBDevice & device) = 0;

	protected:
		virtual void sendUSBDeviceInitialization(USBDevice & device);
		/* --- */

};

template <typename USBAPI>
class USBKeyboardDriver
	:	public USBAPI,
		public KeyboardDriver
{
	public:
		virtual ~USBKeyboardDriver();

		virtual const char* getDriverName() const = 0;

	protected:
		USBKeyboardDriver(void);

	private:
		/* USBAPI */
		int performUSBDeviceKeysInterruptTransfer(
			USBDevice & device,
			unsigned int timeout
		) override {
			return USBAPI::performUSBDeviceKeysInterruptTransfer(device, timeout);
		}

		int performUSBDeviceLCDScreenInterruptTransfer(
			USBDevice & device,
			const unsigned char * buffer,
			int bufferLength,
			unsigned int timeout
		) override {
			return USBAPI::performUSBDeviceLCDScreenInterruptTransfer(device, buffer, bufferLength, timeout);
		}

		void openUSBDevice(USBDevice & device) override {
			USBAPI::openUSBDevice(device);
		}

		void closeUSBDevice(USBDevice & device) override {
			device.destroyLCDPluginsManager();

			USBAPI::closeUSBDevice(device);
		}
		/* --- */
};

template <typename USBAPI>
USBKeyboardDriver<USBAPI>::USBKeyboardDriver(void)
{
}

template <typename USBAPI>
USBKeyboardDriver<USBAPI>::~USBKeyboardDriver()
{
}




} // namespace GLogiK

#endif
