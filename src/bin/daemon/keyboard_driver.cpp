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

#include <iostream>
#include <bitset>

#include <utility>

#include <config.h>

#include "exception.h"

#include "daemon_control.h"
#include "keyboard_driver.h"

#include "include/enums.h"
#include "globals.h"

namespace GLogiKd
{

bool KeyboardDriver::libusb_status_ = false;
uint8_t KeyboardDriver::drivers_cnt_ = 0;

constexpr unsigned char KeyboardDriver::hid_keyboard_[256];

KeyboardDriver::KeyboardDriver(int key_read_length, uint8_t event_length, DescriptorValues values) :
		buffer_("", std::ios_base::app), context_(nullptr) {
	this->expected_usb_descriptors_ = values;
	this->leds_update_event_length_ = event_length;

	if( key_read_length > KEYS_BUFFER_LENGTH ) {
		this->logWarning("interrupt read length too large, set it to max buffer length");
		key_read_length = KEYS_BUFFER_LENGTH;
	}

	this->interrupt_key_read_length = key_read_length;
	KeyboardDriver::drivers_cnt_++;
}

KeyboardDriver::~KeyboardDriver() {
	KeyboardDriver::drivers_cnt_--;
	if (KeyboardDriver::libusb_status_ and KeyboardDriver::drivers_cnt_ == 0)
		this->closeLibusb();
}

std::vector<KeyboardDevice> KeyboardDriver::getSupportedDevices(void) const {
	return this->supported_devices_;
}

void KeyboardDriver::initializeLibusb(InitializedDevice & device) {
	LOG(DEBUG4) << "initializing libusb";
	int ret_value = libusb_init( &(this->context_) );
	if ( this->handleLibusbError(ret_value) ) {
		throw GLogiKExcept("libusb initialization failure");
	}

	KeyboardDriver::libusb_status_ = true;

	libusb_device **list;
	int num_devices = libusb_get_device_list(this->context_, &(list));
	if( num_devices < 0 ) {
		this->handleLibusbError(num_devices);
		throw GLogiKExcept("error getting USB devices list");
	}

	for (int idx = 0; idx < num_devices; ++idx) {
		device.usb_device = list[idx];
		if( (int)libusb_get_bus_number(device.usb_device) == device.bus and
			(int)libusb_get_device_address(device.usb_device) == device.num ) {
			break;
		}
		device.usb_device = nullptr;
	}

	if( device.usb_device == nullptr ) {
		this->buffer_.str("libusb cannot find device ");
		this->buffer_ << device.num << " on bus " << device.bus;
		libusb_free_device_list(list, 1);
		throw GLogiKExcept(this->buffer_.str());
	}

	LOG(DEBUG3) << "libusb found device " << to_uint(device.num)
				<< " on bus " << to_uint(device.bus);

	ret_value = libusb_open( device.usb_device, &(device.usb_handle) );
	if( this->handleLibusbError(ret_value) ) {
		libusb_free_device_list(list, 1);
		throw GLogiKExcept("opening device failure");
	}

	libusb_free_device_list(list, 1);
}

void KeyboardDriver::closeLibusb(void) {
	LOG(DEBUG4) << "closing libusb";
	if( this->initialized_devices_.size() != 0 ) /* sanity check */
		this->logWarning("closing libusb with opened device(s) !");
	libusb_exit(this->context_);
	KeyboardDriver::libusb_status_ = false;
}

int KeyboardDriver::handleLibusbError(int error_code) {
	switch(error_code) {
		case LIBUSB_SUCCESS:
			break;
		default:
			this->buffer_.str("libusb error (");
			this->buffer_ << libusb_error_name(error_code) << ") : "
					<< libusb_strerror((libusb_error)error_code);
			LOG(ERROR) << this->buffer_.str();
			syslog(LOG_ERR, this->buffer_.str().c_str());
			break;
	}

	return error_code;
}

void KeyboardDriver::releaseInterfaces(libusb_device_handle * usb_handle) {
	int ret = 0;
	for(auto it = this->to_release_.begin(); it != this->to_release_.end();) {
		int numInt = (*it);
		LOG(DEBUG1) << "trying to release claimed interface " << numInt;
		ret = libusb_release_interface(usb_handle, numInt); /* release */
		if( this->handleLibusbError(ret) ) {
			this->buffer_.str("failed to release interface ");
			this->buffer_ << numInt;
			LOG(ERROR) << this->buffer_.str();
			syslog(LOG_ERR, this->buffer_.str().c_str());
		}
		it++;
	}
	this->to_release_.clear();
}

void KeyboardDriver::attachKernelDrivers(libusb_device_handle * usb_handle) {
	int ret = 0;
	for(auto it = this->to_attach_.begin(); it != this->to_attach_.end();) {
		int numInt = (*it);
		LOG(DEBUG1) << "trying to attach kernel driver to interface " << numInt;
		ret = libusb_attach_kernel_driver(usb_handle, numInt); /* attaching */
		if( this->handleLibusbError(ret) ) {
			this->buffer_.str("failed to attach kernel driver to interface ");
			this->buffer_ << numInt;
			LOG(ERROR) << this->buffer_.str();
			syslog(LOG_ERR, this->buffer_.str().c_str());
		}
		it++;
	}
	this->to_attach_.clear();
}

std::string KeyboardDriver::getBytes(const InitializedDevice & device, unsigned int actual_length) {
	if( actual_length == 0 )
		return "";
	std::ostringstream s;
	s << std::hex << to_uint(device.keys_buffer[0]);
	for(unsigned int x = 1; x < actual_length; x++) {
		s << ", " << std::hex << to_uint(device.keys_buffer[x]);
	}
	return s.str();
}

void KeyboardDriver::initializeMacroKey(const InitializedDevice & device, const char* name) {
	device.macros_man->initializeMacroKey(name);
}

KeyStatus KeyboardDriver::getPressedKeys(InitializedDevice & device, uint64_t * pressed_keys) {
	int actual_length = 0;

	std::fill_n(device.keys_buffer, KEYS_BUFFER_LENGTH, 0);

	int ret = libusb_interrupt_transfer(device.usb_handle, device.keys_endpoint,
		(unsigned char*)device.keys_buffer, this->interrupt_key_read_length, &actual_length, 10);

	switch(ret) {
		case 0:
			device.last_transfer_length = actual_length;
			if( actual_length > 0 ) {
#if DEBUGGING_ON
				LOG(DEBUG)	<< "exp. rl: " << this->interrupt_key_read_length
							<< " act_l: " << actual_length << ", xBuf[0]: "
							<< std::hex << to_uint(device.keys_buffer[0]);
#endif
				return this->processKeyEvent(device, pressed_keys, actual_length);
			}
			break;
		case LIBUSB_ERROR_TIMEOUT:
#if DEBUGGING_ON
			LOG(DEBUG5) << "timeout reached";
#endif
			return KeyStatus::S_KEY_TIMEDOUT;
			break;
		default:
			LOG(DEBUG) << "getPressedKeys interrupt read error";
			this->handleLibusbError(ret);
			return KeyStatus::S_KEY_SKIPPED;
			break;
	}

	return KeyStatus::S_KEY_TIMEDOUT;
}

void KeyboardDriver::setLeds(const InitializedDevice & device) {
	this->logWarning("setLeds not implemented");
}

void KeyboardDriver::updateCurrentLedsMask(InitializedDevice & device, const uint64_t pressed_keys) {
	uint8_t & mask = device.current_leds_mask;
	/* is macro record mode enabled ? */
	bool MR_ON = mask & to_type(Leds::GK_LED_MR);
	bool Mx_ON = false;

	if( pressed_keys & to_type(Keys::GK_KEY_M1) ) {
		Mx_ON = mask & to_type(Leds::GK_LED_M1);
		mask = 0;
		device.macros_man->setCurrentActiveProfile(MemoryBank::MACROS_M0);
		if( ! Mx_ON ) {
			mask |= to_type(Leds::GK_LED_M1);
			device.macros_man->setCurrentActiveProfile(MemoryBank::MACROS_M1);
		}
	}
	else if( pressed_keys & to_type(Keys::GK_KEY_M2) ) {
		Mx_ON = mask & to_type(Leds::GK_LED_M2);
		mask = 0;
		device.macros_man->setCurrentActiveProfile(MemoryBank::MACROS_M0);
		if( ! Mx_ON ) {
			mask |= to_type(Leds::GK_LED_M2);
			device.macros_man->setCurrentActiveProfile(MemoryBank::MACROS_M2);
		}
	}
	else if( pressed_keys & to_type(Keys::GK_KEY_M3) ) {
		Mx_ON = mask & to_type(Leds::GK_LED_M3);
		mask = 0;
		device.macros_man->setCurrentActiveProfile(MemoryBank::MACROS_M0);
		if( ! Mx_ON ) {
			mask |= to_type(Leds::GK_LED_M3);
			device.macros_man->setCurrentActiveProfile(MemoryBank::MACROS_M3);
		}
	}

	if( pressed_keys & to_type(Keys::GK_KEY_MR) ) {
		if(! MR_ON) { /* MR off, enable it */
			mask |= to_type(Leds::GK_LED_MR);
		}
		else { /* MR on, disable it */
			mask &= ~(to_type(Leds::GK_LED_MR));
		}
	}
}

void KeyboardDriver::handleModifierKeys(const InitializedDevice & device) {
	if( device.previous_keys_buffer[1] == device.keys_buffer[1] )
		return; /* nothing changed here */

	KeyEvent e;
	uint8_t diff = 0;

	/* some modifier keys were released */
	if( device.previous_keys_buffer[1] > device.keys_buffer[1] ) {
		diff = device.previous_keys_buffer[1] - device.keys_buffer[1];
		e.event = EventValue::EVENT_KEY_RELEASE;
	}
	/* some modifier keys were pressed */
	else {
		diff = device.keys_buffer[1] - device.previous_keys_buffer[1];
		e.event = EventValue::EVENT_KEY_PRESS;
	}

	/* KEY_FOO from linux/input-event-codes.h */

	if( diff & to_type(ModifierKeys::GK_KEY_LEFT_CTRL) ) {
		e.event_code = KEY_LEFTCTRL;
		this->standard_keys_events_.push_back(e);

		diff -= to_type(ModifierKeys::GK_KEY_LEFT_CTRL);
		if( diff == 0 )
			return;
	}
	if( diff & to_type(ModifierKeys::GK_KEY_LEFT_SHIFT) ) {
		e.event_code = KEY_LEFTSHIFT;
		this->standard_keys_events_.push_back(e);

		diff -= to_type(ModifierKeys::GK_KEY_LEFT_SHIFT);
		if( diff == 0 )
			return;
	}
	if( diff & to_type(ModifierKeys::GK_KEY_LEFT_ALT) ) {
		e.event_code = KEY_LEFTALT;
		this->standard_keys_events_.push_back(e);

		diff -= to_type(ModifierKeys::GK_KEY_LEFT_ALT);
		if( diff == 0 )
			return;
	}
	if( diff & to_type(ModifierKeys::GK_KEY_LEFT_META) ) {
		e.event_code = KEY_LEFTMETA;
		this->standard_keys_events_.push_back(e);

		diff -= to_type(ModifierKeys::GK_KEY_LEFT_META);
		if( diff == 0 )
			return;
	}
	if( diff & to_type(ModifierKeys::GK_KEY_RIGHT_CTRL) ) {
		e.event_code = KEY_RIGHTCTRL;
		this->standard_keys_events_.push_back(e);

		diff -= to_type(ModifierKeys::GK_KEY_RIGHT_CTRL);
		if( diff == 0 )
			return;
	}
	if( diff & to_type(ModifierKeys::GK_KEY_RIGHT_SHIFT) ) {
		e.event_code = KEY_RIGHTSHIFT;
		this->standard_keys_events_.push_back(e);

		diff -= to_type(ModifierKeys::GK_KEY_RIGHT_SHIFT);
		if( diff == 0 )
			return;
	}
	if( diff & to_type(ModifierKeys::GK_KEY_RIGHT_ALT) ) {
		e.event_code = KEY_RIGHTALT;
		this->standard_keys_events_.push_back(e);

		diff -= to_type(ModifierKeys::GK_KEY_RIGHT_ALT);
		if( diff == 0 )
			return;
	}
	if( diff & to_type(ModifierKeys::GK_KEY_RIGHT_META) ) {
		e.event_code = KEY_RIGHTMETA;
		this->standard_keys_events_.push_back(e);

		diff -= to_type(ModifierKeys::GK_KEY_RIGHT_META);
		if( diff == 0 )
			return;
	}

	/*
	 * diff should have reached zero before here, and the function should have returned.
	 * If you see this warning, you should play to lottery. If this ever happens, this
	 * means that a modifier key was pressed in the exact same event in which another
	 * modifier was released, and in this case, the buffer checks at the top of the
	 * function could have been mixed-up. I don't know if the keyboard can produce such
	 * events. In theory, maybe. But never seen it. And I tried.
	 */
	this->logWarning("diff not equal to zero");
}

void KeyboardDriver::fillStandardKeysEvents(const InitializedDevice & device) {
	unsigned int i = 0;

#if DEBUGGING_ON
	LOG(DEBUG2) << "	b	|	p";
	LOG(DEBUG2) << "  ------------------------------";
#endif
	for(i = this->interrupt_key_read_length-1; i >= 2; --i) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "	" << to_uint(device.keys_buffer[i])
					<< "	|	" << to_uint(device.previous_keys_buffer[i]);
#endif
		if( device.previous_keys_buffer[i] == device.keys_buffer[i] ) {
			continue; /* nothing here */
		}
		else {
			KeyEvent e;

			if( device.previous_keys_buffer[i] == 0 ) {
				e.event_code = KeyboardDriver::hid_keyboard_[ device.keys_buffer[i] ];
				e.event = EventValue::EVENT_KEY_PRESS; /* KeyPress */
			}
			else if( device.keys_buffer[i] == 0 ) {
				e.event_code = KeyboardDriver::hid_keyboard_[ device.previous_keys_buffer[i] ];
				e.event = EventValue::EVENT_KEY_RELEASE; /* KeyRelease */
			}
			else {
				this->logWarning("two different byte values");
				continue;
			}

			this->standard_keys_events_.push_back(e);
		}
	}

