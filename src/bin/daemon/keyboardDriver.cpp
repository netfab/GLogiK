/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2022  Fabrice Delliaux <netbox253@gmail.com>
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

#include "keyboardDriver.hpp"

#include "daemonControl.hpp"
#include "USBAPIenums.hpp"

namespace GLogiK
{

using namespace NSGKUtils;


/*
 *	KeyboardDriver
 */

constexpr unsigned char KeyboardDriver::hidKeyboard[256];

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

#if GKDBUS
void KeyboardDriver::setDBus(NSGKDBus::GKDBus* pDBus)
{
	_pDBus = pDBus;
}
#endif

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


const bool KeyboardDriver::checkDeviceCapability(const USBDeviceID & device, Caps toCheck)
{
	return (device.getCapabilities() & toEnumType(toCheck));
}

KeyStatus KeyboardDriver::getPressedKeys(USBDevice & device)
{
	GK_LOG_FUNC

	std::fill_n(device._pressedKeys, KEYS_BUFFER_LENGTH, 0);

	int ret = this->performUSBDeviceKeysInterruptTransfer(device, 10);

	switch(ret) {
		case 0:
			if( device.getLastKeysInterruptTransferLength() > 0 ) {
#if DEBUGGING_ON && DEBUG_KEYS
				if(GKLogging::GKDebug) {
					LOG(trace)	<< device.getID()
								<< " exp. rl: " << toInt(device.getKeysInterruptBufferMaxLength())
								<< " act_l: " << device.getLastKeysInterruptTransferLength()
								<< " xBuf[0]: " << std::hex << toUInt(device._pressedKeys[0]);
				}
#endif
				return this->processKeyEvent(device);
			}
			break;
		case toEnumType(USBAPIKeysTransferStatus::TRANSFER_TIMEOUT):
			//GKLog(trace, "timeout reached")
			return KeyStatus::S_KEY_TIMEDOUT;
			break;
		default:
			GKSysLogError(device.getID(), " interrupt read error");
			device._fatalErrors++;
			return KeyStatus::S_KEY_SKIPPED;
			break;
	}

	return KeyStatus::S_KEY_TIMEDOUT;
}

void KeyboardDriver::notImplemented(const char* func) const
{
	GK_LOG_FUNC

	std::ostringstream buffer(std::ios_base::app);
	buffer << "not implemented : " << func;
	GKSysLogWarning(buffer.str());
}

void KeyboardDriver::sendUSBDeviceInitialization(USBDevice & device)
{
	this->notImplemented(__func__);
}

void KeyboardDriver::setDeviceMxKeysLeds(USBDevice & device)
{
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
const bool KeyboardDriver::updateDeviceMxKeysLedsMask(USBDevice & device, bool disableMR)
{
	auto & mask = device._MxKeysLedsMask;
	bool mask_updated = false;
	MKeysID pressed_MKey = MKeysID::MKEY_M0;

	/* was MR key enabled ? */
	const bool MR_ON = mask & toEnumType(Leds::GK_LED_MR);

	auto update_MxKey_mask = [&] (const Leds keyledmask, const MKeysID sMKey) -> void
	{
		/* was this Mx key already enabled */
		const bool Mx_ON = mask & toEnumType(keyledmask);

		/* an Mx key (M1, M2, or M3) was pressed, we must reset
		 * the mask, else two differents Mx keys LEDs could be
		 * on at the same time; this also disables Macro Record
		 * mode if MR LED was on */
		mask = 0;
		if( ! Mx_ON ) { /* Mx was off, enable it */
			mask |= toEnumType(keyledmask);
			pressed_MKey = sMKey;
		}
		mask_updated = true;
	};

	/* M1 key was pressed */
	if( device._pressedRKeysMask & toEnumType(Keys::GK_KEY_M1) ) {
		update_MxKey_mask(Leds::GK_LED_M1, MKeysID::MKEY_M1);
	}
	/* M2 key was pressed */
	else if( device._pressedRKeysMask & toEnumType(Keys::GK_KEY_M2) ) {
		update_MxKey_mask(Leds::GK_LED_M2, MKeysID::MKEY_M2);
	}
	/* M3 key was pressed */
	else if( device._pressedRKeysMask & toEnumType(Keys::GK_KEY_M3) ) {
		update_MxKey_mask(Leds::GK_LED_M3, MKeysID::MKEY_M3);
	}

	if( mask_updated ) { /* if a Mx key was pressed */
		try {
			_pDBus->initializeBroadcastSignal(
				NSGKDBus::BusConnection::GKDBUS_SYSTEM,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
				GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
				"GBankSwitch"
			);

			_pDBus->appendStringToBroadcastSignal(device.getID());
			_pDBus->appendMKeysIDToBroadcastSignal(pressed_MKey);

			_pDBus->sendBroadcastSignal();

			LOG(trace)	<< device.getID() << " sent DBus signal: GBankSwitch - M"
						<< pressed_MKey;
		}
		catch (const GKDBusMessageWrongBuild & e) {
			_pDBus->abandonBroadcastSignal();
			GKSysLogWarning(e.what());
		}
	}

	/* MR key was pressed */
	if( device._pressedRKeysMask & toEnumType(Keys::GK_KEY_MR) ) {
		if(! MR_ON) { /* MR was off, enable it */
			mask |= toEnumType(Leds::GK_LED_MR);
		}
		else { /* MR was on, disable it */
			mask &= ~(toEnumType(Leds::GK_LED_MR));
		}
		mask_updated = true;
	}
	else if(disableMR) { /* force disable MR */
		mask &= ~(toEnumType(Leds::GK_LED_MR));
		mask_updated = true;
	}

	return mask_updated;
}

const uint8_t KeyboardDriver::handleModifierKeys(USBDevice & device, const uint16_t interval)
{
	GK_LOG_FUNC

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

			/* Macro Size Limit - see base.hpp */
			if(size >= MACRO_T_MAX_SIZE) {
				/* skip all new events */
				skipEvent = true;

				GKLog(trace, "skipped modifier key event, reached macro max size")
			}

			/* Macro Size Limit - see base.hpp */
			if( ! skipEvent and (size >= MACRO_T_KEYPRESS_MAX_SIZE) ) {
				/* skip events other than release */
				if( e.event != EventValue::EVENT_KEY_RELEASE ) {
					skipEvent = true;

					GKLog(trace, "skipped modifier keypress event, reached macro keypress max size")
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
	GKSysLogWarning("diff not equal to zero");
	return ret;
}

/* used to create macros */
uint16_t KeyboardDriver::getTimeLapse(USBDevice & device)
{
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - device._lastTimePoint);
	device._lastTimePoint = std::chrono::steady_clock::now();
	return (ms.count() % 1000); /* max 1 second TODO */
}

void KeyboardDriver::fillStandardKeysEvents(USBDevice & device)
{
	GK_LOG_FUNC

	unsigned int i = 0;

	uint16_t interval = this->getTimeLapse(device);

	const uint8_t num = this->handleModifierKeys(device, interval);

	if( num > 0 ) {
		GKLog2(trace, toUInt(num), " modifier keys event(s) added")

		/* only first event should have a real timelapse interval */
		interval = 1;
	}

#if DEBUGGING_ON
	if(GKLogging::GKDebug) {
		LOG(trace) << "	b	|	p";
		LOG(trace) << "  ------------------------------";
	}
#endif

	for(i = device.getKeysInterruptBufferMaxLength()-1; i >= 2; --i) {

#if DEBUGGING_ON
		if(GKLogging::GKDebug) {
			LOG(trace)	<< "    " << toUInt(device._pressedKeys[i])
						<< "    |   " << toUInt(device._previousPressedKeys[i]);
		}
#endif

		if( device._previousPressedKeys[i] == device._pressedKeys[i] ) {
			continue; /* nothing here */
		}
		else {
			/* Macro Size Limit - see base.hpp */
			if(device._newMacro.size() >= MACRO_T_MAX_SIZE ) {
				/* skip all new events */
				GKLog(trace, "skipped key event, reached macro max size")
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
				GKSysLogWarning("two different byte values");
				continue;
			}

			/* Macro Size Limit - see base.hpp */
			if(device._newMacro.size() >= MACRO_T_KEYPRESS_MAX_SIZE ) {
				/* skip events other than release */
				if( e.event != EventValue::EVENT_KEY_RELEASE ) {
					GKLog(trace, "skipped keypress event, reached macro keypress max size")
					continue;
				}
			}

			device._newMacro.push_back(e);
			/* only first event should have a real timelapse interval */
			interval = 1;
		}
	}
}

void KeyboardDriver::checkDeviceFatalErrors(USBDevice & device, const std::string & place) const
{
	GK_LOG_FUNC

	/* check to give up */
	if(device._fatalErrors > DEVICE_LISTENING_THREAD_MAX_ERRORS) {
		std::ostringstream err(device.getID(), std::ios_base::app);
		err << "[" << place << "]" << " device " << device.getFullName()
			<< " on bus " << toUInt(device.getBus());
		GKSysLogError(err.str());
		GKSysLogError("reached listening thread maximum fatal errors, giving up");
		device.stopThreads();
	}
}

void KeyboardDriver::enterMacroRecordMode(USBDevice & device)
{
	GK_LOG_FUNC

	GKLog2(trace, device.getID(), " entering macro record mode")

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

#if GKDBUS
				try {
					std::string signal("MacroRecorded");
					if( device._newMacro.empty() ) {
						signal = "MacroCleared";
					}

					_pDBus->initializeBroadcastSignal(
						NSGKDBus::BusConnection::GKDBUS_SYSTEM,
						GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
						GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
						signal.c_str()
					);

					_pDBus->appendStringToBroadcastSignal(device.getID());
					_pDBus->appendGKeysIDToBroadcastSignal(device._GKeyID);

					/* sending macro */
					if( ! device._newMacro.empty() ) {
						// FIXME : macro size
						_pDBus->appendMacroToBroadcastSignal(device._newMacro);
					}

					_pDBus->sendBroadcastSignal();

					LOG(trace)	<< device.getID() << " sent DBus signal: "
								<< signal << " - " << getGKeyName(device._GKeyID);
				}
				catch (const GKDBusMessageWrongBuild & e) {
					_pDBus->abandonBroadcastSignal();
					GKSysLogWarning(e.what());
				}
#endif

				exit = true;
				break;
			}

			default:
				break;
		}
	}

