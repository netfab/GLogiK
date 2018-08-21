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
#include <sstream>
#include <iostream>

#include "lib/utils/utils.h"

#include "macrosManager.h"

namespace GLogiK
{

using namespace NSGKUtils;

MacrosManager::MacrosManager(
	const char* virtualKeyboardName,
	const std::vector<std::string> & keysNames)
		:	_currentMacrosBank(MacrosBank::BANK_M0),
			_virtualKeyboard(virtualKeyboardName)
{
#if DEBUGGING_ON
	LOG(DEBUG) << "initializing " << keysNames.size() << " macro keys";
#endif

	this->initMacrosBanks(keysNames);
}

MacrosManager::~MacrosManager()
{
}

/* returns true if a macro is defined for this key on the current memory bank */
const bool MacrosManager::macroDefined(const std::string & keyName) {
	try {
		const macro_type & macro = _macrosBanks[_currentMacrosBank].at(keyName);
		return (macro.size() > 0);
	}
	catch (const std::out_of_range& oor) {
		GKSysLog(LOG_WARNING, WARNING, "macro key container not found");
	}

	return false;
}

/* run a macro on the virtual keyboard */
void MacrosManager::runMacro(const std::string & keyName) {
	try {
		const macro_type & macro = _macrosBanks[_currentMacrosBank].at(keyName);
		if(macro.size() == 0) {
#if DEBUGGING_ON
			LOG(DEBUG) << "Memory Bank: " << _currentMacrosBank
				<< " - Macro Key: " << keyName << " - no macro recorded";
#endif
			return;
		}

#if DEBUGGING_ON
		LOG(INFO) << "Memory Bank: " << _currentMacrosBank
			<< " - Macro Key: " << keyName << " - running macro";
#endif
		for( const auto &key : macro ) {
			_virtualKeyboard.sendKeyEvent(key);
		}
	}
	catch (const std::out_of_range& oor) {
		GKSysLog(LOG_WARNING, WARNING, "macro key container not found");
	}
}

void MacrosManager::setCurrentMacrosBank(MacrosBank bank) {
#if DEBUGGING_ON
	LOG(DEBUG) << "setting current memory bank : " << bank;
#endif
	_currentMacrosBank = bank;
}

const MacrosBank MacrosManager::getCurrentMacrosBank(void) const {
	return _currentMacrosBank;
}

void MacrosManager::setMacro(
	const std::string & keyName,
	macro_type & macro)
{
	{
		std::vector<MacroEvent> pressedEvents;
		std::vector<MacroEvent> releasedEvents;

		this->fillInVectors(macro, pressedEvents, releasedEvents);
		this->fixMacroReleaseEvents(pressedEvents, releasedEvents, macro);

		//// debug code
		//KeyEvent e1(KEY_UNKNOWN, EventValue::EVENT_KEY_PRESS, 1);
		//KeyEvent e2(KEY_UNKNOWN, EventValue::EVENT_KEY_RELEASE, 1);
		//for(unsigned int i = 0; i < 12; ++i) {
		//	macro.push_back(e1);
		//	macro.push_back(e2);
		//}

		if(macro.size() >= MACRO_T_MAX_SIZE) {
			GKSysLog(LOG_WARNING, WARNING, "macro size greater than MACRO_T_MAX_SIZE, fixing it");
			pressedEvents.clear();
			releasedEvents.clear();
			this->fillInVectors(macro, pressedEvents, releasedEvents);
			this->fixMacroSize(pressedEvents, releasedEvents, macro);
		}
	}

	MacrosBanks::setMacro(_currentMacrosBank, keyName, macro);
}

void MacrosManager::resetMacrosBanks(void) {
	this->setCurrentMacrosBank(MacrosBank::BANK_M0);
	for(auto & bankKeyPair : _macrosBanks) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "clearing all macros for Memory Bank: " << bankKeyPair.first;
#endif
		for(auto & keyMacroPair : bankKeyPair.second) {
			keyMacroPair.second.clear();
		}
	}
}

void MacrosManager::fillInVectors(
	const macro_type & macro,
	std::vector<MacroEvent> & pressedEvents,
	std::vector<MacroEvent> & releasedEvents)
{
	for(unsigned int i = 0; i != macro.size(); ++i) {
		const auto & keyEvent = macro[i];
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
	macro_type & macro)
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
			std::ostringstream buffer(std::ios_base::ate);
			buffer << "missing release event for index " << to_uint(pressed.index) << " - adding event";
			GKSysLog(LOG_WARNING, WARNING, buffer.str());
			KeyEvent e = pressed.key;
			e.event = EventValue::EVENT_KEY_RELEASE;
			e.interval = 1;
			macro.push_back(e);
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
			auto it = std::next(macro.begin(), redundant.index);
			macro.erase(it);
		}
	}
}

void MacrosManager::fixMacroSize(
	const std::vector<MacroEvent> & pressedEvents,
	std::vector<MacroEvent> & releasedEvents,
	macro_type & macro)
{
#if DEBUGGING_ON
	LOG(DEBUG1) << "pressed events : " << pressedEvents.size();
	LOG(DEBUG1) << "released events : " << releasedEvents.size();
#endif
	/* sanity check */
	if(pressedEvents.size() != releasedEvents.size()) {
		GKSysLog(LOG_WARNING, WARNING, "pressed and released events disparity :");
		std::ostringstream buffer(std::ios_base::ate);
		buffer << "pressed: " << pressedEvents.size() << " - released: " << releasedEvents.size();
		GKSysLog(LOG_WARNING, WARNING, buffer.str());
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

	while( (indexes.size() > 1) and (macro.size() >= MACRO_T_MAX_SIZE) ) {
		auto & index = indexes.back();
#if DEBUGGING_ON
		LOG(DEBUG3) << "erasing index : " << index;
#endif
		auto it = std::next(macro.begin(), index);
		macro.erase(it);
		indexes.pop_back();

		index = indexes.back();
#if DEBUGGING_ON
		LOG(DEBUG3) << "erasing index : " << index;
#endif
		it = std::next(macro.begin(), index);
		macro.erase(it);
		indexes.pop_back();
	}

	/* sanity check */
	if( macro.size() >= MACRO_T_MAX_SIZE ) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "macro size : " << macro.size();
#endif
		GKSysLog(LOG_WARNING, WARNING, "macro still greater than MACRO_T_MAX_SIZE, force resize it");
		macro.resize(MACRO_T_MAX_SIZE - 1);
	}

}

} // namespace GLogiK

