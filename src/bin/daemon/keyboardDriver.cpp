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

#include <utility>

#include "lib/utils/utils.h"
#include "lib/dbus/GKDBus.h"
#include "lib/shared/glogik.h"

#include <libusb-1.0/libusb.h>

#include "daemonControl.h"
#include "keyboardDriver.h"

#include "include/enums.h"

namespace GLogiK
{

using namespace NSGKUtils;

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

KeyboardDriver::KeyboardDriver(int key_read_length, uint8_t event_length, const DescriptorValues & values)
	:	LibUSB(values)
{
	this->leds_update_event_length_ = event_length;

	if( key_read_length > KEYS_BUFFER_LENGTH ) {
		GKSysLog(LOG_WARNING, WARNING, "interrupt read length too large, set it to max buffer length");
		key_read_length = KEYS_BUFFER_LENGTH;
	}

	this->interrupt_key_read_length = key_read_length;
}

KeyboardDriver::~KeyboardDriver() {
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
				LOG(DEBUG)	<< device.strID
							<< " exp. rl: " << this->interrupt_key_read_length
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
			std::ostringstream err(device.strID, std::ios_base::app);
			err << " getPressedKeys interrupt read error";
			GKSysLog(LOG_ERR, ERROR, err.str());
			this->USBError(ret);
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

void KeyboardDriver::sendUSBDeviceInitialization(const InitializedDevice & device) {
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
		std::ostringstream err(device.strID, std::ios_base::app);
		err << "device " << device.device.name << " on bus " << to_uint(device.bus);
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
				break;

			default:
				break;
		}
	}

	device.standard_keys_events.clear();

	delete pDBus;
	pDBus = nullptr;

#if DEBUGGING_ON
	LOG(DEBUG) << "exiting macro record mode";
#endif
}

