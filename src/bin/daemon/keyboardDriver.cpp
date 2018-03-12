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

#include <stdexcept>
#include <new>
#include <iostream>
#include <bitset>

#include <utility>

#include "lib/utils/utils.h"
#include "lib/dbus/GKDBus.h"
#include "lib/shared/glogik.h"

#include "daemonControl.h"
#include "keyboardDriver.h"

#include "include/enums.h"

namespace GLogiK
{

using namespace NSGKUtils;

bool KeyboardDriver::libusb_status_ = false;
libusb_context * KeyboardDriver::context_ = nullptr;
uint8_t KeyboardDriver::drivers_cnt_ = 0;
constexpr unsigned char KeyboardDriver::hid_keyboard_[256];
std::vector<std::string> KeyboardDriver::keys_names_ = {};

/* KEY_FOO from linux/input-event-codes.h */
const std::vector< ModifierKey > KeyboardDriver::modifier_keys_ = {
	{   KEY_LEFTCTRL,	GKModifierKeys::GK_KEY_LEFT_CTRL	},
	{  KEY_LEFTSHIFT,	GKModifierKeys::GK_KEY_LEFT_SHIFT	},
	{    KEY_LEFTALT,	GKModifierKeys::GK_KEY_LEFT_ALT		},
	{   KEY_LEFTMETA,	GKModifierKeys::GK_KEY_LEFT_META	},
	{  KEY_RIGHTCTRL,	GKModifierKeys::GK_KEY_RIGHT_CTRL	},
	{ KEY_RIGHTSHIFT,	GKModifierKeys::GK_KEY_RIGHT_SHIFT	},
	{   KEY_RIGHTALT,	GKModifierKeys::GK_KEY_RIGHT_ALT	},
	{  KEY_RIGHTMETA,	GKModifierKeys::GK_KEY_RIGHT_META	}
};

/* -- -- -- */

KeyboardDriver::KeyboardDriver(int key_read_length, uint8_t event_length, DescriptorValues values) :
		buffer_("", std::ios_base::app) {
	this->expected_usb_descriptors_ = values;
	this->leds_update_event_length_ = event_length;

	if( key_read_length > KEYS_BUFFER_LENGTH ) {
		GKSysLog(LOG_WARNING, WARNING, "interrupt read length too large, set it to max buffer length");
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

const std::string KeyboardDriver::getDeviceID(const uint8_t bus, const uint8_t num) {
	std::string devID("b");
	devID += std::to_string(bus);
	devID += "d";
	devID += std::to_string(num);
	return devID;
}

const std::vector<std::string> & KeyboardDriver::getEmptyStringVector(void) {
	KeyboardDriver::keys_names_.clear();
	return KeyboardDriver::keys_names_;
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

#if DEBUGGING_ON
	LOG(DEBUG3) << "libusb found device " << to_uint(device.num)
				<< " on bus " << to_uint(device.bus);
#endif

	int ret_value = libusb_open( device.usb_device, &(device.usb_handle) );
	if( this->handleLibusbError(ret_value) ) {
		libusb_free_device_list(list, 1);
		throw GLogiKExcept("opening device failure");
	}

	libusb_free_device_list(list, 1);
}

void KeyboardDriver::initializeLibusb(void) {
#if DEBUGGING_ON
	LOG(DEBUG3) << "initializing libusb";
#endif
	int ret_value = libusb_init( &(this->context_) );
	if ( this->handleLibusbError(ret_value) ) {
		throw GLogiKExcept("libusb initialization failure");
	}

	KeyboardDriver::libusb_status_ = true;
}

void KeyboardDriver::closeLibusb(void) {
#if DEBUGGING_ON
	LOG(DEBUG3) << "closing libusb";
#endif
	if( this->initialized_devices_.size() != 0 ) { /* sanity check */
		GKSysLog(LOG_WARNING, WARNING, "closing libusb with opened device(s) !");
	}
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
			GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
			break;
	}

	return error_code;
}

void KeyboardDriver::releaseInterfaces(InitializedDevice & device) {
	int ret = 0;
	for(auto it = device.to_release.begin(); it != device.to_release.end();) {
		int numInt = (*it);
#if DEBUGGING_ON
		LOG(DEBUG1) << "trying to release claimed interface " << numInt;
#endif
		ret = libusb_release_interface(device.usb_handle, numInt); /* release */
		if( this->handleLibusbError(ret) ) {
			this->buffer_.str("failed to release interface ");
			this->buffer_ << numInt;
			GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
		}
		it++;
	}
	device.to_release.clear();
}

void KeyboardDriver::attachKernelDrivers(InitializedDevice & device) {
	int ret = 0;
	for(auto it = device.to_attach.begin(); it != device.to_attach.end();) {
		int numInt = (*it);
#if DEBUGGING_ON
		LOG(DEBUG1) << "trying to attach kernel driver to interface " << numInt;
#endif
		ret = libusb_attach_kernel_driver(device.usb_handle, numInt); /* attaching */
		if( this->handleLibusbError(ret) ) {
			this->buffer_.str("failed to attach kernel driver to interface ");
			this->buffer_ << numInt;
			GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
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
#if 0 && DEBUGGING_ON
			LOG(DEBUG5) << "timeout reached";
#endif
			return KeyStatus::S_KEY_TIMEDOUT;
			break;
		default:
			GKSysLog(LOG_ERR, ERROR, "getPressedKeys interrupt read error");
			this->handleLibusbError(ret);
			if(ret == LIBUSB_ERROR_NO_DEVICE)
				device.fatal_errors++;
			return KeyStatus::S_KEY_SKIPPED;
			break;
	}

	return KeyStatus::S_KEY_TIMEDOUT;
}

void KeyboardDriver::notImplemented(const char* func) {
	this->buffer_.str("not implemented : ");
	this->buffer_ << func;
	GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
}

void KeyboardDriver::setMxKeysLeds(const InitializedDevice & device) {
	this->notImplemented(__func__);
}

void KeyboardDriver::setKeyboardColor(const InitializedDevice & device) {
	this->notImplemented(__func__);
}

void KeyboardDriver::sendDeviceInitialization(const InitializedDevice & device) {
	this->notImplemented(__func__);
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
	auto & mask = device.current_leds_mask;
	/* is macro record mode enabled ? */
	bool MR_ON = mask & to_type(Leds::GK_LED_MR);
	bool Mx_ON = false;
	bool mask_updated = false;

	if( device.pressed_keys & to_type(Keys::GK_KEY_M1) ) {
		Mx_ON = mask & to_type(Leds::GK_LED_M1);
		mask = 0;
		mask_updated = true;
		device.macros_man->setCurrentActiveProfile(MemoryBank::BANK_M0);
		if( ! Mx_ON ) {
			mask |= to_type(Leds::GK_LED_M1);
			device.macros_man->setCurrentActiveProfile(MemoryBank::BANK_M1);
		}
	}
	else if( device.pressed_keys & to_type(Keys::GK_KEY_M2) ) {
		Mx_ON = mask & to_type(Leds::GK_LED_M2);
		mask = 0;
		mask_updated = true;
		device.macros_man->setCurrentActiveProfile(MemoryBank::BANK_M0);
		if( ! Mx_ON ) {
			mask |= to_type(Leds::GK_LED_M2);
			device.macros_man->setCurrentActiveProfile(MemoryBank::BANK_M2);
		}
	}
	else if( device.pressed_keys & to_type(Keys::GK_KEY_M3) ) {
		Mx_ON = mask & to_type(Leds::GK_LED_M3);
		mask = 0;
		mask_updated = true;
		device.macros_man->setCurrentActiveProfile(MemoryBank::BANK_M0);
		if( ! Mx_ON ) {
			mask |= to_type(Leds::GK_LED_M3);
			device.macros_man->setCurrentActiveProfile(MemoryBank::BANK_M3);
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

const uint8_t KeyboardDriver::handleModifierKeys(InitializedDevice & device, const uint16_t interval) {
	if( device.previous_keys_buffer[1] == device.keys_buffer[1] )
		return 0; /* nothing changed here */

	KeyEvent e;
	e.interval = interval;
	uint8_t diff = 0;
	uint8_t ret = 0;

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

	for(const auto & key : KeyboardDriver::modifier_keys_) {
		const uint8_t v_mod_key = to_type(key.mod_key);
		if( diff & v_mod_key ) { /* modifier key was pressed or released */
			diff -= v_mod_key;

			bool skip_event = false;
			const uint8_t size = device.standard_keys_events.size();

			/* Macro Size Limit - see keyEvent.h */
			if(size >= MACRO_T_MAX_SIZE) {
				/* skip all new events */
#if DEBUGGING_ON
				LOG(WARNING) << "skipping modifier key event, reached macro max size";
#endif
				skip_event = true;
			}

			/* Macro Size Limit - see keyEvent.h */
			if( ! skip_event and (size >= MACRO_T_KEYPRESS_MAX_SIZE) ) {
				/* skip events other than release */
				if( e.event != EventValue::EVENT_KEY_RELEASE ) {
#if DEBUGGING_ON
					LOG(WARNING) << "skipping modifier keypress event, reached macro keypress max size";
#endif
					skip_event = true;
				}
			}

			if( ! skip_event ) {
				/*
				 * some modifier keys events were already appended,
				 * only first event should have a real timelapse interval
				 */
				if(ret > 0)
					e.interval = 1;
				e.event_code = key.event_code;
				device.standard_keys_events.push_back(e);
				ret++;
			}
		}

		/* make sure we test this at each loop run */
		if( diff == 0 )
			return ret;
	}

	/*
	 * diff should have reached zero before here, and the function should have returned.
	 * If you see this warning, you should play to lottery. If this ever happens, this
	 * means that a modifier key was pressed in the exact same event in which another
	 * modifier was released, and in this case, the buffer checks at the top of the
	 * function could have been mixed-up. I don't know if the keyboard can produce such
	 * events. In theory, maybe. But never seen it. And I tried.
	 */
	GKSysLog(LOG_WARNING, WARNING, "diff not equal to zero");
	return ret;
}

/* used to create macros */
uint16_t KeyboardDriver::getTimeLapse(InitializedDevice & device) {
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - device.last_call);
	device.last_call = std::chrono::steady_clock::now();
	return (ms.count() % 1000); /* max 1 second FIXME */
}

void KeyboardDriver::fillStandardKeysEvents(InitializedDevice & device) {
	unsigned int i = 0;

	uint16_t interval = this->getTimeLapse(device);

	const uint8_t num = this->handleModifierKeys(device, interval);

	if( num > 0 ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << to_uint(num) << " modifier keys event(s) added";
#endif
		/* only first event should have a real timelapse interval */
		interval = 1;
	}

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
			/* Macro Size Limit - see keyEvent.h */
			if(device.standard_keys_events.size() >= MACRO_T_MAX_SIZE ) {
				/* skip all new events */
#if DEBUGGING_ON
				LOG(WARNING) << "skipping key event, reached macro max size";
#endif
				continue;
			}

			KeyEvent e;
			e.interval = interval;

			if( device.previous_keys_buffer[i] == 0 ) {
				e.event_code = KeyboardDriver::hid_keyboard_[ device.keys_buffer[i] ];
				e.event = EventValue::EVENT_KEY_PRESS; /* KeyPress */
			}
			else if( device.keys_buffer[i] == 0 ) {
				e.event_code = KeyboardDriver::hid_keyboard_[ device.previous_keys_buffer[i] ];
				e.event = EventValue::EVENT_KEY_RELEASE; /* KeyRelease */
			}
			else {
				GKSysLog(LOG_WARNING, WARNING, "two different byte values");
				continue;
			}

			/* Macro Size Limit - see keyEvent.h */
			if(device.standard_keys_events.size() >= MACRO_T_KEYPRESS_MAX_SIZE ) {
				/* skip events other than release */
				if( e.event != EventValue::EVENT_KEY_RELEASE ) {
#if DEBUGGING_ON
					LOG(WARNING) << "skipping keypress event, reached macro keypress max size";
#endif
					continue;
				}
			}

			device.standard_keys_events.push_back(e);
			/* only first event should have a real timelapse interval */
			interval = 1;
		}
	}
}

void KeyboardDriver::checkDeviceListeningStatus(InitializedDevice & device) {
	/* check to give up */
	if(device.fatal_errors > DEVICE_LISTENING_THREAD_MAX_ERRORS) {
		std::ostringstream err("device ", std::ios_base::app);
		err << device.device.name << " on bus " << to_uint(device.bus);
		GKSysLog(LOG_ERR, ERROR, err.str());
		GKSysLog(LOG_ERR, ERROR, "reached listening thread maximum fatal errors, giving up");
		device.listen_status = false;
	}
}

void KeyboardDriver::enterMacroRecordMode(InitializedDevice & device, const std::string & devID) {
#if DEBUGGING_ON
	LOG(DEBUG) << "entering macro record mode";
#endif

	NSGKDBus::GKDBus* pDBus = nullptr;

	/* ROOT_NODE only for introspection, don't care */
	try {
		pDBus = new NSGKDBus::GKDBus(GLOGIK_DAEMON_DBUS_ROOT_NODE);
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		LOG(ERROR) << "GKDBus bad allocation";
		pDBus = nullptr;
	}

	auto & exit = device.exit_macro_record_mode;
	exit = false;
	/* initializing time_point */
	device.last_call = std::chrono::steady_clock::now();

	while( (! exit) and DaemonControl::isDaemonRunning() and device.listen_status ) {
		this->checkDeviceListeningStatus(device);
		if( ! device.listen_status )
			continue;

		KeyStatus ret = this->getPressedKeys(device);

		switch( ret ) {
			case KeyStatus::S_KEY_PROCESSED:
				/* did we press one Mx key ? */
				if( device.pressed_keys & to_type(Keys::GK_KEY_M1) or
					device.pressed_keys & to_type(Keys::GK_KEY_M2) or
					device.pressed_keys & to_type(Keys::GK_KEY_M3) or
					device.pressed_keys & to_type(Keys::GK_KEY_MR) ) {
					/* exiting macro record mode */
					exit = true;
					device.standard_keys_events.clear();
					continue;
				}

				if( ! this->checkMacroKey(device) ) {
					/* continue to store standard key events
					 * while a macro key is not pressed */
					continue;
				}

				try {
					/* macro key pressed, recording macro */
					device.macros_man->setMacro(
						device.chosen_macro_key,
						device.standard_keys_events
					);

					if( pDBus ) {
						const uint8_t profile = to_type( device.macros_man->getCurrentActiveProfile() );

						/* open a new connection, GKDBus is not thread-safe */
						pDBus->connectToSystemBus(GLOGIK_DEVICE_THREAD_DBUS_BUS_CONNECTION_NAME);

						try {
							std::string signal("MacroRecorded");
							if( device.standard_keys_events.empty() ) {
								signal = "MacroCleared";
							}

							pDBus->initializeTargetsSignal(
								NSGKDBus::BusConnection::GKDBUS_SYSTEM,
								GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME,
								GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT_PATH,
								GLOGIK_DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE,
								signal.c_str()
							);

							pDBus->appendStringToTargetsSignal(devID);
							pDBus->appendStringToTargetsSignal(device.chosen_macro_key);
							pDBus->appendUInt8ToTargetsSignal(profile);

							pDBus->sendTargetsSignal();
						}
						catch (const GKDBusMessageWrongBuild & e) {
							pDBus->abandonTargetsSignal();
							GKSysLog(LOG_WARNING, WARNING, e.what());
						}
					}
				}
				catch (const GLogiKExcept & e) {
					GKSysLog(LOG_WARNING, WARNING, e.what());
				}

				exit = true;
				device.standard_keys_events.clear();
				break;
			default:
				break;
		}
	}

	delete pDBus;
	pDBus = nullptr;

#if DEBUGGING_ON
	LOG(DEBUG) << "exiting macro record mode";
#endif
}

void KeyboardDriver::runMacro(const std::string & devID) {
	InitializedDevice &device = this->initialized_devices_[devID];
#if DEBUGGING_ON
	LOG(DEBUG2) << "spawned running macro thread for " << device.device.name;
#endif
	device.macros_man->runMacro(device.chosen_macro_key);
#if DEBUGGING_ON
	LOG(DEBUG2) << "exiting running macro thread";
#endif
}

void KeyboardDriver::listenLoop(const std::string & devID) {
	InitializedDevice &device = this->initialized_devices_[devID];
	device.listen_thread_id = std::this_thread::get_id();

#if DEBUGGING_ON
	LOG(INFO) << "spawned listening thread for " << device.device.name
				<< " on bus " << to_uint(device.bus);
#endif

	const auto & mask = device.current_leds_mask;

	while( DaemonControl::isDaemonRunning() and device.listen_status ) {
		this->checkDeviceListeningStatus(device);
		if( ! device.listen_status )
			continue;

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
						this->enterMacroRecordMode(device, devID);

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
								GKSysLog(LOG_WARNING, WARNING, "error while spawning running macro thread");
							}
						}
					}
				}
				break;
			default:
				break;
		}
	}

