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

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <iostream>

#include "lib/utils/utils.h"

#include "macrosManager.h"

namespace GLogiK
{

using namespace NSGKUtils;

MacrosManager::MacrosManager(
	const char* virtual_keyboard_name,
	const std::vector<std::string> & keys_names)
		:	buffer_("", std::ios_base::app),
			currentActiveProfile_(MemoryBank::BANK_M0),
			virtual_keyboard(virtual_keyboard_name)
{
#if DEBUGGING_ON
	LOG(DEBUG) << "initializing " << keys_names.size() << " macro keys";
#endif

	this->initMacrosProfiles(keys_names);
}

MacrosManager::~MacrosManager()
{
}

/* returns true if a macro is defined for this key on the current profile */
const bool MacrosManager::macroDefined(const std::string & macro_key_name) {
	try {
		const macro_t & macro = this->macros_profiles_[this->currentActiveProfile_].at(macro_key_name);
		return (macro.size() > 0);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog(LOG_WARNING, WARNING, "macro key container not found");
	}

	return false;
}

/* run a macro on the virtual keyboard */
void MacrosManager::runMacro(const std::string & macro_key_name) {
	try {
		const macro_t & macro = this->macros_profiles_[this->currentActiveProfile_].at(macro_key_name);
		if(macro.size() == 0) {
#if DEBUGGING_ON
			LOG(DEBUG) << "Macros Profile: " << to_uint(this->currentActiveProfile_)
				<< " - Macro Key: " << macro_key_name << " - no macro recorded";
#endif
			return;
		}

#if DEBUGGING_ON
		LOG(INFO) << "Macros Profile: " << to_uint(this->currentActiveProfile_)
			<< " - Macro Key: " << macro_key_name << " - running macro";
#endif
		for( const auto &key : macro ) {
			this->virtual_keyboard.sendKeyEvent(key);
		}
	}
	catch (const std::out_of_range& oor) {
		GKSysLog(LOG_WARNING, WARNING, "macro key container not found");
	}
}

/*
void MacrosManager::logProfiles(void) {
	for(const auto & profile : this->macros_profiles_) {
		LOG(DEBUG2) << "MemoryBank: " << to_uint(profile.first);
		for(const auto & macro_key : profile.second) {
			LOG(DEBUG3) << "MacroKey: " << macro_key.first << " MacroLength: " << macro_key.second.size();
		}
	}
}
*/

/* set the current active macros profile */
void MacrosManager::setCurrentActiveProfile(MemoryBank bank) {
#if DEBUGGING_ON
	LOG(DEBUG) << "setting current active macros profile : " << to_uint(bank);
#endif
	this->currentActiveProfile_ = bank;
}

const MemoryBank MacrosManager::getCurrentActiveProfile(void) const {
	return this->currentActiveProfile_;
}

void MacrosManager::setMacro(
	const std::string & keyName,
	macro_t & macro_array)
{
	{
		std::vector<MacroEvent> pressedEvents;
		std::vector<MacroEvent> releasedEvents;

		this->fillInVectors(macro_array, pressedEvents, releasedEvents);
		this->fixMacroReleaseEvents(pressedEvents, releasedEvents, macro_array);

		//// debug code
		//KeyEvent e1(KEY_UNKNOWN, EventValue::EVENT_KEY_PRESS, 1);
		//KeyEvent e2(KEY_UNKNOWN, EventValue::EVENT_KEY_RELEASE, 1);
		//for(unsigned int i = 0; i < 12; ++i) {
		//	macro_array.push_back(e1);
		//	macro_array.push_back(e2);
		//}

		if(macro_array.size() >= MACRO_T_MAX_SIZE) {
			GKSysLog(LOG_WARNING, WARNING, "macro size greater than MACRO_T_MAX_SIZE, fixing it");
			pressedEvents.clear();
			releasedEvents.clear();
			this->fillInVectors(macro_array, pressedEvents, releasedEvents);
			this->fixMacroSize(pressedEvents, releasedEvents, macro_array);
		}
	}

	MacrosBanks::setMacro(this->currentActiveProfile_, keyName, macro_array);
}

/* clear all macros profiles */
void MacrosManager::clearMacroProfiles(void) {
	this->setCurrentActiveProfile(MemoryBank::BANK_M0);
	for(auto & profile_pair : this->macros_profiles_) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "clearing macros for MemoryBank: " << to_uint(profile_pair.first);
#endif
		for(auto & macro_key_pair : profile_pair.second) {
			macro_key_pair.second.clear();
		}
	}
}

