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
	const std::vector<std::string> & keys_names
) : buffer_("", std::ios_base::app),
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

		for(unsigned int i = 0; i != macro_array.size(); ++i) {
			const auto & keyEvent = macro_array[i];
			MacroEvent e(keyEvent, i);

			if( keyEvent.event == EventValue::EVENT_KEY_PRESS )
				pressedEvents.push_back(e);
			else if( keyEvent.event == EventValue::EVENT_KEY_RELEASE )
				releasedEvents.push_back(e);
		}

		this->fixMacroReleaseEvents(pressedEvents, releasedEvents, macro_array);
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

void MacrosManager::fixMacroReleaseEvents(
	const std::vector<MacroEvent> & pressedEvents,
	std::vector<MacroEvent> & releasedEvents,
	macro_t & macro_array
) {
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
			this->buffer_ << pressed.index;
			this->buffer_ << " - adding event";
			GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
			KeyEvent e = pressed.key;
			e.event = EventValue::EVENT_KEY_RELEASE;
			e.interval = 1;
			macro_array.push_back(e);
		}
	}

	/* remove redundant release events */
	if( ! releasedEvents.empty() ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "some redundant release events were found : " << releasedEvents.size();
#endif
		for(const auto & redundant : releasedEvents) {
#if DEBUGGING_ON
			LOG(DEBUG3) << "erasing redundant release event at index : " << redundant.index;
#endif
			auto it = std::next(macro_array.begin(), redundant.index);
			macro_array.erase(it);
		}
	}

}

} // namespace GLogiK