#if DEBUGGING_ON
	LOG(INFO) << "exiting listening thread for " << device.device.name
				<< " on bus " << to_uint(device.bus);
#endif
}

void KeyboardDriver::sendControlRequest(libusb_device_handle * usb_handle, uint16_t wValue, uint16_t wIndex,
		unsigned char * data, uint16_t wLength)
{
	int ret = libusb_control_transfer( usb_handle,
		LIBUSB_ENDPOINT_OUT|LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,
		LIBUSB_REQUEST_SET_CONFIGURATION, /* 0x09 */
		wValue, wIndex, data, wLength, 10000 );
	if( ret < 0 ) {
		GKSysLog(LOG_ERR, ERROR, "error sending control request");
		this->handleLibusbError(ret);
	}
	else {
#if DEBUGGING_ON
		LOG(DEBUG2) << "sent " << ret << " bytes - expected: " << wLength;
#endif
	}
}

void KeyboardDriver::initializeDevice(const KeyboardDevice &dev, const uint8_t bus, const uint8_t num) {
#if DEBUGGING_ON
	LOG(DEBUG3) << "trying to initialize " << dev.name << "("
				<< dev.vendor_id << ":" << dev.product_id << "), device "
				<< to_uint(num) << " on bus " << to_uint(bus);
#endif

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
			device.macros_man = new MacrosManager(
				this->buffer_.str().c_str(),
				this->getMacroKeysNames()
			);
		}
		catch (const std::bad_alloc& e) { /* handle new() failure */
			throw GLogiKBadAlloc("macros manager allocation failure");
		}

