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

#ifndef SRC_BIN_DAEMON_USB_DEVICE_HPP_
#define SRC_BIN_DAEMON_USB_DEVICE_HPP_

#include <cstdint>

#include <atomic>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>

#include "LCDScreenPluginsManager.hpp"

#include "USBDeviceID.hpp"

#include "include/base.hpp"

#include "config.h"

#include <libusb-1.0/libusb.h>

#if HAVE_HIDAPI
#include <hidapi.h>
#endif

namespace GLogiK
{

class USBDevice
	:	public USBDeviceID
{
	public:
		USBDevice(void) = default;
		~USBDevice(void) = default;

		USBDevice(const USBDevice & dev) = delete;
		USBDevice(const USBDeviceID & dev);

		void operator=(const USBDevice& dev);

		std::mutex					_LCDMutex;

#if HAVE_LIBUSB
	private:
		friend class libusb;

		std::mutex					_libUSBMutex;
#endif

	public:
		std::string					_mediaKey;
		std::string					_LCDKey;

		macro_type					_newMacro;

#if HAVE_LIBUSB
	private:
		std::vector<int>			_toRelease;
		std::vector<int>			_toAttach;
#endif

	public:
		std::uint64_t				_pressedRKeysMask;
		std::uint64_t				_LCDPluginsMask1;

	private:
		friend class USBInit;

		LCDScreenPluginsManager*	_pLCDPluginsManager;

		libusb_device*				_pUSBDevice;

#if HAVE_LIBUSB
		libusb_device_handle*		_pUSBDeviceHandle;
#elif HAVE_HIDAPI
		friend class hidapi;

		hid_device*					_pHIDDevice;
#endif

	public:
		std::thread::id				_keysThreadID;
		std::thread::id				_LCDThreadID;

		std::chrono::steady_clock::time_point
									_lastTimePoint;

		std::atomic<std::uint8_t>	_MxKeysLedsMask;
		std::atomic<bool>			_exitMacroRecordMode;

	private:
		std::atomic<bool>			_threadsStatus;
		std::atomic<bool>			_USBRequestsStatus;

		int							_lastKeysInterruptTransferLength;
		int							_lastLCDInterruptTransferLength;

		std::uint8_t _RGB[3];

	public:
		unsigned int				_fatalErrors;

		unsigned char				_pressedKeys[KEYS_BUFFER_LENGTH];
		unsigned char				_previousPressedKeys[KEYS_BUFFER_LENGTH];

#if HAVE_LIBUSB
	private:
		unsigned char				_keysEndpoint;
		unsigned char				_LCDEndpoint;

	public:
#endif

		GKeysID						_GKeyID; // G-Key

		/* -- -- -- */

	public:
		void setLCDPluginsManager(LCDScreenPluginsManager* pLCDPluginsManager)
		{
			_pLCDPluginsManager = pLCDPluginsManager;
		}
		void destroyLCDPluginsManager(void) noexcept;

		/* getters */
		LCDScreenPluginsManager* const & getLCDPluginsManager(void) const
		{
			return _pLCDPluginsManager;
		}
		const bool getThreadsStatus(void) const
		{
			return _threadsStatus;
		}
		const bool getUSBRequestsStatus(void) const
		{
			return _USBRequestsStatus;
		}
		const int getLastKeysInterruptTransferLength(void) const
		{
			return _lastKeysInterruptTransferLength;
		}
		const int getLastLCDInterruptTransferLength(void) const
		{
			return _lastLCDInterruptTransferLength;
		}
		/* -- -- -- */

		void stopThreads(void) noexcept
		{
			_threadsStatus = false;
		}
		void skipUSBRequests(void) noexcept
		{
			_USBRequestsStatus = false;
		}

		void setRGBBytes(
			const std::uint8_t r,
			const std::uint8_t g,
			const std::uint8_t b
		);
		void getRGBBytes(
			std::uint8_t & r,
			std::uint8_t & g,
			std::uint8_t & b
		) const;
};

} // namespace GLogiK

#endif