	device._newMacro.clear();

	GKLog2(trace, device.getID(), " exiting macro record mode")
}

void KeyboardDriver::LCDScreenLoop(const std::string & devID)
{
	GK_LOG_FUNC

	try {
		USBDevice & device = _initializedDevices.at(devID);
		device._LCDThreadID = std::this_thread::get_id();

		GKLog3(trace, devID, " spawned LCD screen thread for ", device.getFullName())

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

			const PixelsData & LCDBuffer = device.getLCDPluginsManager()->getNextLCDScreenBuffer(LCDKey, LCDPluginsMask1);
			int ret = this->performUSBDeviceLCDScreenInterruptTransfer(
				device,
				LCDBuffer.data(),
				LCDBuffer.size(),
				1000
			);

			auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1);
			auto one = std::chrono::milliseconds( device.getLCDPluginsManager()->getPluginTiming() );

#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
			if(GKLogging::GKDebug) {
				LOG(trace)	<< devID << " refreshed LCD screen for "
							<< device.getFullName()
							<< " - ret: " << ret
							<< " - interval: " << interval.count()
							<< " - next sleep: " << one.count();
			}
#endif
			if(ret != 0) { // TODO stop thread ?
				GKSysLogError("LCD refresh failure");
			}

			if( interval < one ) {
				one -= interval;
				std::this_thread::sleep_for(one);
			}
		}

		device.getLCDPluginsManager()->unlockPlugin();
		device.getLCDPluginsManager()->jumpToNextPlugin();

		const uint64_t endscreen = toEnumType(LCDScreenPlugin::GK_LCD_ENDSCREEN);
		/* make sure endscreen plugin is loaded before using it */
		if( device.getLCDPluginsManager()->findOneLCDScreenPlugin( endscreen ) ) {
			const PixelsData & LCDBuffer = device.getLCDPluginsManager()->getNextLCDScreenBuffer("", endscreen);
			int ret = this->performUSBDeviceLCDScreenInterruptTransfer(
				device,
				LCDBuffer.data(),
				LCDBuffer.size(),
				1000
			);
			if(ret != 0) {
				GKSysLogError("endscreen LCD refresh failure");
			}
		}
		else {
			GKSysLogWarning("endscreen LCD plugin not loaded");
		}

		GKLog3(trace, devID, " exiting LCD screen thread for ", device.getFullName())
	} /* try */
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
	}
	catch (const GLogiKExcept & e) {
		GKSysLogError(e.what());
	}
	catch( const std::exception & e ) {
		GKSysLogError("uncaught std::exception : ", e.what());
	}
}