#if DEBUGGING_ON
		LOG(DEBUG1) << "resetting MxKeys leds status";
#endif
		device.current_leds_mask = 0;
		this->setMxKeysLeds(device);

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
#if DEBUGGING_ON
		LOG(DEBUG1) << "kernel driver currently attached to the interface " << numInt << ", trying to detach it";
#endif
		ret = libusb_detach_kernel_driver(device.usb_handle, numInt); /* detaching */
		if( this->handleLibusbError(ret) ) {
			this->buffer_.str("detaching the kernel driver from USB interface ");
			this->buffer_ << numInt << " failed, this is fatal";
			GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
			throw GLogiKExcept(this->buffer_.str());
		}

		device.to_attach.push_back(numInt);	/* detached */
	}
	else {
#if DEBUGGING_ON
		LOG(DEBUG1) << "interface " << numInt << " is currently free :)";
#endif
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
#if DEBUGGING_ON
	LOG(DEBUG1) << "setting up usb device configuration";
#endif

	int b = -1;
	ret = libusb_get_configuration(device.usb_handle, &b);
	if ( this->handleLibusbError(ret) )
		throw GLogiKExcept("libusb get_configuration error");

#if DEBUGGING_ON
	LOG(DEBUG3) << "current active configuration value : " << b;
#endif

	if ( b == (int)(this->expected_usb_descriptors_.b_configuration_value) ) {
#if DEBUGGING_ON
		LOG(INFO) << "current active configuration value matches the wanted value, skipping configuration";
#endif
		return;
	}

#if DEBUGGING_ON
	LOG(DEBUG2) << "wanted configuration : " << (int)(this->expected_usb_descriptors_.b_configuration_value);
	LOG(DEBUG2) << "will try to set the active configuration to the wanted value";
#endif

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
			GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
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
#if DEBUGGING_ON
	LOG(DEBUG2) << "checking current active configuration";
#endif
	ret = libusb_set_configuration(device.usb_handle, (int)this->expected_usb_descriptors_.b_configuration_value);
	if ( this->handleLibusbError(ret) ) {
		throw GLogiKExcept("libusb set_configuration failure");
	}

	this->attachKernelDrivers(device);
}

