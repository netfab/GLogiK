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
#include <map>
#include <sstream>
#include <algorithm>
#include <thread>
#include <chrono>

#include <libusb-1.0/libusb.h>

#include <linux/input-event-codes.h>

#include <syslog.h>
#include "include/log.h"

#include "key_event.h"
#include "macros_manager.h"

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
	uint64_t pressed_keys;
	libusb_device *usb_device;
	libusb_device_handle *usb_handle;
	std::vector<int> to_release;
	std::vector<int> to_attach;
	MacrosManager *macros_man;
	std::vector<libusb_endpoint_descriptor> endpoints;
	std::thread::id listen_thread_id;
	uint8_t current_leds_mask;
	int transfer_length;
	unsigned char keys_buffer[KEYS_BUFFER_LENGTH];
	unsigned char previous_keys_buffer[KEYS_BUFFER_LENGTH];
	std::vector<KeyEvent> standard_keys_events;
	std::string chosen_macro_key;
	std::chrono::steady_clock::time_point last_call;
	unsigned char rgb[3];

	InitializedDevice()=default;

	InitializedDevice(KeyboardDevice k, uint8_t b, uint8_t n)
	  : device(k), bus(b), num(n), keys_endpoint(0), listen_status(false),
		pressed_keys(0), current_leds_mask(0), transfer_length(0)
	{
		this->usb_device = nullptr;
		this->usb_handle = nullptr;
		this->macros_man = nullptr;
		std::fill_n(this->keys_buffer, KEYS_BUFFER_LENGTH, 0);
		std::fill_n(this->previous_keys_buffer, KEYS_BUFFER_LENGTH, 0);
		this->rgb[0] = 0xFF;
		this->rgb[1] = 0xFF;
		this->rgb[2] = 0xFF;
	}
};

struct DescriptorValues {
	uint8_t b_configuration_value;
	uint8_t b_interface_number;
	uint8_t b_alternate_setting;
	uint8_t b_num_endpoints;
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

		virtual void initializeDevice(const KeyboardDevice &dev, const uint8_t bus, const uint8_t num);
		virtual void closeDevice(const KeyboardDevice &dev, const uint8_t bus, const uint8_t num);

	protected:
		std::ostringstream buffer_;
		std::vector<KeyboardDevice> supported_devices_;

		void initializeLibusb(InitializedDevice & device);

		DescriptorValues expected_usb_descriptors_;
		int interrupt_key_read_length;

		std::string getBytes(const InitializedDevice & device);
		void logWarning(const char*);

		virtual void initializeMacroKeys(const InitializedDevice & device) = 0;
		virtual KeyStatus processKeyEvent(InitializedDevice & device) = 0;
		virtual KeyStatus getPressedKeys(InitializedDevice & device);
		virtual const bool checkMacroKey(InitializedDevice & device) = 0;

		virtual void sendDeviceInitialization(const InitializedDevice & device);
		virtual void setMxKeysLeds(const InitializedDevice & device);
		virtual void setKeyboardColor(const InitializedDevice & device);

		void initializeMacroKey(const InitializedDevice & device, const char* name);
		void fillStandardKeysEvents(InitializedDevice & device);
		void sendControlRequest(libusb_device_handle * usb_handle, uint16_t wValue, uint16_t wIndex,
			unsigned char * data, uint16_t wLength);

	private:
		static bool libusb_status_;			/* is libusb initialized ? */
		static uint8_t drivers_cnt_;		/* initialized drivers counter */
		libusb_context *context_;
		libusb_device **list_;
		int8_t leds_update_event_length_;

		std::map<const std::string, InitializedDevice> initialized_devices_;
		std::vector<std::thread> threads_;

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
		void setConfiguration(InitializedDevice & device);
		void findExpectedUSBInterface(InitializedDevice & device);
		void releaseInterfaces(InitializedDevice & device);
		void attachKernelDrivers(InitializedDevice & device);
		void detachKernelDriver(InitializedDevice & device, int numInt);
		void listenLoop(const std::string devID);
		const bool updateCurrentLedsMask(InitializedDevice & device, bool force_MR_off=false);
		void updateKeyboardColor(InitializedDevice & device, const uint8_t red=0xFF,
			const uint8_t green=0xFF, const uint8_t blue=0xFF);
		void enterMacroRecordMode(InitializedDevice & device);
		void handleModifierKeys(InitializedDevice & device);
		uint16_t getTimeLapse(InitializedDevice & device);
		void runMacro(const std::string devID);
};

inline void KeyboardDriver::logWarning(const char* warning)
{
	this->buffer_.str("warning : ");
	this->buffer_ << warning;
	LOG(WARNING) << this->buffer_.str();
	syslog(LOG_WARNING, this->buffer_.str().c_str());
}

} // namespace GLogiKd

#endif