	this->handleModifierKeys(device);
}

void KeyboardDriver::enterMacroRecordMode(InitializedDevice & device) {
	LOG(DEBUG) << "entering macro record mode";

	bool keys_found = false;

	while( ! keys_found and DaemonControl::is_daemon_enabled() and device.listen_status ) {
		uint64_t pressed_keys = 0;
		KeyStatus ret = this->getPressedKeys(device, &pressed_keys);

		switch( ret ) {
			case KeyStatus::S_KEY_PROCESSED:
				if( ! this->checkMacroKey(device, pressed_keys) ) {
					/* continue to store standard key events
					 * while a macro key is not pressed */
					// TODO limit ?
					continue;
				}

				/* macro key pressed, recording macro */
				device.macros_man->setMacro(device.chosen_macro_key, this->standard_keys_events_);
				keys_found = true;
				this->standard_keys_events_.clear();
				break;
			default:
				break;
		}
	}
}

void KeyboardDriver::listenLoop(const std::string devID) {
	InitializedDevice &device = this->initialized_devices_[devID];
	device.listen_thread_id = std::this_thread::get_id();

	LOG(INFO) << "spawned listening thread for " << device.device.name
				<< " on bus " << to_uint(device.bus);

	uint8_t & mask = device.current_leds_mask;

	LOG(DEBUG1) << "resetting M-Keys leds status";
	device.current_leds_mask = 0;
	this->setLeds(device);

	this->initializeMacroKeys(device);

	while( DaemonControl::is_daemon_enabled() and device.listen_status ) {
		uint64_t pressed_keys = 0;
		KeyStatus ret = this->getPressedKeys(device, &pressed_keys);
		switch( ret ) {
			case KeyStatus::S_KEY_PROCESSED:
				/* update M1-MR leds status only after proper event */
				if( device.last_transfer_length == this->leds_update_event_length_ ) {
					this->updateCurrentLedsMask(device, pressed_keys);
					this->setLeds(device);

					/* did we press the MR key ? */
					if( mask & to_type(Leds::GK_LED_MR) ) {
						this->enterMacroRecordMode(device);

						/* disabling macro record mode */
						mask &= ~(to_type(Leds::GK_LED_MR));
						this->setLeds(device);
					}
					else { /* check to run macro */
						if( this->checkMacroKey(device, pressed_keys) )
							device.macros_man->runMacro(device.chosen_macro_key);
					}
				}
				break;
			default:
				break;
		}
	}

	//device.macros_man->logProfiles();

	LOG(DEBUG1) << "resetting M-Keys leds status";
	mask = 0;
	this->setLeds(device);

	LOG(INFO) << "exiting listening thread for " << device.device.name
				<< " on bus " << to_uint(device.bus);
}