void KeyboardDriver::findExpectedUSBInterface(InitializedDevice & device) {
	unsigned int i, j, k, l = 0;
	int ret = 0;

#if DEBUGGING_ON
	LOG(DEBUG1) << "trying to find expected interface";
#endif

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
			GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
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
#if DEBUGGING_ON
			LOG(DEBUG2) << "interface " << j << " has " << iface->num_altsetting << " alternate settings";
#endif

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
#if DEBUGGING_ON
					LOG(WARNING) << "interface " << j << " alternate settings " << k << " is not for HID device, skipping it";
#endif
					continue; /* sanity check */
				}

				if ( as_descriptor->bNumEndpoints != this->expected_usb_descriptors_.b_num_endpoints) {
#if DEBUGGING_ON
					LOG(WARNING) << "skipping settings. num_endpoints: " << to_uint(as_descriptor->bNumEndpoints)
							<< " expected: " << to_uint(this->expected_usb_descriptors_.b_num_endpoints);
#endif
					libusb_free_config_descriptor( config_descriptor ); /* free */
					throw GLogiKExcept("num_endpoints does not match");
				}

				/* specs found */
#if DEBUGGING_ON
				LOG(DEBUG1) << "found the expected interface, keep going on this road";
#endif

				int numInt = (int)as_descriptor->bInterfaceNumber;

				try {
					this->detachKernelDriver(device, numInt);
				}
				catch ( const GLogiKExcept & e ) {
					libusb_free_config_descriptor( config_descriptor ); /* free */
					throw;
				}

				/* claiming interface */
