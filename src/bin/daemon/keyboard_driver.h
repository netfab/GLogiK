/*
 *
 *	This file is part of GLogiK project.
 *	GLogiKd, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2017  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef __GLOGIKD_KEYBOARD_DRIVER_H__
#define __GLOGIKD_KEYBOARD_DRIVER_H__

#include <cstdint>

#include <string>
#include <vector>
#include <sstream>

#include <thread>

#include <libusb-1.0/libusb.h>

#include "virtual_keyboard.h"

namespace GLogiKd
{

#define KEYS_BUFFER_LENGTH 16

enum class KeyStatus
{
	S_KEY_PROCESSED = 0,
	S_KEY_TIMEDOUT,
	S_KEY_SKIPPED,
	S_KEY_UNKNOWN
};

struct KeyboardDevice {
	const char* name;
	const char* vendor_id;
	const char* product_id;
};

struct InitializedDevice {
	KeyboardDevice device;
	uint8_t bus;
	uint8_t num;
	uint8_t keys_endpoint;
	bool listen_status;
	libusb_device *usb_device;
	libusb_device_handle *usb_handle;
	VirtualKeyboard *virtual_keyboard;
	std::vector<libusb_endpoint_descriptor> endpoints;
	std::thread::id listen_thread_id;
};

struct DescriptorValues {
	uint8_t b_configuration_value;
	uint8_t b_interface_number;
	uint8_t b_alternate_setting;
	uint8_t b_num_endpoints;
};

class KeyboardDriver
{
	public:
		KeyboardDriver(int key_read_length, uint8_t event_length, DescriptorValues values);
		virtual ~KeyboardDriver();

		virtual const char* getDriverName() const = 0;
		virtual uint16_t getDriverID() const = 0;

		std::vector<KeyboardDevice> getSupportedDevices(void) const;

		virtual void initializeDevice(const KeyboardDevice &device, const uint8_t bus, const uint8_t num);
		virtual void closeDevice(const KeyboardDevice &device, const uint8_t bus, const uint8_t num);

	protected:
		std::ostringstream buffer_;
		std::vector<KeyboardDevice> supported_devices_;
		unsigned char keys_buffer_[KEYS_BUFFER_LENGTH];
		uint8_t current_leds_mask_;

		void initializeLibusb(InitializedDevice & current_device);
		VirtualKeyboard* initializeVirtualKeyboard( const char* device_name );

		DescriptorValues expected_usb_descriptors_;
		int interrupt_key_read_length;

		std::string getBytes(unsigned int actual_length);

		virtual KeyStatus processKeyEvent(uint64_t * pressed_keys, unsigned int actual_length) = 0;
		virtual KeyStatus getPressedKeys(const InitializedDevice & current_device, uint64_t * pressed_keys);
		virtual const bool checkMacroKey(const uint64_t pressed_keys) const = 0;

		virtual void sendDeviceInitialization(const InitializedDevice & current_device);
		virtual void setLeds(const InitializedDevice & current_device);

		void sendControlRequest(libusb_device_handle * usb_handle, uint16_t wValue, uint16_t wIndex,
			unsigned char * data, uint16_t wLength);

	private:
		static bool libusb_status_;			/* is libusb initialized ? */
		static uint8_t drivers_cnt_;		/* initialized drivers counter */
		libusb_context *context_;
		libusb_device **list_;
		std::vector<int> to_attach_;
		std::vector<int> to_release_;
		int8_t last_transfer_length_;
		int8_t leds_update_event_length_;

		std::vector<InitializedDevice> initialized_devices_;
		std::vector<std::thread> threads_;

		void closeLibusb(void);
		int handleLibusbError(int error_code);
		void setConfiguration(const InitializedDevice & current_device);
		void findExpectedUSBInterface(InitializedDevice & current_device);
		void releaseInterfaces(libusb_device_handle * usb_handle);
		void attachKernelDrivers(libusb_device_handle * usb_handle);
		void detachKernelDriver(libusb_device_handle * usb_handle, int numInt);
		void listenLoop(const InitializedDevice & current_device);
		void updateCurrentLedsMask(const uint64_t pressed_keys);
		void enterMacroRecordMode(const InitializedDevice & current_device);
};

} // namespace GLogiKd

#endif
