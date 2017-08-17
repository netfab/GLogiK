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

#include <linux/input-event-codes.h> // KEY_UNKNOWN

#include "virtual_keyboard.h"

namespace GLogiKd
{

#define KEYS_BUFFER_LENGTH 16
#define unk	KEY_UNKNOWN

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

struct KeyEvent {
	unsigned char event_code;
	unsigned short event;
	uint64_t pressed_keys;

	KeyEvent(unsigned char c=unk, unsigned short e=3, uint64_t p = 0)
		: event_code(c), event(e), pressed_keys(p) {}
};

enum class ModifierKeys : uint8_t
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
		unsigned char previous_keys_buffer_[KEYS_BUFFER_LENGTH];
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
		void fillStandardKeysEvents(void);

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
		std::vector<KeyEvent> standard_keys_events_;

		/* USB HID Usage Tables as defined in USB specification,
		 *        Chapter 10 "Keyboard/Keypad Page (0x07)"
		 * http://www.usb.org/developers/hidpage/Hut1_12v2.pdf
		 *
		 * See linux/drivers/hid/hid-input.c
		 * and linux/input-event-codes.h
		 */
		static constexpr unsigned char hid_keyboard_[256] = {
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
		void handleModifierKeys(void);
};

} // namespace GLogiKd

#endif