void KeyboardDriver::sendControlRequest(libusb_device_handle * usb_handle, uint16_t wValue, uint16_t wIndex,
		unsigned char * data, uint16_t wLength)
{
	int ret = libusb_control_transfer( usb_handle,
		LIBUSB_ENDPOINT_OUT|LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,
		LIBUSB_REQUEST_SET_CONFIGURATION, /* 0x09 */
		wValue, wIndex, data, wLength, 10000 );
	if( ret < 0 ) {
		this->buffer_.str("error sending control request");
		LOG(ERROR) << this->buffer_.str();
		syslog(LOG_ERR, this->buffer_.str().c_str());
		this->handleLibusbError(ret);
	}
	else {
		LOG(DEBUG2) << "sent " << ret << " bytes - expected: " << wLength;
	}
}

void KeyboardDriver::sendDeviceInitialization(const InitializedDevice & device) {
	this->logWarning("sendDeviceInitialization not implemented");
}

void KeyboardDriver::initializeDevice(const KeyboardDevice &dev, const uint8_t bus, const uint8_t num) {
	LOG(DEBUG3) << "trying to initialize " << dev.name << "("
				<< dev.vendor_id << ":" << dev.product_id << "), device "
				<< to_uint(num) << " on bus " << to_uint(bus);

	InitializedDevice device(dev, bus, num);

	this->initializeLibusb(device); /* device opened */

	try {
		this->setConfiguration(device);
		this->findExpectedUSBInterface(device);
		this->sendDeviceInitialization(device);
	}
	catch ( const GLogiKExcept & e ) {
		/* if we ever claimed or detached some interfaces, set them back
		 * to the same state in which we found them */
		this->releaseInterfaces( device.usb_handle );
		this->attachKernelDrivers( device.usb_handle );
		libusb_close( device.usb_handle );
		throw;
	}

	this->buffer_.str("b");
	this->buffer_ << to_uint(bus);
	this->buffer_ << "d";
	this->buffer_ << to_uint(num);

	const std::string devID = this->buffer_.str();

	/* virtual keyboard name */
	this->buffer_.str("Virtual ");
	this->buffer_ << device.device.name << " " << devID;

	try {
		device.macros_man = new MacrosManager(this->buffer_.str().c_str());
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		/* if we ever claimed or detached some interfaces, set them back
		 * to the same state in which we found them */
		this->releaseInterfaces( device.usb_handle );
		this->attachKernelDrivers( device.usb_handle );
		libusb_close( device.usb_handle );
		throw GLogiKExcept("virtual keyboard allocation failure");
	}

	device.listen_status = true;
	this->initialized_devices_[devID] = device;

	/* spawn listening thread */
	try {
		std::thread listen_thread(&KeyboardDriver::listenLoop, this, devID);
		this->threads_.push_back( std::move(listen_thread) );
	}
	catch (const std::system_error& e) {
		this->buffer_.str("error while spawning listening thread : ");
		this->buffer_ << e.what();
		throw GLogiKExcept(this->buffer_.str());
	}

}

