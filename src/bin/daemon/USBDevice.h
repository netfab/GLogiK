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

#ifndef __GLOGIKD_USB_DEVICE_H__
#define __GLOGIKD_USB_DEVICE_H__

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
			const std::string & n,
			const std::string & v,
			const std::string & p,
			const uint64_t c,
			uint8_t b,
			uint8_t num,
			const std::string & id);

		void operator=(const USBDevice& dev);

		unsigned int fatal_errors;
		std::thread::id listen_thread_id;
		std::thread::id lcd_thread_id;
		MacrosManager* macros_man;
		uint64_t pressed_keys;
		std::atomic<uint8_t> current_leds_mask;
		std::atomic<bool> exit_macro_record_mode;
		unsigned char keys_buffer[KEYS_BUFFER_LENGTH];
		unsigned char previous_keys_buffer[KEYS_BUFFER_LENGTH];
		std::string chosen_macro_key;
		std::string media_key;
		macro_type standard_keys_events;
		std::chrono::steady_clock::time_point last_call;

		void initializeMacrosManager(
			const char* vk_name,
			const std::vector<std::string> & keys_names
		);
		void destroyMacrosManager(void);

		const std::string & getStrID(void) const { return this->strID; }
		const bool getThreadsStatus(void) const { return _threadsStatus; }
		void deactivateThreads(void) { _threadsStatus = false; }
		const int getLastKeysInterruptTransferLength(void) const { return _lastKeysInterruptTransferLength; }
		const int getLastLCDInterruptTransferLength(void) const { return _lastLCDInterruptTransferLength; }
		void setRGBBytes(const uint8_t r, const uint8_t g, const uint8_t b);
		void getRGBBytes(uint8_t & r, uint8_t & g, uint8_t & b) const;

	private:
		friend class LibUSB;

		// TODO
		std::string strID;	/* [devID] */

		uint8_t _RGB[3];

		int _lastKeysInterruptTransferLength;
		int _lastLCDInterruptTransferLength;
		unsigned char _keysEndpoint;
		unsigned char _LCDEndpoint;
		libusb_device * _pUSBDevice;
		libusb_device_handle * _pUSBDeviceHandle;
		std::vector<libusb_endpoint_descriptor> _USBEndpoints;
		std::vector<int> _toRelease;
		std::vector<int> _toAttach;

		std::atomic<bool> _threadsStatus;
		std::mutex _libUSBMutex;
};

} // namespace GLogiK

#endif