void MacrosManager::fillInVectors(
	const macro_t & macro_array,
	std::vector<MacroEvent> & pressedEvents,
	std::vector<MacroEvent> & releasedEvents)
{
	for(unsigned int i = 0; i != macro_array.size(); ++i) {
		const auto & keyEvent = macro_array[i];
		MacroEvent e(keyEvent, i);

		if( keyEvent.event == EventValue::EVENT_KEY_PRESS )
			pressedEvents.push_back(e);
		else if( keyEvent.event == EventValue::EVENT_KEY_RELEASE )
			releasedEvents.push_back(e);
	}
}

void MacrosManager::fixMacroReleaseEvents(
	const std::vector<MacroEvent> & pressedEvents,
	std::vector<MacroEvent> & releasedEvents,
	macro_t & macro_array)
{
	/* fix missing release events */
	for(const auto & pressed : pressedEvents) {
		bool found = false;
		for(auto it = releasedEvents.begin(); it != releasedEvents.end(); ++it) {
			if(pressed.key.event_code == (*it).key.event_code) {
				if(pressed.index < (*it).index) {
					/* KEY_RELEASE event found for current KEY_PRESS event */
					releasedEvents.erase(it);
					found = true;
					break; /* jumping to next press event */
				}
			}
		}
		if( ! found ) {
			this->buffer_.str("missing release event for index ");
			this->buffer_ << to_uint(pressed.index);
			this->buffer_ << " - adding event";
			GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
			KeyEvent e = pressed.key;
			e.event = EventValue::EVENT_KEY_RELEASE;
			e.interval = 1;
			macro_array.push_back(e);
		}
	}

	/* remove redundant release events in reverse order */
	if( ! releasedEvents.empty() ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "some redundant release events were found : " << releasedEvents.size();
#endif
		std::reverse(releasedEvents.begin(), releasedEvents.end());

		for(const auto & redundant : releasedEvents) {
#if DEBUGGING_ON
			LOG(DEBUG3) << "erasing redundant release event at index : " << to_uint(redundant.index);
#endif
			auto it = std::next(macro_array.begin(), redundant.index);
			macro_array.erase(it);
		}
	}
}

void MacrosManager::fixMacroSize(
	const std::vector<MacroEvent> & pressedEvents,
	std::vector<MacroEvent> & releasedEvents,
	macro_t & macro_array)
{
#if DEBUGGING_ON
	LOG(DEBUG1) << "pressed events : " << pressedEvents.size();
	LOG(DEBUG1) << "released events : " << releasedEvents.size();
#endif
	/* sanity check */
	if(pressedEvents.size() != releasedEvents.size()) {
		GKSysLog(LOG_WARNING, WARNING, "pressed and released events disparity :");
		this->buffer_.str("pressed: ");
		this->buffer_ << pressedEvents.size();
		this->buffer_.str(" - released: ");
		this->buffer_ << releasedEvents.size();
		GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
	}

	std::vector<unsigned int> indexes;

	for(const auto & pressed : pressedEvents) {
		for(auto it = releasedEvents.begin(); it != releasedEvents.end(); ++it) {
			if(pressed.key.event_code == (*it).key.event_code) {
				if(pressed.index < (*it).index) {
					indexes.push_back( pressed.index );
					indexes.push_back( (*it).index );
					releasedEvents.erase(it);
					break;
				}
			}
		}
	}

	std::sort(indexes.begin(), indexes.end());

#if DEBUGGING_ON
	LOG(DEBUG2) << "indexes size : " << indexes.size();
#endif

	while( (indexes.size() > 1) and (macro_array.size() >= MACRO_T_MAX_SIZE) ) {
		auto & index = indexes.back();
#if DEBUGGING_ON
		LOG(DEBUG3) << "erasing index : " << index;
#endif
		auto it = std::next(macro_array.begin(), index);
		macro_array.erase(it);
		indexes.pop_back();

		index = indexes.back();
#if DEBUGGING_ON
		LOG(DEBUG3) << "erasing index : " << index;
#endif
		it = std::next(macro_array.begin(), index);
		macro_array.erase(it);
		indexes.pop_back();
	}

	/* sanity check */
	if( macro_array.size() >= MACRO_T_MAX_SIZE ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "macro size : " << macro_array.size();
#endif
		GKSysLog(LOG_WARNING, WARNING, "macro still greater than MACRO_T_MAX_SIZE, force resize it");
		macro_array.resize(MACRO_T_MAX_SIZE - 1);
	}

}

} // namespace GLogiK