void KeyboardDriver::detachKernelDriver(libusb_device_handle * usb_handle, int numInt) {
	int ret = libusb_kernel_driver_active(usb_handle, numInt);
	if( ret < 0 ) {
		this->handleLibusbError(ret);
		throw GLogiKExcept("libusb kernel_driver_active error");
	}
	if( ret ) {
		LOG(DEBUG1) << "kernel driver currently attached to the interface " << numInt << ", trying to detach it";
		ret = libusb_detach_kernel_driver(usb_handle, numInt); /* detaching */
		if( this->handleLibusbError(ret) ) {
			this->buffer_.str("detaching the kernel driver from USB interface ");
			this->buffer_ << numInt << " failed, this is fatal";
			LOG(ERROR) << this->buffer_.str();
			syslog(LOG_ERR, this->buffer_.str().c_str());
			throw GLogiKExcept(this->buffer_.str());
		}

		this->to_attach_.push_back(numInt);	/* detached */
	}
	else {
		LOG(DEBUG1) << "interface " << numInt << " is currently free :)";
	}
}

/*
 * This function is trying to (in this order) :
 *	- check the current active configuration value
 *	- if previous value matches the wanted one, simply return, else :
 *	- detach all kernel drivers from all interfaces
 *	- set the wanted configuration
 *	- re-attach all the previously attached drivers to interfaces
 *
 * In the comments on libusb/core.c, you can find :
 *		-#	libusb will be unable to set a configuration if other programs or
 *			drivers have claimed interfaces. In particular, this means that kernel
 *			drivers must be detached from all the interfaces before
 *			libusb_set_configuration() may succeed.
 *
 */