#if DEBUGGING_ON
				LOG(DEBUG1) << "trying to claim interface " << numInt;
#endif
				ret = libusb_claim_interface(device.usb_handle, numInt);	/* claiming */
				if( this->handleLibusbError(ret) ) {
					libusb_free_config_descriptor( config_descriptor ); /* free */
					throw GLogiKExcept("error claiming interface");
				}
				device.to_release.push_back(numInt);	/* claimed */

				/* once that the interface is claimed, check that the right configuration is set */
#if DEBUGGING_ON
				LOG(DEBUG3) << "checking current active configuration";
#endif
				int b = -1;
				ret = libusb_get_configuration(device.usb_handle, &b);
				if ( this->handleLibusbError(ret) ) {
					libusb_free_config_descriptor( config_descriptor ); /* free */
					throw GLogiKExcept("libusb get_configuration error");
				}

#if DEBUGGING_ON
				LOG(DEBUG3) << "current active configuration value : " << b;
#endif
				if ( b != (int)(this->expected_usb_descriptors_.b_configuration_value) ) {
					libusb_free_config_descriptor( config_descriptor ); /* free */
					this->buffer_.str("wrong configuration value ");
					this->buffer_ << b << ", what a pity :(";
					GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
					throw GLogiKExcept("wrong configuration value");
				}

				for (l = 0; l < to_uint(as_descriptor->bNumEndpoints); l++) {
					const struct libusb_endpoint_descriptor * ep = &(as_descriptor->endpoint[l]);

					/* storing endpoint for later usage */
					device.endpoints.push_back(*ep);

					/* In: device-to-host */
					if( to_uint(ep->bEndpointAddress) & LIBUSB_ENDPOINT_IN ) {
						unsigned int addr = to_uint(ep->bEndpointAddress);
#if DEBUGGING_ON
						LOG(DEBUG1) << "found [Keys] endpoint, address 0x" << std::hex << addr
									<< " MaxPacketSize " << to_uint(ep->wMaxPacketSize);
#endif
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
					const std::string err("[Keys] endpoint not found");
					GKSysLog(LOG_ERR, ERROR, err);
					throw GLogiKExcept(err);
				}

#if DEBUGGING_ON
				LOG(INFO) << "all done ! " << device.device.name << " interface " << numInt
							<< " opened and ready for I/O transfers";
#endif

			} /* for ->num_altsetting */
		} /* for ->bNumInterfaces */

		libusb_free_config_descriptor( config_descriptor ); /* free */
	} /* for .bNumConfigurations */

}