void KeyboardDriver::listenLoop(const std::string & devID)
{
	GK_LOG_FUNC

	try {
		USBDevice & device = _initializedDevices.at(devID);
		device._keysThreadID = std::this_thread::get_id();

		GKLog3(trace, devID, " spawned listening thread for ", device.getFullName())

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
								this->enterMacroRecordMode(device);

								/* don't need to update leds status if the mask is already 0 */
								if(device._MxKeysLedsMask != 0) {
									/* disabling macro record mode */
									if(this->updateDeviceMxKeysLedsMask(device, true))
										this->setDeviceMxKeysLeds(device);
								}
							}
							else { /* check to run macro */
								if( this->checkMacroKey(device) ) {
#if GKDBUS
									try {
										_pDBus->initializeBroadcastSignal(
											NSGKDBus::BusConnection::GKDBUS_SYSTEM,
											GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
											GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
											"deviceGKeyEvent"
										);

										_pDBus->appendStringToBroadcastSignal(devID);
										_pDBus->appendGKeysIDToBroadcastSignal(device._GKeyID);

										_pDBus->sendBroadcastSignal();

										LOG(trace)	<< device.getID() << " sent DBus signal: deviceGKeyEvent - "
													<< getGKeyName(device._GKeyID);
									}
									catch (const GKDBusMessageWrongBuild & e) {
										_pDBus->abandonBroadcastSignal();
										GKSysLogWarning(e.what());
									}
#endif
								}
							}
						}
					}

					if( this->checkDeviceCapability(device, Caps::GK_MEDIA_KEYS) ) {
						if( device.getLastKeysInterruptTransferLength() == device.getMediaKeysLength() ) {
							if( this->checkMediaKey(device) ) {
#if GKDBUS
								try {
									_pDBus->initializeBroadcastSignal(
										NSGKDBus::BusConnection::GKDBUS_SYSTEM,
										GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH,
										GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE,
										"deviceMediaEvent"
									);

									_pDBus->appendStringToBroadcastSignal(devID);
									_pDBus->appendStringToBroadcastSignal(device._mediaKey);

									_pDBus->sendBroadcastSignal();

									LOG(trace)	<< devID << " sent DBus signal: deviceMediaEvent - "
												<< device._mediaKey;
								}
								catch (const GKDBusMessageWrongBuild & e) {
									_pDBus->abandonBroadcastSignal();
									GKSysLogWarning(e.what());
								}
#endif
							}
						}
					}

					if( this->checkDeviceCapability(device, Caps::GK_LCD_SCREEN) ) {
						if( device.getLastKeysInterruptTransferLength() == device.getLCDKeysLength() ) {
							if( this->checkLCDKey(device) ) {
#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
								std::lock_guard<std::mutex> lock(device._LCDMutex);
								GKLog3(trace, devID, " LCD key pressed : ", device._LCDKey)
#endif
							}
						}
					}
					break;
				default:
					break;
			}
		}

		GKLog3(trace, devID, " exiting listening thread for ", device.getFullName())
	} /* try */
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
	}
	catch (const std::system_error& e) {
		GKSysLogError("error while spawning LCDScreen loop thread");
	}
	catch( const std::exception & e ) {
		GKSysLogError("uncaught std::exception : ", e.what());
	}
}

