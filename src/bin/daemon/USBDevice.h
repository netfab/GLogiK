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

#include <iterator>
#include <atomic>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <algorithm>

#include <libusb-1.0/libusb.h>

#include "lib/shared/keyEvent.h"
#include "macrosManager.h"

#define KEYS_BUFFER_LENGTH 16

namespace GLogiK
{

struct KeyboardDevice {
	const char* name;
	const char* vendor_id;
	const char* product_id;
};

class USBDevice
{
	public:
		USBDevice(void);
		~USBDevice(void);

		//USBDevice()=default;

		USBDevice(KeyboardDevice k, uint8_t b, uint8_t n, const std::string & id)
			:	device(k), bus(b), num(n), strID(id),
				keys_endpoint(0),
				pressed_keys(0),
				transfer_length(0),
				fatal_errors(0)
		{
			this->usb_device = nullptr;
			this->usb_handle = nullptr;
			this->macros_man = nullptr;
			std::fill_n(this->keys_buffer, KEYS_BUFFER_LENGTH, 0);
			std::fill_n(this->previous_keys_buffer, KEYS_BUFFER_LENGTH, 0);
			this->rgb[0] = 0xFF;
			this->rgb[1] = 0xFF;
			this->rgb[2] = 0xFF;
			this->listen_status = false;
			this->exit_macro_record_mode = false;
			this->current_leds_mask = 0;
		}

		void operator=(const USBDevice& dev)
		{
			this->device = dev.device;
			this->bus = dev.bus;
			this->num = dev.num;
			this->strID = dev.strID;
			this->keys_endpoint = dev.keys_endpoint;
			this->listen_status = static_cast<bool>(dev.listen_status);
			this->pressed_keys = dev.pressed_keys;
			this->usb_device = dev.usb_device;
			this->usb_handle = dev.usb_handle;
			this->to_release = dev.to_release;
			this->to_attach = dev.to_attach;
			this->macros_man = dev.macros_man;
			this->endpoints = dev.endpoints;
			this->listen_thread_id = dev.listen_thread_id;
			this->current_leds_mask = static_cast<uint8_t>(dev.current_leds_mask);
			this->transfer_length = dev.transfer_length;
			std::copy(
				std::begin(dev.keys_buffer),
				std::end(dev.keys_buffer),
				std::begin(this->keys_buffer)
			);
			std::copy(
				std::begin(dev.previous_keys_buffer),
				std::end(dev.previous_keys_buffer),
				std::begin(this->previous_keys_buffer)
			);
			this->exit_macro_record_mode = static_cast<bool>(dev.exit_macro_record_mode);
			this->standard_keys_events = dev.standard_keys_events;
			this->chosen_macro_key = dev.chosen_macro_key;
			this->last_call = dev.last_call;
			std::copy(
				std::begin(dev.rgb),
				std::end(dev.rgb),
				std::begin(this->rgb)
			);
			this->fatal_errors = dev.fatal_errors;
		}

	//protected:

	//private:
		KeyboardDevice device;
		uint8_t bus;
		uint8_t num;
		std::string strID;	/* [devID] */
		uint8_t keys_endpoint;
		std::atomic<bool> listen_status;
		uint64_t pressed_keys;
		libusb_device *usb_device;
		libusb_device_handle *usb_handle;
		std::vector<int> to_release;
		std::vector<int> to_attach;
		MacrosManager *macros_man;
		std::vector<libusb_endpoint_descriptor> endpoints;
		std::thread::id listen_thread_id;
		std::atomic<uint8_t> current_leds_mask;
		int transfer_length;
		unsigned char keys_buffer[KEYS_BUFFER_LENGTH];
		unsigned char previous_keys_buffer[KEYS_BUFFER_LENGTH];
		std::atomic<bool> exit_macro_record_mode;
		macro_t standard_keys_events;
		std::string chosen_macro_key;
		std::chrono::steady_clock::time_point last_call;
		uint8_t rgb[3];
		unsigned int fatal_errors;

};

} // namespace GLogiK

#endif