void KeyboardDriver::resetDeviceState(const std::string & devID) {
	try {
		InitializedDevice & device = this->initialized_devices_.at(devID);

		/* exit MacroRecordMode if necessary */
		device.exit_macro_record_mode = true;
		device.standard_keys_events.clear();

		device.macros_man->clearMacroProfiles();

#if DEBUGGING_ON
		LOG(DEBUG1) << "resetting MxKeys leds status";
#endif
		device.current_leds_mask = 0;
		this->setMxKeysLeds(device);

#if DEBUGGING_ON
		LOG(DEBUG1) << "resetting keyboard color";
#endif
		this->updateKeyboardColor(device);
		this->setKeyboardColor(device);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
}

void KeyboardDriver::resetDeviceState(const KeyboardDevice &dev, const uint8_t bus, const uint8_t num) {
#if DEBUGGING_ON
	LOG(DEBUG3) << "resetting state of " << dev.name << "("
				<< dev.vendor_id << ":" << dev.product_id << "), device "
				<< to_uint(num) << " on bus " << to_uint(bus);
#endif

	const std::string devID = KeyboardDriver::getDeviceID(bus, num);
	this->resetDeviceState(devID);
}

void KeyboardDriver::closeDevice(const KeyboardDevice &dev, const uint8_t bus, const uint8_t num) {
#if DEBUGGING_ON
	LOG(DEBUG3) << "trying to close " << dev.name << "("
				<< dev.vendor_id << ":" << dev.product_id << "), device "
				<< to_uint(num) << " on bus " << to_uint(bus);
#endif

	const std::string devID = KeyboardDriver::getDeviceID(bus, num);

	try {
		InitializedDevice & device = this->initialized_devices_[devID];
		device.listen_status = false;

		bool found = false;
		for(auto it = this->threads_.begin(); it != this->threads_.end();) {
			if( device.listen_thread_id == (*it).get_id() ) {
				found = true;
#if DEBUGGING_ON
				LOG(INFO) << "waiting for " << device.device.name << " listening thread";
#endif
				(*it).join();
				it = this->threads_.erase(it);
				break;
			}
			else {
				it++;
			}
		}

		if(! found) {
			GKSysLog(LOG_WARNING, WARNING, "listening thread not found !");
		}

		this->resetDeviceState(devID);

		delete device.macros_man;
		device.macros_man = nullptr;

		this->releaseInterfaces(device);
		this->attachKernelDrivers(device);

		libusb_close( device.usb_handle );
		this->initialized_devices_.erase(devID);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
}

/* only called when setting active user's parameters */
void KeyboardDriver::setDeviceBacklightColor(const std::string & devID, const uint8_t r, const uint8_t g, const uint8_t b) {
	try {
		InitializedDevice & device = this->initialized_devices_.at(devID);
		this->updateKeyboardColor(device, r, g, b);
		this->setKeyboardColor(device);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
}

/* only called when setting active user's parameters */
void KeyboardDriver::setDeviceMacrosProfiles(
	const std::string & devID,
	const macros_map_t & macros_profiles
) {
	try {
		InitializedDevice & device = this->initialized_devices_.at(devID);

		/* exit MacroRecordMode if necessary */
		device.exit_macro_record_mode = true;
		device.standard_keys_events.clear();

		device.macros_man->setMacrosProfiles(macros_profiles);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
}

const macros_map_t & KeyboardDriver::getDeviceMacrosProfiles(const std::string & devID) {
	try {
		InitializedDevice & device = this->initialized_devices_.at(devID);
		return device.macros_man->getMacrosProfiles();
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}

	return MacrosBanks::empty_macros_profiles_;
}

} // namespace GLogiK