void KeyboardDriver::setConfiguration(const InitializedDevice & device) {
	unsigned int i, j, k = 0;
	int ret = 0;
	LOG(DEBUG1) << "setting up usb device configuration";

	int b = -1;
	ret = libusb_get_configuration(device.usb_handle, &b);
	if ( this->handleLibusbError(ret) )
		throw GLogiKExcept("libusb get_configuration error");

	LOG(DEBUG3) << "current active configuration value : " << b;
	if ( b == (int)(this->expected_usb_descriptors_.b_configuration_value) ) {
		LOG(INFO) << "current active configuration value matches the wanted value, skipping configuration";
		return;
	}

	LOG(DEBUG2) << "wanted configuration : " << (int)(this->expected_usb_descriptors_.b_configuration_value);
	LOG(DEBUG2) << "will try to set the active configuration to the wanted value";

	/* have to detach all interfaces first */

	struct libusb_device_descriptor device_descriptor;
	ret = libusb_get_device_descriptor(device.usb_device, &device_descriptor);
	if ( this->handleLibusbError(ret) )
		throw GLogiKExcept("libusb get_device_descriptor failure");

	for (i = 0; i < to_uint(device_descriptor.bNumConfigurations); i++) {
		/* configuration descriptor */
		struct libusb_config_descriptor * config_descriptor = nullptr;
		ret = libusb_get_config_descriptor(device.usb_device, i, &config_descriptor);
		if ( this->handleLibusbError(ret) ) {
			this->buffer_.str("get_config_descriptor failure with index : ");
			this->buffer_ << i;
			LOG(ERROR) << this->buffer_.str();
			syslog(LOG_ERR, this->buffer_.str().c_str());
			continue;
		}

		for (j = 0; j < to_uint(config_descriptor->bNumInterfaces); j++) {
			const struct libusb_interface *iface = &(config_descriptor->interface[j]);

			for (k = 0; k < to_uint(iface->num_altsetting); k++) {
				/* interface alt_setting descriptor */
				const struct libusb_interface_descriptor * as_descriptor = &(iface->altsetting[k]);

				int numInt = (int)as_descriptor->bInterfaceNumber;

				try {
					this->detachKernelDriver(device.usb_handle, numInt);
				}
				catch ( const GLogiKExcept & e ) {
					libusb_free_config_descriptor( config_descriptor ); /* free */
					throw;
				}
			} /* for ->num_altsetting */
		} /* for ->bNumInterfaces */

		libusb_free_config_descriptor( config_descriptor ); /* free */
	} /* for .bNumConfigurations */

	/* trying to set configuration */
	LOG(DEBUG2) << "checking current active configuration";
	ret = libusb_set_configuration(device.usb_handle, (int)this->expected_usb_descriptors_.b_configuration_value);
	if ( this->handleLibusbError(ret) ) {
		throw GLogiKExcept("libusb set_configuration failure");
	}

	this->attachKernelDrivers( device.usb_handle );
}

