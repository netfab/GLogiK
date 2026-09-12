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

#pragma once

#include "config.h"

#include <cstdint>

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>

#include "KeyboardDriverDetail.hpp"

#include "USBAPI/USBDeviceID.hpp"
#include "USBAPI/USBDevice.hpp"

#include "include/enums.hpp"

#if GKDBUS
#include "lib/dbus/GKDBus.hpp"
#include "lib/shared/GKeysMacro.hpp"
#endif

namespace USBKeyboard::keyboard
{

// FIXME
using Caps = GLogiK::Caps;
using LCDPPArray_type = GLogiK::LCDPPArray_type;
using MKeysIDArray_type = GLogiK::MKeysIDArray_type;
using GKeysIDArray_type = GLogiK::GKeysIDArray_type;

class KeyboardDriver
#if GKDBUS
	:	private GLogiK::GKeysMacro
#endif
{
	private:
		using USBDeviceID = USBAPI::device::USBDeviceID;
		using USBDevice = USBAPI::device::USBDevice;

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
			const std::uint8_t r,
			const std::uint8_t g,
			const std::uint8_t b,
			const std::uint64_t LCDPluginsMask1
		);
		const LCDPPArray_type & getDeviceLCDPluginsProperties(const std::string & devID) const;

		/* --- */
		virtual const std::uint16_t getDriverID() const = 0;

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

		std::mutex _threadsMutex;

		std::vector<std::thread> _threads;
		std::map<std::string, USBDevice> _initializedDevices;

#if DEBUGGING_ON && DEBUG_KEYS
		const std::string getDeviceBytes(const USBDevice & device) const;
#endif

		void fillDeviceStandardKeysEvents(USBDevice & device);

	private:
#if GKDBUS
		const NSGKDBus::BusConnection & _systemBus = NSGKDBus::GKDBus::SystemBus;
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

#if GKDBUS
		void enterDeviceMacroRecordMode(USBDevice & device);
		void sendDeviceMBankSwitchSignal(USBDevice & device);
#endif

		void LCDScreenLoop(const std::string & devID);
		void listenLoop(const std::string & devID);

		/* internal */
		void notImplemented(const char* func) const;

		detail::KeyStatus getDevicePressedKeys(USBDevice & device);

		void setDeviceLCDPluginsMask(
			USBDevice & device,
			std::uint64_t mask = 0
		);

		std::uint16_t getDeviceTimeLapse(USBDevice & device);
		const std::uint8_t handleDeviceModifierKeys(
			USBDevice & device,
			const std::uint16_t interval
		);

		void checkDeviceFatalErrors(
			USBDevice & device,
			const std::string & place
		) const;

		void resetDeviceState(USBDevice & device);
		void joinDeviceThreads(USBDevice & device);
		/* --- */

		/* driver instantiation */
		virtual detail::KeyStatus processDeviceKeyEvent(USBDevice & device) = 0;
		virtual void setDeviceMxKeysLeds(USBDevice & device);
		virtual void setDeviceBacklightColor(
			USBDevice & device,
			const std::uint8_t r=0xFF,
			const std::uint8_t g=0xFF,
			const std::uint8_t b=0xFF
		);
		virtual const bool updateDeviceMxKeysLedsMask(
			USBDevice & device,
			bool disableMR=false
		);
		/* return true if any G-Key was pressed  */
		virtual const bool checkDevicePressedAnyGKey(USBDevice & device) = 0;
		/* return true if any media key was pressed */
		virtual const bool checkDevicePressedAnyMediaKey(USBDevice & device) = 0;
		/* return true if any LCD key was pressed */
		virtual const bool checkDevicePressedAnyLCDKey(USBDevice & device) = 0;
		/* return true if any Mx key was pressed */
		virtual const bool checkDevicePressedAnyMxKey(USBDevice & device) = 0;
		/* return true if MR key is enabled */
		virtual const bool isDeviceMRKeyEnabled(USBDevice & device) = 0;

	protected:
		virtual void sendUSBDeviceInitialization(USBDevice & device);
		/* --- */

};

} // namespace USBKeyboard::keyboard