const bool KeyboardDriver::getDeviceThreadsStatus(const std::string & devID) const
{
	GK_LOG_FUNC

	try {
		return _initializedDevices.at(devID).getThreadsStatus();
	} /* try */
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
	}

	return false;
}

void KeyboardDriver::resetDeviceState(USBDevice & device)
{
	GK_LOG_FUNC

	GKLog3(trace, device.getID(), " resetting state of device : ", device.getFullName())

	if( this->checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
		GKLog2(trace, device.getID(), " resetting device MxKeys leds status")

		/* exit MacroRecordMode if necessary */
		device._exitMacroRecordMode = true;
		device._MxKeysLedsMask = 0;
		this->setDeviceMxKeysLeds(device);
	}

	if( this->checkDeviceCapability(device, Caps::GK_BACKLIGHT_COLOR) ) {
		GKLog2(trace, device.getID(), " resetting device backlight color")

		this->setDeviceBacklightColor(device);
	}

	if( this->checkDeviceCapability(device, Caps::GK_LCD_SCREEN) ) {
		GKLog2(trace, device.getID(), " resetting device LCD plugins mask")

		this->setDeviceLCDPluginsMask(device);
	}
}

void KeyboardDriver::resetDeviceState(const USBDeviceID & det)
{
	GK_LOG_FUNC

	try {
		USBDevice & device = _initializedDevices.at(det.getID());
		this->resetDeviceState(device);
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, det.getID());
	}
}

