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

#ifndef SRC_BIN_DAEMON_USB_DEVICE_HPP_
#define SRC_BIN_DAEMON_USB_DEVICE_HPP_

#include <cstdint>

#include <atomic>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>

#include "DeviceID.h"

#include <libusb-1.0/libusb.h>

#include "include/keyEvent.h"
#include "virtualKeyboard.h"
#include "macrosManager.h"

#define KEYS_BUFFER_LENGTH 16

namespace GLogiK
{

class USBDevice
	:	public BusNumDeviceID
{
	public:
		USBDevice(void) = default;
		~USBDevice(void) = default;
		USBDevice(const USBDevice & dev) = delete;

		USBDevice(
			const std::string & name,
			const std::string & vendorID,
			const std::string & productID,
			const uint64_t capabilities,
			uint8_t bus,
			uint8_t num,
			const std::string & stringID);

		void operator=(const USBDevice& dev);

		unsigned int			_fatalErrors;
		std::thread::id			_keysThreadID;
		std::thread::id			_LCDThreadID;
		MacrosManager*			_pMacrosManager;
		uint64_t				_pressedRKeysMask;
		std::atomic<uint8_t>	_banksLedsMask;
		std::atomic<bool>		_exitMacroRecordMode;
		unsigned char			_pressedKeys[KEYS_BUFFER_LENGTH];
		unsigned char			_previousPressedKeys[KEYS_BUFFER_LENGTH];
		std::string				_macroKey;
		std::string				_mediaKey;
		macro_type				_newMacro;

		std::chrono::steady_clock::time_point _lastTimePoint;

		void initializeMacrosManager(
			const char* virtualKeyboardName,
			const std::vector<std::string> & keysNames
		);
		void destroyMacrosManager(void);

		const std::string & getStringID(void) const {
			return _stringID;
		}
		const bool getThreadsStatus(void) const {
			return _threadsStatus;
		}
		void deactivateThreads(void) {
			_threadsStatus = false;
		}
		const int getLastKeysInterruptTransferLength(void) const {
			return _lastKeysInterruptTransferLength;
		}
		const int getLastLCDInterruptTransferLength(void) const {
			return _lastLCDInterruptTransferLength;
		}
		void setRGBBytes(const uint8_t r, const uint8_t g, const uint8_t b);
		void getRGBBytes(uint8_t & r, uint8_t & g, uint8_t & b) const;

	private:
		friend class LibUSB;

		// TODO
		std::string _stringID;	/* [devID] */

		uint8_t _RGB[3];

		int 					_lastKeysInterruptTransferLength;
		int 					_lastLCDInterruptTransferLength;
		unsigned char			_keysEndpoint;
		unsigned char			_LCDEndpoint;
		libusb_device *			_pUSBDevice;
		libusb_device_handle *	_pUSBDeviceHandle;
		std::vector<libusb_endpoint_descriptor> _USBEndpoints;
		std::vector<int>		_toRelease;
		std::vector<int>		_toAttach;

		std::atomic<bool>	_threadsStatus;
		std::mutex			_libUSBMutex;
};

} // namespace GLogiK

#endif
