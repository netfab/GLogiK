/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
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

#include "lib/utils/utils.h"

#include "daemonControl.h"
#include "keyboardDriver.h"

#include "include/enums.h"

namespace GLogiK
{

bool KeyboardDriver::libusb_status_ = false;
libusb_context * KeyboardDriver::context_ = nullptr;

uint8_t KeyboardDriver::drivers_cnt_ = 0;

constexpr unsigned char KeyboardDriver::hid_keyboard_[256];

KeyboardDriver::KeyboardDriver(int key_read_length, uint8_t event_length, DescriptorValues values) :
		buffer_("", std::ios_base::app) {
	this->expected_usb_descriptors_ = values;
	this->leds_update_event_length_ = event_length;

	if( key_read_length > KEYS_BUFFER_LENGTH ) {
		this->logWarning("interrupt read length too large, set it to max buffer length");
		key_read_length = KEYS_BUFFER_LENGTH;
	}

	this->interrupt_key_read_length = key_read_length;
	KeyboardDriver::drivers_cnt_++;

	if( ! KeyboardDriver::libusb_status_ )
		this->initializeLibusb();
}

KeyboardDriver::~KeyboardDriver() {
	KeyboardDriver::drivers_cnt_--;
	if (KeyboardDriver::libusb_status_ and KeyboardDriver::drivers_cnt_ == 0)
		this->closeLibusb();
}

std::vector<KeyboardDevice> KeyboardDriver::getSupportedDevices(void) const {
	return this->supported_devices_;
}

const std::string KeyboardDriver::getDeviceID(const uint8_t bus, const uint8_t num) {
	std::string devID("b");
	devID += std::to_string(bus);
	devID += "d";
	devID += std::to_string(num);
	return devID;
}

void KeyboardDriver::openLibUSBDevice(InitializedDevice & device) {
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

	int ret_value = libusb_open( device.usb_device, &(device.usb_handle) );
	if( this->handleLibusbError(ret_value) ) {
		libusb_free_device_list(list, 1);
		throw GLogiKExcept("opening device failure");
	}

	libusb_free_device_list(list, 1);
}

void KeyboardDriver::initializeLibusb(void) {
	LOG(DEBUG4) << "initializing libusb";
	int ret_value = libusb_init( &(this->context_) );
	if ( this->handleLibusbError(ret_value) ) {
		throw GLogiKExcept("libusb initialization failure");
	}

	KeyboardDriver::libusb_status_ = true;
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

void KeyboardDriver::releaseInterfaces(InitializedDevice & device) {
	int ret = 0;
	for(auto it = device.to_release.begin(); it != device.to_release.end();) {
		int numInt = (*it);
		LOG(DEBUG1) << "trying to release claimed interface " << numInt;
		ret = libusb_release_interface(device.usb_handle, numInt); /* release */
		if( this->handleLibusbError(ret) ) {
			this->buffer_.str("failed to release interface ");
			this->buffer_ << numInt;
			LOG(ERROR) << this->buffer_.str();
			syslog(LOG_ERR, this->buffer_.str().c_str());
		}
		it++;
	}
	device.to_release.clear();
}

void KeyboardDriver::attachKernelDrivers(InitializedDevice & device) {
	int ret = 0;
	for(auto it = device.to_attach.begin(); it != device.to_attach.end();) {
		int numInt = (*it);
		LOG(DEBUG1) << "trying to attach kernel driver to interface " << numInt;
		ret = libusb_attach_kernel_driver(device.usb_handle, numInt); /* attaching */
		if( this->handleLibusbError(ret) ) {
			this->buffer_.str("failed to attach kernel driver to interface ");
			this->buffer_ << numInt;
			LOG(ERROR) << this->buffer_.str();
			syslog(LOG_ERR, this->buffer_.str().c_str());
		}
		it++;
	}
	device.to_attach.clear();
}

std::string KeyboardDriver::getBytes(const InitializedDevice & device) {
	if( device.transfer_length == 0 )
		return "";
	std::ostringstream s;
	s << std::hex << to_uint(device.keys_buffer[0]);
	for(unsigned int x = 1; x < to_uint(device.transfer_length); x++) {
		s << ", " << std::hex << to_uint(device.keys_buffer[x]);
	}
	return s.str();
}

void KeyboardDriver::initializeMacroKey(const InitializedDevice & device, const char* name) {
	device.macros_man->initializeMacroKey(name);
}

KeyStatus KeyboardDriver::getPressedKeys(InitializedDevice & device) {
	std::fill_n(device.keys_buffer, KEYS_BUFFER_LENGTH, 0);

	int ret = libusb_interrupt_transfer(device.usb_handle, device.keys_endpoint,
		(unsigned char*)device.keys_buffer, this->interrupt_key_read_length, &(device.transfer_length), 10);

	switch(ret) {
		case 0:
			if( device.transfer_length > 0 ) {
#if DEBUGGING_ON
				LOG(DEBUG)	<< "exp. rl: " << this->interrupt_key_read_length
							<< " act_l: " << device.transfer_length << ", xBuf[0]: "
							<< std::hex << to_uint(device.keys_buffer[0]);
#endif
				return this->processKeyEvent(device);
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

void KeyboardDriver::setMxKeysLeds(const InitializedDevice & device) {
	LOG(WARNING) << __func__ << " not implemented";
}

void KeyboardDriver::setKeyboardColor(const InitializedDevice & device) {
	LOG(WARNING) << __func__ << " not implemented";
}

void KeyboardDriver::sendDeviceInitialization(const InitializedDevice & device) {
	LOG(WARNING) << __func__ << " not implemented";
}

void KeyboardDriver::updateKeyboardColor(InitializedDevice & device, const uint8_t red,
	const uint8_t green, const uint8_t blue)
{
	device.rgb[0] = red;
	device.rgb[1] = green;
	device.rgb[2] = blue;
}

/*
 * return true if leds_mask has been updated (meaning that setMxKeysLeds should be called)
 */
const bool KeyboardDriver::updateCurrentLedsMask(InitializedDevice & device, bool force_MR_off) {
	uint8_t & mask = device.current_leds_mask;
	/* is macro record mode enabled ? */
	bool MR_ON = mask & to_type(Leds::GK_LED_MR);
	bool Mx_ON = false;
	bool mask_updated = false;

	if( device.pressed_keys & to_type(Keys::GK_KEY_M1) ) {
		Mx_ON = mask & to_type(Leds::GK_LED_M1);
		mask = 0;
		mask_updated = true;
		device.macros_man->setCurrentActiveProfile(MemoryBank::MACROS_M0);
		if( ! Mx_ON ) {
			mask |= to_type(Leds::GK_LED_M1);
			device.macros_man->setCurrentActiveProfile(MemoryBank::MACROS_M1);
		}
	}
	else if( device.pressed_keys & to_type(Keys::GK_KEY_M2) ) {
		Mx_ON = mask & to_type(Leds::GK_LED_M2);
		mask = 0;
		mask_updated = true;
		device.macros_man->setCurrentActiveProfile(MemoryBank::MACROS_M0);
		if( ! Mx_ON ) {
			mask |= to_type(Leds::GK_LED_M2);
			device.macros_man->setCurrentActiveProfile(MemoryBank::MACROS_M2);
		}
	}
	else if( device.pressed_keys & to_type(Keys::GK_KEY_M3) ) {
		Mx_ON = mask & to_type(Leds::GK_LED_M3);
		mask = 0;
		mask_updated = true;
		device.macros_man->setCurrentActiveProfile(MemoryBank::MACROS_M0);
		if( ! Mx_ON ) {
			mask |= to_type(Leds::GK_LED_M3);
			device.macros_man->setCurrentActiveProfile(MemoryBank::MACROS_M3);
		}
	}

	if( device.pressed_keys & to_type(Keys::GK_KEY_MR) ) {
		if(! MR_ON) { /* MR off, enable it */
			mask |= to_type(Leds::GK_LED_MR);
		}
		else { /* MR on, disable it */
			mask &= ~(to_type(Leds::GK_LED_MR));
		}
		mask_updated = true;
	}
	else if(force_MR_off) {
		mask &= ~(to_type(Leds::GK_LED_MR));
		mask_updated = true;
	}

	return mask_updated;
}

void KeyboardDriver::handleModifierKeys(InitializedDevice & device) {
	if( device.previous_keys_buffer[1] == device.keys_buffer[1] )
		return; /* nothing changed here */

	KeyEvent e;
	e.interval = this->getTimeLapse(device);
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
		device.standard_keys_events.push_back(e);

		diff -= to_type(ModifierKeys::GK_KEY_LEFT_CTRL);
		if( diff == 0 )
			return;
	}
	if( diff & to_type(ModifierKeys::GK_KEY_LEFT_SHIFT) ) {
		e.event_code = KEY_LEFTSHIFT;
		device.standard_keys_events.push_back(e);

		diff -= to_type(ModifierKeys::GK_KEY_LEFT_SHIFT);
		if( diff == 0 )
			return;
	}
	if( diff & to_type(ModifierKeys::GK_KEY_LEFT_ALT) ) {
		e.event_code = KEY_LEFTALT;
		device.standard_keys_events.push_back(e);

		diff -= to_type(ModifierKeys::GK_KEY_LEFT_ALT);
		if( diff == 0 )
			return;
	}
	if( diff & to_type(ModifierKeys::GK_KEY_LEFT_META) ) {
		e.event_code = KEY_LEFTMETA;
		device.standard_keys_events.push_back(e);

		diff -= to_type(ModifierKeys::GK_KEY_LEFT_META);
		if( diff == 0 )
			return;
	}
	if( diff & to_type(ModifierKeys::GK_KEY_RIGHT_CTRL) ) {
		e.event_code = KEY_RIGHTCTRL;
		device.standard_keys_events.push_back(e);

		diff -= to_type(ModifierKeys::GK_KEY_RIGHT_CTRL);
		if( diff == 0 )
			return;
	}
	if( diff & to_type(ModifierKeys::GK_KEY_RIGHT_SHIFT) ) {
		e.event_code = KEY_RIGHTSHIFT;
		device.standard_keys_events.push_back(e);

		diff -= to_type(ModifierKeys::GK_KEY_RIGHT_SHIFT);
		if( diff == 0 )
			return;
	}
	if( diff & to_type(ModifierKeys::GK_KEY_RIGHT_ALT) ) {
		e.event_code = KEY_RIGHTALT;
		device.standard_keys_events.push_back(e);

		diff -= to_type(ModifierKeys::GK_KEY_RIGHT_ALT);
		if( diff == 0 )
			return;
	}
	if( diff & to_type(ModifierKeys::GK_KEY_RIGHT_META) ) {
		e.event_code = KEY_RIGHTMETA;
		device.standard_keys_events.push_back(e);

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

uint16_t KeyboardDriver::getTimeLapse(InitializedDevice & device) {
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - device.last_call);
	device.last_call = std::chrono::steady_clock::now();
	return (ms.count() % 1000); /* max 1 second FIXME */
}

void KeyboardDriver::fillStandardKeysEvents(InitializedDevice & device) {
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
			e.interval = this->getTimeLapse(device);

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

			device.standard_keys_events.push_back(e);
		}
	}

	this->handleModifierKeys(device);
}

void KeyboardDriver::enterMacroRecordMode(InitializedDevice & device) {
	LOG(DEBUG) << "entering macro record mode";

	bool keys_found = false;
	/* initializing time_point */
	device.last_call = std::chrono::steady_clock::now();

	while( ! keys_found and DaemonControl::is_daemon_enabled() and device.listen_status ) {
		KeyStatus ret = this->getPressedKeys(device);

		switch( ret ) {
			case KeyStatus::S_KEY_PROCESSED:
				/* did we press one Mx key ? */
				if( device.pressed_keys & to_type(Keys::GK_KEY_M1) or
					device.pressed_keys & to_type(Keys::GK_KEY_M2) or
					device.pressed_keys & to_type(Keys::GK_KEY_M3) or
					device.pressed_keys & to_type(Keys::GK_KEY_MR) ) {
					/* exiting macro record mode */
					keys_found = true;
					device.standard_keys_events.clear();
					continue;
				}

				if( ! this->checkMacroKey(device) ) {
					/* continue to store standard key events
					 * while a macro key is not pressed */
					// TODO limit ?
					continue;
				}

				/* macro key pressed, recording macro */
				device.macros_man->setMacro(device.chosen_macro_key, device.standard_keys_events);
				keys_found = true;
				device.standard_keys_events.clear();
				break;
			default:
				break;
		}
	}

	LOG(DEBUG) << "exiting macro record mode";
}

void KeyboardDriver::runMacro(const std::string & devID) {
	InitializedDevice &device = this->initialized_devices_[devID];
	LOG(DEBUG2) << "spawned running macro thread for " << device.device.name;
	device.macros_man->runMacro(device.chosen_macro_key);
	LOG(DEBUG2) << "exiting running macro thread";
}

void KeyboardDriver::listenLoop(const std::string & devID) {
	InitializedDevice &device = this->initialized_devices_[devID];
	device.listen_thread_id = std::this_thread::get_id();

	LOG(INFO) << "spawned listening thread for " << device.device.name
				<< " on bus " << to_uint(device.bus);

	uint8_t & mask = device.current_leds_mask;

	LOG(DEBUG1) << "resetting MxKeys leds status";
	mask = 0;
	this->setMxKeysLeds(device);

	this->initializeMacroKeys(device);
	this->updateKeyboardColor(device, 0xFF, 0x0, 0x0);
	this->setKeyboardColor(device);

	while( DaemonControl::is_daemon_enabled() and device.listen_status ) {
		KeyStatus ret = this->getPressedKeys(device);
		switch( ret ) {
			case KeyStatus::S_KEY_PROCESSED:
				/*
				 * update M1-MR leds status, launch macro record mode and
				 * run macros only after proper event length
				 */
				if( device.transfer_length == this->leds_update_event_length_ ) {
					if(this->updateCurrentLedsMask(device))
						this->setMxKeysLeds(device);

					/* is macro record mode enabled ? */
					if( mask & to_type(Leds::GK_LED_MR) ) {
						this->enterMacroRecordMode(device);

						/* disabling macro record mode */
						if(this->updateCurrentLedsMask(device, true))
							this->setMxKeysLeds(device);
					}
					else { /* check to run macro */
						if( this->checkMacroKey(device) ) {
							try {
								/* spawn thread only if macro defined */
								if(device.macros_man->macroDefined(device.chosen_macro_key)) {
									std::thread macro_thread(&KeyboardDriver::runMacro, this, devID);
									macro_thread.detach();
								}
							}
							catch (const std::system_error& e) {
								this->logWarning("error while spawning running macro thread");
							}
						}
					}
				}
				break;
			default:
				break;
		}
	}

	//device.macros_man->logProfiles();

	LOG(DEBUG1) << "resetting MxKeys leds status";
	mask = 0;
	this->setMxKeysLeds(device);

	LOG(DEBUG1) << "resetting keyboard color";
	this->updateKeyboardColor(device);
	this->setKeyboardColor(device);

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

void KeyboardDriver::initializeDevice(const KeyboardDevice &dev, const uint8_t bus, const uint8_t num) {
	LOG(DEBUG3) << "trying to initialize " << dev.name << "("
				<< dev.vendor_id << ":" << dev.product_id << "), device "
				<< to_uint(num) << " on bus " << to_uint(bus);

	InitializedDevice device(dev, bus, num);

	this->openLibUSBDevice(device); /* device opened */

	try {
		this->setConfiguration(device);
		this->findExpectedUSBInterface(device);
		this->sendDeviceInitialization(device);

		const std::string devID = KeyboardDriver::getDeviceID(bus, num);

		/* virtual keyboard name */
		this->buffer_.str("Virtual ");
		this->buffer_ << device.device.name << " " << devID;

		try {
			device.macros_man = new MacrosManager(this->buffer_.str().c_str());
		}
		catch (const std::bad_alloc& e) { /* handle new() failure */
			throw GLogiKExcept("macros manager allocation failure");
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
	catch ( const GLogiKExcept & e ) {
		/* if we ever claimed or detached some interfaces, set them back
		 * to the same state in which we found them */
		this->releaseInterfaces(device);
		this->attachKernelDrivers(device);
		libusb_close( device.usb_handle );
		throw;
	}
}

const bool KeyboardDriver::isDeviceInitialized(const std::string & devID) const {
	return ( this->initialized_devices_.count(devID) == 1 );
}

void KeyboardDriver::detachKernelDriver(InitializedDevice & device, int numInt) {
	int ret = libusb_kernel_driver_active(device.usb_handle, numInt);
	if( ret < 0 ) {
		this->handleLibusbError(ret);
		throw GLogiKExcept("libusb kernel_driver_active error");
	}
	if( ret ) {
		LOG(DEBUG1) << "kernel driver currently attached to the interface " << numInt << ", trying to detach it";
		ret = libusb_detach_kernel_driver(device.usb_handle, numInt); /* detaching */
		if( this->handleLibusbError(ret) ) {
			this->buffer_.str("detaching the kernel driver from USB interface ");
			this->buffer_ << numInt << " failed, this is fatal";
			LOG(ERROR) << this->buffer_.str();
			syslog(LOG_ERR, this->buffer_.str().c_str());
			throw GLogiKExcept(this->buffer_.str());
		}

		device.to_attach.push_back(numInt);	/* detached */
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
void KeyboardDriver::setConfiguration(InitializedDevice & device) {
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
					this->detachKernelDriver(device, numInt);
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

	this->attachKernelDrivers(device);
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
					this->detachKernelDriver(device, numInt);
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
				device.to_release.push_back(numInt);	/* claimed */

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

	const std::string devID = KeyboardDriver::getDeviceID(bus, num);
	InitializedDevice & device = this->initialized_devices_[devID];
	device.listen_status = false;

	bool found = false;
	for(auto it = this->threads_.begin(); it != this->threads_.end();) {
		if( device.listen_thread_id == (*it).get_id() ) {
			found = true;
			LOG(INFO) << "waiting for " << device.device.name << " listening thread";
			(*it).join();
			it = this->threads_.erase(it);
			break;
		}
		else {
			it++;
		}
	}

	if(! found)
		this->logWarning("listening thread not found !");

	delete device.macros_man;
	device.macros_man = nullptr;

	this->releaseInterfaces(device);
	this->attachKernelDrivers(device);

	libusb_close( device.usb_handle );
	this->initialized_devices_.erase(devID);
}

} // namespace GLogiK