void KeyboardDriver::setDeviceLCDPluginsMask(USBDevice & device, uint64_t mask)
{
	GK_LOG_FUNC

	if( mask == 0 ) {
		/* default enabled plugins */
		mask |= toEnumType(LCDScreenPlugin::GK_LCD_SPLASHSCREEN);
		mask |= toEnumType(LCDScreenPlugin::GK_LCD_SYSTEM_MONITOR);
	}

	GKLog3(trace, device.getID(), " setting device LCD plugins mask to : ", mask)

	{
		yield_for(std::chrono::microseconds(100));
		std::lock_guard<std::mutex> lock(device._LCDMutex);
		device._LCDPluginsMask1 = mask;
	}

	/* Current active plugin may be locked */
	device.getLCDPluginsManager()->unlockPlugin();

	/* jump if current active plugin is not in the new mask */
	if( ! (device.getLCDPluginsManager()->getCurrentPluginID() & mask) ) {
		device.getLCDPluginsManager()->jumpToNextPlugin();
	}
}

void KeyboardDriver::joinDeviceThreads(USBDevice & device)
{
	GK_LOG_FUNC

	device.stopThreads();

	bool found = false;
	std::thread::id thread_id;

	auto find_thread = [&thread_id, &found] (auto & item) -> const bool {
		if( thread_id == item.get_id() ) {
			found = true;
			GKLog(trace, "thread found !")
			item.join();
			return true;
		}
		return false;
	};

	found = false; thread_id = device._keysThreadID;

	GKLog2(trace, device.getID(), " waiting for listening thread")
	{
		std::lock_guard<std::mutex> lock(_threadsMutex);
		_threads.erase(
			std::remove_if(_threads.begin(), _threads.end(), find_thread),
			_threads.end()
		);
	}

	if(! found) {
		GKSysLogWarning("listening thread not found !");
	}

	/* Caps::GK_LCD_SCREEN */
	if( this->checkDeviceCapability(device, Caps::GK_LCD_SCREEN) ) {
		found = false; thread_id = device._LCDThreadID;

		GKLog2(trace, device.getID(), " waiting for LCD screen thread")
		{
			std::lock_guard<std::mutex> lock(_threadsMutex);
			_threads.erase(
				std::remove_if(_threads.begin(), _threads.end(), find_thread),
				_threads.end()
			);
		}

		if(! found) {
			GKSysLogWarning("LCD screen thread not found !");
		}
	}
}

/* called when setting active user's configuration */
void KeyboardDriver::setDeviceActiveConfiguration(
	const std::string & devID,
	const uint8_t r,
	const uint8_t g,
	const uint8_t b,
	const uint64_t LCDPluginsMask1)
{
	GK_LOG_FUNC

	try {
		USBDevice & device = _initializedDevices.at(devID);

		if( this->checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
			/* exit MacroRecordMode if necessary */
			device._exitMacroRecordMode = true;
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
		GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
	}
}

const LCDPluginsPropertiesArray_type &
	KeyboardDriver::getDeviceLCDPluginsProperties(const std::string & devID) const
{
	GK_LOG_FUNC

	try {
		const USBDevice & device = _initializedDevices.at(devID);
		return device.getLCDPluginsManager()->getLCDPluginsProperties();
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
	}

	return LCDScreenPluginsManager::_LCDPluginsPropertiesEmptyArray;
}

/* throws GLogiKExcept on any failure */
void KeyboardDriver::openDevice(const USBDeviceID & det)
{
	GK_LOG_FUNC

	GKLog3(trace, det.getID(), " initializing device : ", det.getFullName())

	/* sanity check */
	if(_initializedDevices.count(det.getID()) > 0) {
		std::ostringstream err("device ID already used in started container : ", std::ios_base::app);
		err << det.getID();
		throw GLogiKExcept(err.str());
	}

	USBDevice device(det);

	this->openUSBDevice(device); /* throws on any failure */
	/* libusb device opened */

	try {
		this->sendUSBDeviceInitialization(device);

		// FIXME
		//if( this->checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
		//}

		if( this->checkDeviceCapability(device, Caps::GK_LCD_SCREEN) ) {
			try {
				device.setLCDPluginsManager( new LCDScreenPluginsManager(device.getProduct()) );
			}
			catch (const std::bad_alloc& e) { /* handle new() failure */
				throw GLogiKBadAlloc("LCD Plugins manager allocation failure");
			}
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
		this->closeUSBDevice(device);
		throw;
	}
}

void KeyboardDriver::closeDevice(
	const USBDeviceID & det,
	const bool skipUSBRequests) noexcept
{
	GK_LOG_FUNC

	const std::string & devID = det.getID();

	GKLog3(trace, devID, " closing device : ", det.getFullName())

	try {
		USBDevice & device = _initializedDevices.at(devID);

		if(skipUSBRequests)
			device.skipUSBRequests();

		this->joinDeviceThreads(device);
		this->resetDeviceState(device);
		this->closeUSBDevice(device);

		_initializedDevices.erase(devID);
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
	}
}

} // namespace GLogiK

