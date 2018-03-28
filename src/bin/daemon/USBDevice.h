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

#include <libusb-1.0/libusb.h>

#include "lib/shared/keyEvent.h"
#include "macrosManager.h"

#define KEYS_BUFFER_LENGTH 16

namespace GLogiK
{

class DeviceID {
	public:
		DeviceID(
			const std::string & n,
			const std::string & v,
			const std::string & p
		)	:	name(n), vendor_id(v), product_id(p) {};
		~DeviceID(void) = default;

		const std::string & getName(void) const { return this->name; }
		const std::string & getVendorID(void) const { return this->vendor_id; }
		const std::string & getProductID(void) const { return this->product_id; }

	protected:
		DeviceID(void) = default;

	private:
		friend class USBDevice;

		std::string name;
		std::string vendor_id;
		std::string product_id;
};

class BusNumDeviceID
	:	public DeviceID
{
	public:
		const uint8_t getBus(void) const { return this->bus; }
		const uint8_t getNum(void) const { return this->num; }

	protected:
		BusNumDeviceID(void) = default;
		BusNumDeviceID(
			const std::string & n,
			const std::string & v,
			const std::string & p,
			const uint8_t b,
			const uint8_t nu
		)	:	DeviceID(n,v,p), bus(b), num(nu) {};
		~BusNumDeviceID(void) = default;

	private:
		friend class USBDevice;

		uint8_t bus;
		uint8_t num;
};

class USBDevice
	:	public BusNumDeviceID
{
	public:
		USBDevice(void) = default;
		~USBDevice(void) = default;

		USBDevice(
			const std::string & n,
			const std::string & v,
			const std::string & p,
			uint8_t b,
			uint8_t nu,
			const std::string & id);

		void operator=(const USBDevice& dev);

		unsigned int fatal_errors;
		std::thread::id listen_thread_id;
		MacrosManager *macros_man;
		uint64_t pressed_keys;
		std::atomic<uint8_t> current_leds_mask;
		std::atomic<bool> exit_macro_record_mode;
		unsigned char keys_buffer[KEYS_BUFFER_LENGTH];
		unsigned char previous_keys_buffer[KEYS_BUFFER_LENGTH];
		std::string chosen_macro_key;
		macro_t standard_keys_events;
		std::chrono::steady_clock::time_point last_call;

		const std::string & getStrID(void) const { return this->strID; }
		const bool getListeningThreadStatus(void) const { return this->listen_status; }
		void disableListeningThread(void) { this->listen_status = false; }
		const int getInterruptTransferLength(void) const { return transfer_length; }
		void setRGBBytes(const uint8_t r, const uint8_t g, const uint8_t b);
		void getRGBBytes(uint8_t & r, uint8_t & g, uint8_t & b) const;

	private:
		friend class LibUSB;

		std::string strID;	/* [devID] */
		uint8_t rgb[3];

		int transfer_length;
		uint8_t keys_endpoint;
		libusb_device *usb_device;
		libusb_device_handle *usb_handle;
		std::vector<libusb_endpoint_descriptor> endpoints;
		std::vector<int> to_release;
		std::vector<int> to_attach;

		std::atomic<bool> listen_status;
};

} // namespace GLogiK

#endif