void KeyboardDriver::findExpectedUSBInterface(InitializedDevice & device) {
	unsigned int i, j, k, l = 0;
	int ret = 0;

	LOG(DEBUG1) << "trying to find expected interface";

	struct libusb_device_descriptor device_descriptor;
	ret = libusb_get_device_descriptor(device.usb_device, &device_descriptor);
	if ( this->handleLibusbError(ret) )
		throw GLogiKExcept("libusb get_device_descriptor failure");

#if DEBUGGING_ON
	LOG(DEBUG2) << "device has " << to_uint(device_descriptor.bNumConfigurations)
				<< " possible configuration(s)";

	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "device descriptor";
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "bLength            : " << to_uint(device_descriptor.bLength);
	LOG(DEBUG4) << "bDescriptorType    : " << to_uint(device_descriptor.bDescriptorType);
	LOG(DEBUG4) << "bcdUSB             : " << std::bitset<16>( to_uint(device_descriptor.bcdUSB) ).to_string();
	LOG(DEBUG4) << "bDeviceClass       : " << to_uint(device_descriptor.bDeviceClass);
	LOG(DEBUG4) << "bDeviceSubClass    : " << to_uint(device_descriptor.bDeviceSubClass);
	LOG(DEBUG4) << "bDeviceProtocol    : " << to_uint(device_descriptor.bDeviceProtocol);
	LOG(DEBUG4) << "bMaxPacketSize0    : " << to_uint(device_descriptor.bMaxPacketSize0);
	LOG(DEBUG4) << "idVendor           : " << std::hex << to_uint(device_descriptor.idVendor);
	LOG(DEBUG4) << "idProduct          : " << std::hex << to_uint(device_descriptor.idProduct);
	LOG(DEBUG4) << "bcdDevice          : " << std::bitset<16>( to_uint(device_descriptor.bcdDevice) ).to_string();
	LOG(DEBUG4) << "iManufacturer      : " << to_uint(device_descriptor.iManufacturer);
	LOG(DEBUG4) << "iProduct           : " << to_uint(device_descriptor.iProduct);
	LOG(DEBUG4) << "iSerialNumber      : " << to_uint(device_descriptor.iSerialNumber);
	LOG(DEBUG4) << "bNumConfigurations : " << to_uint(device_descriptor.bNumConfigurations);
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "--";
#endif

	for (i = 0; i < to_uint(device_descriptor.bNumConfigurations); i++) {
		struct libusb_config_descriptor * config_descriptor = nullptr;
		ret = libusb_get_config_descriptor(device.usb_device, i, &config_descriptor);
		if ( this->handleLibusbError(ret) ) {
			this->buffer_.str("get_config_descriptor failure with index : ");
			this->buffer_ << i;
			LOG(ERROR) << this->buffer_.str();
			syslog(LOG_ERR, this->buffer_.str().c_str());
			continue;
		}

#if DEBUGGING_ON
		LOG(DEBUG2) << "configuration " << to_uint(config_descriptor->bConfigurationValue)
					<< " has " << to_uint(config_descriptor->bNumInterfaces) << " interface(s)";

		LOG(DEBUG4) << "--";
		LOG(DEBUG4) << "config descriptor";
		LOG(DEBUG4) << "--";
		LOG(DEBUG4) << "bLength             : " << to_uint(config_descriptor->bLength);
		LOG(DEBUG4) << "bDescriptorType     : " << to_uint(config_descriptor->bDescriptorType);
		LOG(DEBUG4) << "wTotalLength        : " << to_uint(config_descriptor->wTotalLength);
		LOG(DEBUG4) << "bNumInterfaces      : " << to_uint(config_descriptor->bNumInterfaces);
		LOG(DEBUG4) << "bConfigurationValue : " << to_uint(config_descriptor->bConfigurationValue);
		LOG(DEBUG4) << "iConfiguration      : " << to_uint(config_descriptor->iConfiguration);
		LOG(DEBUG4) << "bmAttributes        : " << to_uint(config_descriptor->bmAttributes);
		LOG(DEBUG4) << "MaxPower            : " << to_uint(config_descriptor->MaxPower);
		/* TODO extra */
		LOG(DEBUG4) << "extra_length        : " << (int)config_descriptor->extra_length;
		LOG(DEBUG4) << "--";
		LOG(DEBUG4) << "--";
		LOG(DEBUG4) << "--";
#endif

		if ( config_descriptor->bConfigurationValue != this->expected_usb_descriptors_.b_configuration_value ) {
			libusb_free_config_descriptor( config_descriptor ); /* free */
			continue; /* skip non expected configuration */
		}

		for (j = 0; j < to_uint(config_descriptor->bNumInterfaces); j++) {
			const struct libusb_interface *iface = &(config_descriptor->interface[j]);
			LOG(DEBUG2) << "interface " << j << " has " << iface->num_altsetting << " alternate settings";

			for (k = 0; k < to_uint(iface->num_altsetting); k++) {
				const struct libusb_interface_descriptor * as_descriptor = &(iface->altsetting[k]);

#if DEBUGGING_ON
				LOG(DEBUG3) << "interface " << j << " alternate setting " << to_uint(as_descriptor->bAlternateSetting)
							<< " has " << to_uint(as_descriptor->bNumEndpoints) << " endpoints";

				LOG(DEBUG4) << "--";
				LOG(DEBUG4) << "interface descriptor";
				LOG(DEBUG4) << "--";
				LOG(DEBUG4) << "bLength            : " << to_uint(as_descriptor->bLength);
				LOG(DEBUG4) << "bDescriptorType    : " << to_uint(as_descriptor->bDescriptorType);
				LOG(DEBUG4) << "bInterfaceNumber   : " << to_uint(as_descriptor->bInterfaceNumber);
				LOG(DEBUG4) << "bAlternateSetting  : " << to_uint(as_descriptor->bAlternateSetting);
				LOG(DEBUG4) << "bNumEndpoints      : " << to_uint(as_descriptor->bNumEndpoints);
				LOG(DEBUG4) << "bInterfaceClass    : " << to_uint(as_descriptor->bInterfaceClass);
				LOG(DEBUG4) << "bInterfaceSubClass : " << to_uint(as_descriptor->bInterfaceSubClass);
				LOG(DEBUG4) << "bInterfaceProtocol : " << to_uint(as_descriptor->bInterfaceProtocol);
				LOG(DEBUG4) << "iInterface         : " << to_uint(as_descriptor->iInterface);
				/* TODO extra */
				LOG(DEBUG4) << "extra_length       : " << (int)as_descriptor->extra_length;
				LOG(DEBUG4) << "--";
				LOG(DEBUG4) << "--";
				LOG(DEBUG4) << "--";
#endif

				if ( as_descriptor->bInterfaceNumber != this->expected_usb_descriptors_.b_interface_number) {
					continue; /* skip non expected interface */
				}

				if ( as_descriptor->bAlternateSetting != this->expected_usb_descriptors_.b_alternate_setting) {
					continue; /* skip non expected alternate setting */
				}

				if( as_descriptor->bInterfaceClass != LIBUSB_CLASS_HID ) {
					LOG(WARNING) << "interface " << j << " alternate settings " << k << " is not for HID device, skipping it";
					continue; /* sanity check */
				}

				if ( as_descriptor->bNumEndpoints != this->expected_usb_descriptors_.b_num_endpoints) {
					LOG(WARNING) << "skipping settings. num_endpoints: " << to_uint(as_descriptor->bNumEndpoints)
							<< " expected: " << to_uint(this->expected_usb_descriptors_.b_num_endpoints);
					libusb_free_config_descriptor( config_descriptor ); /* free */
					throw GLogiKExcept("num_endpoints does not match");
				}

				/* specs found */
				LOG(DEBUG1) << "found the expected interface, keep going on this road";

				int numInt = (int)as_descriptor->bInterfaceNumber;

				try {
					this->detachKernelDriver(device.usb_handle, numInt);
				}
				catch ( const GLogiKExcept & e ) {
					libusb_free_config_descriptor( config_descriptor ); /* free */
					throw;
				}

				/* claiming interface */
				LOG(DEBUG1) << "trying to claim interface " << numInt;
				ret = libusb_claim_interface(device.usb_handle, numInt);	/* claiming */
				if( this->handleLibusbError(ret) ) {
					libusb_free_config_descriptor( config_descriptor ); /* free */
					throw GLogiKExcept("error claiming interface");
				}
				this->to_release_.push_back(numInt);	/* claimed */

				/* once that the interface is claimed, check that the right configuration is set */
				LOG(DEBUG3) << "checking current active configuration";
				int b = -1;
				ret = libusb_get_configuration(device.usb_handle, &b);
				if ( this->handleLibusbError(ret) ) {
					libusb_free_config_descriptor( config_descriptor ); /* free */
					throw GLogiKExcept("libusb get_configuration error");
				}

				LOG(DEBUG3) << "current active configuration value : " << b;
				if ( b != (int)(this->expected_usb_descriptors_.b_configuration_value) ) {
					libusb_free_config_descriptor( config_descriptor ); /* free */
					this->buffer_.str("error : wrong configuration value ");
					this->buffer_ << b << ", what a pity :(";
					LOG(ERROR) << this->buffer_.str();
					syslog(LOG_ERR, this->buffer_.str().c_str());
					throw GLogiKExcept("wrong configuration value");
				}

				for (l = 0; l < to_uint(as_descriptor->bNumEndpoints); l++) {
					const struct libusb_endpoint_descriptor * ep = &(as_descriptor->endpoint[l]);

					/* storing endpoint for later usage */
					device.endpoints.push_back(*ep);

					/* In: device-to-host */
					if( to_uint(ep->bEndpointAddress) & LIBUSB_ENDPOINT_IN ) {
						unsigned int addr = to_uint(ep->bEndpointAddress);
						LOG(DEBUG1) << "found [Keys] endpoint, address 0x" << std::hex << addr
									<< " MaxPacketSize " << to_uint(ep->wMaxPacketSize);
						device.keys_endpoint = addr & 0xff;
					}

#if DEBUGGING_ON
					LOG(DEBUG3) << "int. " << j << " alt_s. " << to_uint(as_descriptor->bAlternateSetting)
								<< " endpoint " << l;

					LOG(DEBUG4) << "--";
					LOG(DEBUG4) << "endpoint descriptor";
					LOG(DEBUG4) << "--";
					LOG(DEBUG4) << "bLength          : " << to_uint(ep->bLength);
					LOG(DEBUG4) << "bDescriptorType  : " << to_uint(ep->bDescriptorType);
					LOG(DEBUG4) << "bEndpointAddress : " << to_uint(ep->bEndpointAddress);
					LOG(DEBUG4) << "bmAttributes     : " << to_uint(ep->bmAttributes);
					LOG(DEBUG4) << "wMaxPacketSize   : " << to_uint(ep->wMaxPacketSize);
					LOG(DEBUG4) << "bInterval        : " << to_uint(ep->bInterval);
					LOG(DEBUG4) << "bRefresh         : " << to_uint(ep->bRefresh);
					LOG(DEBUG4) << "bSynchAddress    : " << to_uint(ep->bSynchAddress);
					/* TODO extra */
					LOG(DEBUG4) << "extra_length     : " << (int)ep->extra_length;
					LOG(DEBUG4) << "--";
					LOG(DEBUG4) << "--";
					LOG(DEBUG4) << "--";
#endif
				}

				if(device.keys_endpoint == 0) {
					libusb_free_config_descriptor( config_descriptor ); /* free */
					this->buffer_.str("error : [Keys] endpoint not found ! ");
					LOG(ERROR) << this->buffer_.str();
					syslog(LOG_ERR, this->buffer_.str().c_str());
					throw GLogiKExcept("[Keys] endpoint not found");
				}

				LOG(INFO) << "all done ! " << device.device.name << " interface " << numInt
							<< " opened and ready for I/O transfers";

			} /* for ->num_altsetting */
		} /* for ->bNumInterfaces */

		libusb_free_config_descriptor( config_descriptor ); /* free */
	} /* for .bNumConfigurations */

}

void KeyboardDriver::closeDevice(const KeyboardDevice &dev, const uint8_t bus, const uint8_t num) {
	LOG(DEBUG3) << "trying to close " << dev.name << "("
				<< dev.vendor_id << ":" << dev.product_id << "), device "
				<< to_uint(num) << " on bus " << to_uint(bus);

	this->buffer_.str("b");
	this->buffer_ << to_uint(bus);
	this->buffer_ << "d";
	this->buffer_ << to_uint(num);

	const std::string devID = this->buffer_.str();

	// FIXME at() exception ?
	InitializedDevice & device = this->initialized_devices_[devID];
	for(auto it2 = this->threads_.begin(); it2 != this->threads_.end();) {
		if( device.listen_thread_id == (*it2).get_id() ) {
			LOG(INFO) << "waiting for " << device.device.name << " listening thread";
			(*it2).join();
			it2 = this->threads_.erase(it2);
			break;
		}
		else {
			it2++;
		}
	}

	delete device.macros_man;
	device.macros_man = nullptr;

	this->releaseInterfaces( device.usb_handle );
	this->attachKernelDrivers( device.usb_handle );

	libusb_close( device.usb_handle );
	this->initialized_devices_.erase(devID);
}

} // namespace GLogiKd