void KeyboardDriver::runMacro(const std::string & devID) {
	try {
		InitializedDevice & device = this->initialized_devices_.at(devID);
#if DEBUGGING_ON
		LOG(DEBUG2) << device.strID << " spawned running macro thread for " << device.device.name;
#endif
		device.macros_man->runMacro(device.chosen_macro_key);
#if DEBUGGING_ON
		LOG(DEBUG2) << device.strID << " exiting running macro thread";
#endif
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
}

void KeyboardDriver::listenLoop(const std::string & devID) {
	try {
		InitializedDevice & device = this->initialized_devices_.at(devID);
		device.listen_thread_id = std::this_thread::get_id();

#if DEBUGGING_ON
		LOG(INFO) << device.strID << " spawned listening thread for " << device.device.name;
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
						/* update mask with potential pressed keys */
						if(this->updateCurrentLedsMask(device))
							this->setMxKeysLeds(device);

						/* is macro record mode enabled ? */
						if( mask & to_type(Leds::GK_LED_MR) ) {
							this->enterMacroRecordMode(device, devID);

							/* don't need to update leds status if the mask is already 0 */
							if(device.current_leds_mask != 0) {
								/* disabling macro record mode */
								if(this->updateCurrentLedsMask(device, true))
									this->setMxKeysLeds(device);
							}
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
		LOG(INFO) << device.strID << " exiting listening thread for " << device.device.name;
#endif
	} /* try */
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
	catch( const std::exception & e ) {
		std::ostringstream err("uncaught std::exception : ", std::ios_base::app);
		err << e.what();
		GKSysLog(LOG_ERR, ERROR, err.str());
	}
}

void KeyboardDriver::initializeDevice(const KeyboardDevice &dev, const uint8_t bus, const uint8_t num) {
	const std::string devID = KeyboardDriver::getDeviceID(bus, num);
	this->buffer_.str("[");
	this->buffer_ << devID << "]";
	const std::string strID(this->buffer_.str());

#if DEBUGGING_ON
	LOG(DEBUG3) << strID << " trying to initialize " << dev.name << "("
				<< dev.vendor_id << ":" << dev.product_id << "), device "
				<< to_uint(num) << " on bus " << to_uint(bus);
#endif

	InitializedDevice device(dev, bus, num, strID);

	this->openUSBDevice(device); /* throws on any failure */
	/* libusb device opened */

	try {
		this->setUSBDeviceActiveConfiguration(device);
		this->findUSBDeviceInterface(device);
		this->sendUSBDeviceInitialization(device);

		/* virtual keyboard name */
		this->buffer_.str("Virtual ");
		this->buffer_ << device.device.name << " "<< device.strID;

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
		LOG(DEBUG1) << device.strID << " resetting MxKeys leds status";
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
		this->releaseUSBDeviceInterfaces(device);
		this->closeUSBDevice(device);
		throw;
	}
}

const bool KeyboardDriver::isDeviceInitialized(const std::string & devID) const {
	return ( this->initialized_devices_.count(devID) == 1 );
}

void KeyboardDriver::resetDeviceState(InitializedDevice & device) {
	/* exit MacroRecordMode if necessary */
	device.exit_macro_record_mode = true;

	device.macros_man->clearMacroProfiles();

#if DEBUGGING_ON
	LOG(DEBUG1) << device.strID << " resetting MxKeys leds status";
#endif
	device.current_leds_mask = 0;
	this->setMxKeysLeds(device);

#if DEBUGGING_ON
	LOG(DEBUG1) << device.strID << " resetting keyboard backlight color";
#endif
	this->updateKeyboardColor(device);
	this->setKeyboardColor(device);
}

void KeyboardDriver::resetDeviceState(const KeyboardDevice &dev, const uint8_t bus, const uint8_t num) {
	const std::string devID = KeyboardDriver::getDeviceID(bus, num);

	try {
		InitializedDevice & device = this->initialized_devices_.at(devID);
#if DEBUGGING_ON
		LOG(DEBUG3) << "[" << devID << "] resetting state of " << dev.name << "("
					<< dev.vendor_id << ":" << dev.product_id << "), device "
					<< to_uint(num) << " on bus " << to_uint(bus);
#endif

		this->resetDeviceState(device);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
}

void KeyboardDriver::closeDevice(const KeyboardDevice &dev, const uint8_t bus, const uint8_t num) {
	const std::string devID = KeyboardDriver::getDeviceID(bus, num);

#if DEBUGGING_ON
	LOG(DEBUG3) << "[" << devID << "] trying to close " << dev.name << "("
				<< dev.vendor_id << ":" << dev.product_id << "), device "
				<< to_uint(num) << " on bus " << to_uint(bus);
#endif

	try {
		InitializedDevice & device = this->initialized_devices_.at(devID);
		device.listen_status = false;

		bool found = false;
		for(auto it = this->threads_.begin(); it != this->threads_.end();) {
			if( device.listen_thread_id == (*it).get_id() ) {
				found = true;
#if DEBUGGING_ON
				LOG(INFO) << device.strID << " waiting for listening thread";
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

		this->resetDeviceState(device);

		delete device.macros_man;
		device.macros_man = nullptr;

		this->releaseUSBDeviceInterfaces(device);
		this->closeUSBDevice(device);

		this->initialized_devices_.erase(devID);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
}

/* called when setting active user's configuration */
void KeyboardDriver::setDeviceActiveConfiguration(
	const std::string & devID,
	const macros_map_t & macros_profiles,
	const uint8_t r,
	const uint8_t g,
	const uint8_t b
) {
	try {
		InitializedDevice & device = this->initialized_devices_.at(devID);

		/* exit MacroRecordMode if necessary */
		device.exit_macro_record_mode = true;

		/* set macros profiles */
		device.macros_man->setMacrosProfiles(macros_profiles);

		/* set backlight color */
		this->updateKeyboardColor(device, r, g, b);
		this->setKeyboardColor(device);
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

