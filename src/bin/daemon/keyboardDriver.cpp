/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2020  Fabrice Delliaux <netbox253@gmail.com>
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
#include <chrono>
#include <algorithm>
#include <sstream>

#include "lib/shared/glogik.hpp"
#include "lib/utils/utils.hpp"

#if GKDBUS
#include "lib/dbus/GKDBus.hpp"
#endif

#include "keyboardDriver.hpp"

#include "daemonControl.hpp"

namespace GLogiK
{

using namespace NSGKUtils;


/*
 *	KeyboardDriver
 */

constexpr unsigned char KeyboardDriver::hidKeyboard[256];
std::vector<std::string> KeyboardDriver::macrosKeysNames = {};

/* KEY_FOO from linux/input-event-codes.h */
const std::vector< ModifierKey > KeyboardDriver::modifierKeys = {
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

#if DEBUGGING_ON && DEBUG_KEYS
const std::string KeyboardDriver::getBytes(const USBDevice & device) const
{
	const unsigned int last_length = toUInt(device.getLastKeysInterruptTransferLength());
	if( last_length == 0 )
		return "";
	std::ostringstream s;
	s << std::hex << toUInt(device._pressedKeys[0]);
	for(unsigned int x = 1; x < last_length; x++) {
		s << ", " << std::hex << toUInt(device._pressedKeys[x]);
	}
	return s.str();
}
#endif


const bool KeyboardDriver::checkDeviceCapability(const USBDeviceID & device, Caps toCheck) {
	return (device.getCapabilities() & toEnumType(toCheck));
}

const std::vector<std::string> & KeyboardDriver::getEmptyStringVector(void) {
	KeyboardDriver::macrosKeysNames.clear();
	return KeyboardDriver::macrosKeysNames;
}

KeyStatus KeyboardDriver::getPressedKeys(USBDevice & device) {
	std::fill_n(device._pressedKeys, KEYS_BUFFER_LENGTH, 0);

	int ret = this->performKeysInterruptTransfer(device, 10);

	switch(ret) {
		case 0:
			if( device.getLastKeysInterruptTransferLength() > 0 ) {
#if DEBUGGING_ON && DEBUG_KEYS
				LOG(DEBUG)	<< device.getID()
							<< " exp. rl: " << toInt(device.getKeysInterruptBufferMaxLength())
							<< " act_l: " << device.getLastKeysInterruptTransferLength() << ", xBuf[0]: "
							<< std::hex << toUInt(device._pressedKeys[0]);
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
			std::ostringstream err(device.getID(), std::ios_base::app);
			err << " getPressedKeys interrupt read error";
			GKSysLog(LOG_ERR, ERROR, err.str());
			device._fatalErrors++;
			return KeyStatus::S_KEY_SKIPPED;
			break;
	}

	return KeyStatus::S_KEY_TIMEDOUT;
}

void KeyboardDriver::notImplemented(const char* func) const {
	std::ostringstream buffer(std::ios_base::app);
	buffer << "not implemented : " << func;
	GKSysLog(LOG_WARNING, WARNING, buffer.str());
}

void KeyboardDriver::sendUSBDeviceInitialization(USBDevice & device) {
	this->notImplemented(__func__);
}

void KeyboardDriver::setDeviceMxKeysLeds(USBDevice & device) {
	this->notImplemented(__func__);
}

void KeyboardDriver::setDeviceBacklightColor(
	USBDevice & device,
	const uint8_t r,
	const uint8_t g,
	const uint8_t b)
{
	this->notImplemented(__func__);
}

/*
 * return true if leds_mask has been updated (meaning that setDeviceMxKeysLeds should be called)
 */
const bool KeyboardDriver::updateDeviceMxKeysLedsMask(USBDevice & device, bool disableMR) {
	auto & mask = device._MxKeysLedsMask;
	/* is macro record mode enabled ? */
	bool MR_ON = mask & toEnumType(Leds::GK_LED_MR);
	bool Mx_ON = false;
	bool mask_updated = false;

	if( device._pressedRKeysMask & toEnumType(Keys::GK_KEY_M1) ) {
		Mx_ON = mask & toEnumType(Leds::GK_LED_M1);
		mask = 0;
		mask_updated = true;
		device._pMacrosManager->setCurrentMacrosBankID(BankID::BANK_M0);
		if( ! Mx_ON ) {
			mask |= toEnumType(Leds::GK_LED_M1);
			device._pMacrosManager->setCurrentMacrosBankID(BankID::BANK_M1);
		}
	}
	else if( device._pressedRKeysMask & toEnumType(Keys::GK_KEY_M2) ) {
		Mx_ON = mask & toEnumType(Leds::GK_LED_M2);
		mask = 0;
		mask_updated = true;
		device._pMacrosManager->setCurrentMacrosBankID(BankID::BANK_M0);
		if( ! Mx_ON ) {
			mask |= toEnumType(Leds::GK_LED_M2);
			device._pMacrosManager->setCurrentMacrosBankID(BankID::BANK_M2);
		}
	}
	else if( device._pressedRKeysMask & toEnumType(Keys::GK_KEY_M3) ) {
		Mx_ON = mask & toEnumType(Leds::GK_LED_M3);
		mask = 0;
		mask_updated = true;
		device._pMacrosManager->setCurrentMacrosBankID(BankID::BANK_M0);
		if( ! Mx_ON ) {
			mask |= toEnumType(Leds::GK_LED_M3);
			device._pMacrosManager->setCurrentMacrosBankID(BankID::BANK_M3);
		}
	}

	if( device._pressedRKeysMask & toEnumType(Keys::GK_KEY_MR) ) {
		if(! MR_ON) { /* MR off, enable it */
			mask |= toEnumType(Leds::GK_LED_MR);
		}
		else { /* MR on, disable it */
			mask &= ~(toEnumType(Leds::GK_LED_MR));
		}
		mask_updated = true;
	}
	else if(disableMR) {
		mask &= ~(toEnumType(Leds::GK_LED_MR));
		mask_updated = true;
	}

	return mask_updated;
}

const uint8_t KeyboardDriver::handleModifierKeys(USBDevice & device, const uint16_t interval) {
	if( device._previousPressedKeys[1] == device._pressedKeys[1] )
		return 0; /* nothing changed here */

	KeyEvent e;
	e.interval = interval;
	uint8_t diff = 0;
	uint8_t ret = 0;

	/* some modifier keys were released */
	if( device._previousPressedKeys[1] > device._pressedKeys[1] ) {
		diff = device._previousPressedKeys[1] - device._pressedKeys[1];
		e.event = EventValue::EVENT_KEY_RELEASE;
	}
	/* some modifier keys were pressed */
	else {
		diff = device._pressedKeys[1] - device._previousPressedKeys[1];
		e.event = EventValue::EVENT_KEY_PRESS;
	}

	for(const auto & mKey : KeyboardDriver::modifierKeys) {
		const uint8_t modKey = toEnumType(mKey.key);
		if( diff & modKey ) { /* modifier key was pressed or released */
			diff -= modKey;

			bool skipEvent = false;
			const uint8_t size = device._newMacro.size();

			/* Macro Size Limit - see keyEvent.hpp */
			if(size >= MACRO_T_MAX_SIZE) {
				/* skip all new events */
#if DEBUGGING_ON
				LOG(WARNING) << "skipping modifier key event, reached macro max size";
#endif
				skipEvent = true;
			}

			/* Macro Size Limit - see keyEvent.hpp */
			if( ! skipEvent and (size >= MACRO_T_KEYPRESS_MAX_SIZE) ) {
				/* skip events other than release */
				if( e.event != EventValue::EVENT_KEY_RELEASE ) {
#if DEBUGGING_ON
					LOG(WARNING) << "skipping modifier keypress event, reached macro keypress max size";
#endif
					skipEvent = true;
				}
			}

			if( ! skipEvent ) {
				/*
				 * some modifier keys events were already appended,
				 * only first event should have a real timelapse interval
				 */
				if(ret > 0)
					e.interval = 1;
				e.code = mKey.code;
				device._newMacro.push_back(e);
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
uint16_t KeyboardDriver::getTimeLapse(USBDevice & device) {
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - device._lastTimePoint);
	device._lastTimePoint = std::chrono::steady_clock::now();
	return (ms.count() % 1000); /* max 1 second TODO */
}

void KeyboardDriver::fillStandardKeysEvents(USBDevice & device) {
	unsigned int i = 0;

	uint16_t interval = this->getTimeLapse(device);

	const uint8_t num = this->handleModifierKeys(device, interval);

	if( num > 0 ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << toUInt(num) << " modifier keys event(s) added";
#endif
		/* only first event should have a real timelapse interval */
		interval = 1;
	}

#if DEBUGGING_ON
	LOG(DEBUG2) << "	b	|	p";
	LOG(DEBUG2) << "  ------------------------------";
#endif
	for(i = device.getKeysInterruptBufferMaxLength()-1; i >= 2; --i) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "	" << toUInt(device._pressedKeys[i])
					<< "	|	" << toUInt(device._previousPressedKeys[i]);
#endif
		if( device._previousPressedKeys[i] == device._pressedKeys[i] ) {
			continue; /* nothing here */
		}
		else {
			/* Macro Size Limit - see keyEvent.hpp */
			if(device._newMacro.size() >= MACRO_T_MAX_SIZE ) {
				/* skip all new events */
#if DEBUGGING_ON
				LOG(WARNING) << "skipping key event, reached macro max size";
#endif
				continue;
			}

			KeyEvent e;
			e.interval = interval;

			if( device._previousPressedKeys[i] == 0 ) {
				e.code = KeyboardDriver::hidKeyboard[ device._pressedKeys[i] ];
				e.event = EventValue::EVENT_KEY_PRESS; /* KeyPress */
			}
			else if( device._pressedKeys[i] == 0 ) {
				e.code = KeyboardDriver::hidKeyboard[ device._previousPressedKeys[i] ];
				e.event = EventValue::EVENT_KEY_RELEASE; /* KeyRelease */
			}
			else {
				GKSysLog(LOG_WARNING, WARNING, "two different byte values");
				continue;
			}

			/* Macro Size Limit - see keyEvent.hpp */
			if(device._newMacro.size() >= MACRO_T_KEYPRESS_MAX_SIZE ) {
				/* skip events other than release */
				if( e.event != EventValue::EVENT_KEY_RELEASE ) {
#if DEBUGGING_ON
					LOG(WARNING) << "skipping keypress event, reached macro keypress max size";
#endif
					continue;
				}
			}

			device._newMacro.push_back(e);
			/* only first event should have a real timelapse interval */
			interval = 1;
		}
	}
}

void KeyboardDriver::checkDeviceFatalErrors(USBDevice & device, const std::string & place) const {
	/* check to give up */
	if(device._fatalErrors > DEVICE_LISTENING_THREAD_MAX_ERRORS) {
		std::ostringstream err(device.getID(), std::ios_base::app);
		err << "[" << place << "]" << " device " << device.getName() << " on bus " << toUInt(device.getBus());
		GKSysLog(LOG_ERR, ERROR, err.str());
		GKSysLog(LOG_ERR, ERROR, "reached listening thread maximum fatal errors, giving up");
		device.deactivateThreads();
	}
}

void KeyboardDriver::enterMacroRecordMode(USBDevice & device, const std::string & devID) {
#if DEBUGGING_ON
	LOG(DEBUG) << device.getID() << " entering macro record mode";
#endif

#if GKDBUS
	NSGKDBus::GKDBus* pDBus = nullptr;

	/* ROOT_NODE only for introspection, don't care */
	try {
		pDBus = new NSGKDBus::GKDBus(GLOGIK_DAEMON_DBUS_ROOT_NODE, GLOGIK_DAEMON_DBUS_ROOT_NODE_PATH);
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		LOG(ERROR) << "GKDBus bad allocation";
		pDBus = nullptr;
	}
#endif

	auto & exit = device._exitMacroRecordMode;
	exit = false;
	/* initializing time_point */
	device._lastTimePoint = std::chrono::steady_clock::now();

	while( (! exit) and DaemonControl::isDaemonRunning() ) {
		this->checkDeviceFatalErrors(device, "macro record loop");
		if( ! device.getThreadsStatus() )
			break;

		KeyStatus ret = this->getPressedKeys(device);

		switch( ret ) {
			case KeyStatus::S_KEY_PROCESSED: {
				/* did we press one Mx key ? */
				if( device._pressedRKeysMask & toEnumType(Keys::GK_KEY_M1) or
					device._pressedRKeysMask & toEnumType(Keys::GK_KEY_M2) or
					device._pressedRKeysMask & toEnumType(Keys::GK_KEY_M3) or
					device._pressedRKeysMask & toEnumType(Keys::GK_KEY_MR) ) {
					/* exiting macro record mode */
					exit = true;
					continue;
				}

				if( ! this->checkMacroKey(device) ) {
					/* continue to store standard key events
					 * while a macro key is not pressed */
					continue;
				}

				bool setMacro = true;
				/* don't set macro and don't send signal if
				 * no new macro defined and current macro for the pressed key
				 * is already empty
				 */
				if( ! device._pMacrosManager->macroDefined(device._macroKey) and
					  device._newMacro.empty() ) {
					setMacro = false;
#if DEBUGGING_ON
					LOG(DEBUG2) << device.getID() << " no change for key: " << device._macroKey;
#endif
				}

				if(setMacro) {
					try {
						/* macro key pressed, recording macro */
						device._pMacrosManager->setMacro(device._macroKey, device._newMacro);

#if GKDBUS
						if( pDBus ) {
							const uint8_t bankID = toEnumType( device._pMacrosManager->getCurrentMacrosBankID() );

							/* open a new connection, GKDBus is not thread-safe */
							pDBus->connectToSystemBus(GLOGIK_DEVICE_THREAD_DBUS_BUS_CONNECTION_NAME);

							try {
								std::string signal("MacroRecorded");
								if( device._newMacro.empty() ) {
									signal = "MacroCleared";
								}

								pDBus->initializeBroadcastSignal(
									NSGKDBus::BusConnection::GKDBUS_SYSTEM,
									GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
									GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
									signal.c_str()
								);

								pDBus->appendStringToBroadcastSignal(devID);
								pDBus->appendStringToBroadcastSignal(device._macroKey);
								pDBus->appendUInt8ToBroadcastSignal(bankID);

								pDBus->sendBroadcastSignal();
							}
							catch (const GKDBusMessageWrongBuild & e) {
								pDBus->abandonBroadcastSignal();
								GKSysLog(LOG_WARNING, WARNING, e.what());
							}
						}
#endif
					}
					catch (const GLogiKExcept & e) {
						GKSysLog(LOG_WARNING, WARNING, e.what());
					}
				}

				exit = true;
				break;
			}

			default:
				break;
		}
	}

	device._newMacro.clear();

#if GKDBUS
	delete pDBus;
	pDBus = nullptr;
#endif

#if DEBUGGING_ON
	LOG(DEBUG) << device.getID() << " exiting macro record mode";
#endif
}

void KeyboardDriver::runMacro(const std::string & devID) const {
	try {
		const USBDevice & device = _initializedDevices.at(devID);
#if DEBUGGING_ON
		LOG(DEBUG2) << device.getID() << " spawned running macro thread for " << device.getName();
#endif
		device._pMacrosManager->runMacro(device._macroKey);
#if DEBUGGING_ON
		LOG(DEBUG2) << device.getID() << " exiting running macro thread";
#endif
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
}

void KeyboardDriver::LCDScreenLoop(const std::string & devID) {
	try {
		USBDevice & device = _initializedDevices.at(devID);
		device._LCDThreadID = std::this_thread::get_id();

#if DEBUGGING_ON
		LOG(INFO) << device.getID() << " spawned LCD screen thread for " << device.getName();
#endif

		while( DaemonControl::isDaemonRunning() ) {
			this->checkDeviceFatalErrors(device, "LCD screen loop");
			if( ! device.getThreadsStatus() )
				break;

			auto t1 = std::chrono::high_resolution_clock::now();

			std::string LCDKey;
			uint64_t LCDPluginsMask1 = 0;

			{
				yield_for(std::chrono::microseconds(100));
				std::lock_guard<std::mutex> lock(device._LCDMutex);
				if( ! device._LCDKey.empty() ) {
					LCDKey = device._LCDKey;
					device._LCDKey.clear();
				}
				LCDPluginsMask1 = device._LCDPluginsMask1;
			}

			LCDDataArray & LCDBuffer = device._pLCDPluginsManager->getNextLCDScreenBuffer(LCDKey, LCDPluginsMask1);
			int ret = this->performLCDScreenInterruptTransfer(
				device,
				LCDBuffer.data(),
				LCDBuffer.size(),
				1000);

			auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1);
			auto one = std::chrono::milliseconds( device._pLCDPluginsManager->getPluginTiming() );

#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
			LOG(DEBUG1) << "refreshed LCD screen for " << device.getName()
				<< " - ret: " << ret
				<< " - interval: " << interval.count()
				<< " - next sleep: " << one.count();
#endif
			if(ret != 0) { // TODO stop thread ?
				GKSysLog(LOG_ERR, ERROR, "LCD refresh failure");
			}

			if( interval < one ) {
				one -= interval;
				std::this_thread::sleep_for(one);
			}
		}

		device._pLCDPluginsManager->unlockPlugin();
		device._pLCDPluginsManager->jumpToNextPlugin();

		LCDDataArray & LCDBuffer = device._pLCDPluginsManager->getNextLCDScreenBuffer("", toEnumType(LCDScreenPlugin::GK_LCD_ENDSCREEN));
		int ret = this->performLCDScreenInterruptTransfer( device, LCDBuffer.data(), LCDBuffer.size(), 1000);
		if(ret != 0) {
			GKSysLog(LOG_ERR, ERROR, "endscreen LCD refresh failure");
		}

#if DEBUGGING_ON
		LOG(INFO) << device.getID() << " exiting LCD screen thread for " << device.getName();
#endif
	} /* try */
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
	catch (const GLogiKExcept & e) {
		GKSysLog(LOG_ERR, ERROR, e.what());
	}
	catch( const std::exception & e ) {
		std::ostringstream err("uncaught std::exception : ", std::ios_base::app);
		err << e.what();
		GKSysLog(LOG_ERR, ERROR, err.str());
	}
}

void KeyboardDriver::listenLoop(const std::string & devID) {
	try {
		USBDevice & device = _initializedDevices.at(devID);
		device._keysThreadID = std::this_thread::get_id();

#if DEBUGGING_ON
		LOG(INFO) << device.getID() << " spawned listening thread for " << device.getName();
#endif

		if( this->checkDeviceCapability(device, Caps::GK_LCD_SCREEN) ) {
			std::thread lcd_thread(&KeyboardDriver::LCDScreenLoop, this, devID);
			std::lock_guard<std::mutex> lock(_threadsMutex);
			_threads.push_back( std::move(lcd_thread) );
		}

		while( DaemonControl::isDaemonRunning() ) {
			this->checkDeviceFatalErrors(device, "listen loop");
			if( ! device.getThreadsStatus() )
				break;

			KeyStatus ret = this->getPressedKeys(device);
			switch( ret ) {
				case KeyStatus::S_KEY_PROCESSED:

					if( this->checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
						/*
						 * update M1-MR leds status, launch macro record mode and
						 * run macros only after proper event length
						 */
						if( device.getLastKeysInterruptTransferLength() == device.getMacrosKeysLength() ) {
							/* update mask with potential pressed keys */
							if(this->updateDeviceMxKeysLedsMask(device))
								this->setDeviceMxKeysLeds(device);

							/* is macro record mode enabled ? */
							if( device._MxKeysLedsMask & toEnumType(Leds::GK_LED_MR) ) {
								this->enterMacroRecordMode(device, devID);

								/* don't need to update leds status if the mask is already 0 */
								if(device._MxKeysLedsMask != 0) {
									/* disabling macro record mode */
									if(this->updateDeviceMxKeysLedsMask(device, true))
										this->setDeviceMxKeysLeds(device);
								}
							}
							else { /* check to run macro */
								if( this->checkMacroKey(device) ) {
									try {
										/* spawn thread only if macro defined */
										if(device._pMacrosManager->macroDefined(device._macroKey)) {
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
					}

					if( this->checkDeviceCapability(device, Caps::GK_MEDIA_KEYS) ) {
						if( device.getLastKeysInterruptTransferLength() == device.getMediaKeysLength() ) {
							if( this->checkMediaKey(device) ) {
#if GKDBUS
								NSGKDBus::GKDBus* pDBus = nullptr;

								/* ROOT_NODE only for introspection, don't care */
								try {
									pDBus = new NSGKDBus::GKDBus(GLOGIK_DAEMON_DBUS_ROOT_NODE, GLOGIK_DAEMON_DBUS_ROOT_NODE_PATH);
								}
								catch (const std::bad_alloc& e) { /* handle new() failure */
									LOG(ERROR) << "GKDBus bad allocation";
									pDBus = nullptr;
								}

								if( pDBus ) {
									pDBus->connectToSystemBus(GLOGIK_DEVICE_THREAD_DBUS_BUS_CONNECTION_NAME);

									try {
										pDBus->initializeBroadcastSignal(
											NSGKDBus::BusConnection::GKDBUS_SYSTEM,
											GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
											GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
											"deviceMediaEvent"
										);

										pDBus->appendStringToBroadcastSignal(devID);
										pDBus->appendStringToBroadcastSignal(device._mediaKey);

										pDBus->sendBroadcastSignal();
									}
									catch (const GKDBusMessageWrongBuild & e) {
										pDBus->abandonBroadcastSignal();
										GKSysLog(LOG_WARNING, WARNING, e.what());
									}
								}

								delete pDBus; pDBus = nullptr;
#endif
							}
						}
					}

					if( this->checkDeviceCapability(device, Caps::GK_LCD_SCREEN) ) {
						if( device.getLastKeysInterruptTransferLength() == device.getLCDKeysLength() ) {
							if( this->checkLCDKey(device) ) {
#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
								std::lock_guard<std::mutex> lock(device._LCDMutex);
								LOG(DEBUG2) << device.getID() << " LCD key pressed : " << device._LCDKey;
#endif
							}
						}
					}
					break;
				default:
					break;
			}
		}

#if DEBUGGING_ON
		LOG(INFO) << device.getID() << " exiting listening thread for " << device.getName();
#endif
	} /* try */
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
	catch (const std::system_error& e) {
		GKSysLog(LOG_ERR, ERROR, "error while spawning LCDScreen loop thread");
	}
	catch( const std::exception & e ) {
		std::ostringstream err("uncaught std::exception : ", std::ios_base::app);
		err << e.what();
		GKSysLog(LOG_ERR, ERROR, err.str());
	}
}

const bool KeyboardDriver::getDeviceThreadsStatus(const std::string & devID) const
{
	try {
		return _initializedDevices.at(devID).getThreadsStatus();
	} /* try */
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}

	return false;
}

void KeyboardDriver::resetDeviceState(USBDevice & device) {
	if( this->checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
		/* exit MacroRecordMode if necessary */
		device._exitMacroRecordMode = true;

		device._pMacrosManager->resetMacrosBanks();

#if DEBUGGING_ON
		LOG(DEBUG1) << device.getID() << " resetting device MxKeys leds status";
#endif
		device._MxKeysLedsMask = 0;
		this->setDeviceMxKeysLeds(device);
	}

	if( this->checkDeviceCapability(device, Caps::GK_BACKLIGHT_COLOR) ) {
#if DEBUGGING_ON
		LOG(DEBUG1) << device.getID() << " resetting device backlight color";
#endif
		this->setDeviceBacklightColor(device);
	}

	if( this->checkDeviceCapability(device, Caps::GK_LCD_SCREEN) ) {
#if DEBUGGING_ON
		LOG(DEBUG1) << device.getID() << " resetting device LCD plugins mask";
#endif
		this->setDeviceLCDPluginsMask(device);
	}
}

void KeyboardDriver::resetDeviceState(const USBDeviceID & det)
{
	const std::string & devID = det.getID();

	try {
		USBDevice & device = _initializedDevices.at(devID);
#if DEBUGGING_ON
		LOG(DEBUG3) << devID << " resetting state of " << device.getName() << "("
					<< device.getVendorID() << ":" << device.getProductID() << "), device "
					<< toUInt(device.getNum()) << " on bus " << toUInt(device.getBus());
#endif

		this->resetDeviceState(device);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
}

void KeyboardDriver::setDeviceLCDPluginsMask(USBDevice & device, uint64_t mask)
{
	if( mask == 0 ) {
		/* default enabled plugins */
		mask |= toEnumType(LCDScreenPlugin::GK_LCD_SPLASHSCREEN);
		mask |= toEnumType(LCDScreenPlugin::GK_LCD_SYSTEM_MONITOR);
	}

#if DEBUGGING_ON
	LOG(DEBUG3) << device.getID() << " setting device LCD plugins mask to : " << mask;
#endif

	{
		yield_for(std::chrono::microseconds(100));
		std::lock_guard<std::mutex> lock(device._LCDMutex);
		device._LCDPluginsMask1 = mask;
	}

	/* Current active plugin may be locked */
	device._pLCDPluginsManager->unlockPlugin();

	/* jump if current active plugin is not in the new mask */
	if( ! (device._pLCDPluginsManager->getCurrentPluginID() & mask) ) {
		device._pLCDPluginsManager->jumpToNextPlugin();
	}
}

void KeyboardDriver::joinDeviceThreads(const USBDevice & device)
{
	bool found = false;
	std::thread::id thread_id;

	auto find_thread = [&thread_id, &found] (auto & item) -> const bool {
		if( thread_id == item.get_id() ) {
			found = true;
#if DEBUGGING_ON
			LOG(DEBUG1) << "thread found !";
#endif
			item.join();
			return true;
		}
		return false;
	};

	found = false; thread_id = device._keysThreadID;
#if DEBUGGING_ON
	LOG(DEBUG) << device.getID() << " waiting for listening thread";
#endif
	{
		std::lock_guard<std::mutex> lock(_threadsMutex);
		_threads.erase(
			std::remove_if(_threads.begin(), _threads.end(), find_thread),
			_threads.end()
		);
	}

	if(! found) {
		GKSysLog(LOG_WARNING, WARNING, "listening thread not found !");
	}

	/* Caps::GK_LCD_SCREEN */
	if( this->checkDeviceCapability(device, Caps::GK_LCD_SCREEN) ) {
		found = false; thread_id = device._LCDThreadID;

#if DEBUGGING_ON
		LOG(DEBUG) << device.getID() << " waiting for LCD screen thread";
#endif
		{
			std::lock_guard<std::mutex> lock(_threadsMutex);
			_threads.erase(
				std::remove_if(_threads.begin(), _threads.end(), find_thread),
				_threads.end()
			);
		}

		if(! found) {
			GKSysLog(LOG_WARNING, WARNING, "LCD screen thread not found !");
		}
	}
}

/* called when setting active user's configuration */
void KeyboardDriver::setDeviceActiveConfiguration(
	const std::string & devID,
	const banksMap_type & macrosBanks,
	const uint8_t r,
	const uint8_t g,
	const uint8_t b,
	const uint64_t LCDPluginsMask1
) {
	try {
		USBDevice & device = _initializedDevices.at(devID);

		if( this->checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
			/* exit MacroRecordMode if necessary */
			device._exitMacroRecordMode = true;

			/* set macros banks */
			device._pMacrosManager->setMacrosBanks(macrosBanks);
		}

		if( this->checkDeviceCapability(device, Caps::GK_BACKLIGHT_COLOR) ) {
			/* set backlight color */
			this->setDeviceBacklightColor(device, r, g, b);
		}

		if( this->checkDeviceCapability(device, Caps::GK_LCD_SCREEN) ) {
			this->setDeviceLCDPluginsMask(device, LCDPluginsMask1);
		}
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
}

const banksMap_type & KeyboardDriver::getDeviceMacrosBanks(const std::string & devID) const {
	try {
		const USBDevice & device = _initializedDevices.at(devID);
		if( this->checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
			return device._pMacrosManager->getMacrosBanks();
		}
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}

	return MacrosBanks::emptyMacrosBanks;
}

const LCDPluginsPropertiesArray_type & KeyboardDriver::getDeviceLCDPluginsProperties(
	const std::string & devID
) const
{
	try {
		const USBDevice & device = _initializedDevices.at(devID);
		return device._pLCDPluginsManager->getLCDPluginsProperties();
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}

	return LCDScreenPluginsManager::_LCDPluginsPropertiesEmptyArray;
}

/* throws GLogiKExcept on any failure */
void KeyboardDriver::openDevice(const USBDeviceID & det)
{
	/* sanity check */
	if(_initializedDevices.count(det.getID()) > 0) {
		std::ostringstream err("device ID already used in started container : ", std::ios_base::app);
		err << det.getID();
		throw GLogiKExcept(err.str());
	}

#if DEBUGGING_ON
	LOG(DEBUG3) << det.getID() << " initializing " << det.getName() << "("
				<< det.getVendorID() << ":" << det.getProductID() << "), device "
				<< toUInt(det.getNum()) << " on bus " << toUInt(det.getBus());
#endif

	USBDevice device(
		det.getName(),
		det.getVendorID(),
		det.getProductID(),
		det.getCapabilities(),
		det.getBus(),
		det.getNum(),
		/* USB_INTERFACE_DESCRIPTOR */
		det.getBConfigurationValue(),
		det.getBInterfaceNumber(),
		det.getBAlternateSetting(),
		det.getBNumEndpoints(),
		det.getKeysInterruptBufferMaxLength(),
		det.getMacrosKeysLength(),
		det.getMediaKeysLength(),
		det.getLCDKeysLength()
	);

	this->openUSBDevice(device); /* throws on any failure */
	/* libusb device opened */

	try {
		this->sendUSBDeviceInitialization(device);

		if( this->checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
			/* virtual keyboard name */
			std::ostringstream buffer(std::ios_base::app);
			buffer	<< "Virtual " << device.getName()
					<< " " << device.getID();

			device.initializeMacrosManager(
				buffer.str().c_str(),
				this->getMacroKeysNames()
			);
		}

		if( this->checkDeviceCapability(device, Caps::GK_LCD_SCREEN) ) {
			device.initializeLCDPluginsManager();
		}

		this->resetDeviceState(device);

		_initializedDevices[ device.getID() ] = device;

		/* spawn listening thread */
		try {
			std::thread listen_thread(&KeyboardDriver::listenLoop, this, device.getID() );
			std::lock_guard<std::mutex> lock(_threadsMutex);
			_threads.push_back( std::move(listen_thread) );
		}
		catch (const std::system_error& e) {
			std::ostringstream buffer(std::ios_base::app);
			buffer << "error while spawning listening thread : " << e.what();
			throw GLogiKExcept(buffer.str());
		}
	}
	catch ( const GLogiKExcept & e ) {
		device.destroyMacrosManager();
		device.destroyLCDPluginsManager();

		this->closeUSBDevice(device);
		throw;
	}
}

void KeyboardDriver::closeDevice(const USBDeviceID & det) noexcept
{
	const std::string & devID = det.getID();

	try {
		USBDevice & device = _initializedDevices.at(devID);
#if DEBUGGING_ON
		LOG(DEBUG3) << device.getID() << " closing " << device.getName() << "("
					<< device.getVendorID() << ":" << device.getProductID() << "), device "
					<< toUInt(device.getNum()) << " on bus " << toUInt(device.getBus());
#endif

		device.deactivateThreads();
		this->joinDeviceThreads(device);
		this->resetDeviceState(device);
		device.destroyMacrosManager(); /* reset device state needs this pointer */
		device.destroyLCDPluginsManager();

		this->closeUSBDevice(device);

		_initializedDevices.erase(devID);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
}

} // namespace GLogiK

