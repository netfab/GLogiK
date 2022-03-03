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

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <sstream>
#include <iostream>

#include "lib/utils/utils.hpp"

#include "macrosManager.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

MacrosManager::MacrosManager(
	const char* virtualKeyboardName,
	const uint8_t numBanks,
	const std::vector<std::string> & keysNames)
		:	_virtualKeyboard(virtualKeyboardName),
			_currentBankID(BankID::BANK_M0)
{
	this->initMacrosBanks(numBanks, keysNames);
}

MacrosManager::~MacrosManager()
{
}

/* returns true if a macro is defined for this key on the current memory bank */
const bool MacrosManager::macroDefined(const std::string & keyName)
{
	GK_LOG_FUNC

	try {
		const macro_type & macro = _macrosBanks[_currentBankID].at(keyName);
		return (macro.size() > 0);
	}
	catch (const std::out_of_range& oor) {
		GKSysLogWarning("macro key container not found");
	}

	return false;
}

/* run a macro on the virtual keyboard */
void MacrosManager::runMacro(const std::string & keyName)
{
	GK_LOG_FUNC

	try {
		const macro_type & macro = _macrosBanks[_currentBankID].at(keyName);
		if(macro.size() == 0) {
#if DEBUGGING_ON
			if(GKLogging::GKDebug) {
				LOG(trace)	<< "Memory Bank: " << _currentBankID
							<< " - Macro Key: " << keyName
							<< " - no macro recorded";
			}
#endif
			return;
		}

#if DEBUGGING_ON
		if(GKLogging::GKDebug) {
			LOG(trace)	<< "Memory Bank: " << _currentBankID
						<< " - Macro Key: " << keyName
						<< " - running macro";
		}
#endif
		for( const auto &key : macro ) {
			_virtualKeyboard.sendKeyEvent(key);
		}
	}
	catch (const std::out_of_range& oor) {
		GKSysLogWarning("macro key container not found");
	}
}

void MacrosManager::setCurrentMacrosBankID(BankID bankID)
{
	GK_LOG_FUNC

	GKLog2(trace, "setting current bank ID : ", bankID)

	_currentBankID = bankID;
}

const BankID MacrosManager::getCurrentMacrosBankID(void) const
{
	return _currentBankID;
}

void MacrosManager::setMacro(
	const std::string & keyName,
	macro_type & macro)
{
	GK_LOG_FUNC

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
			GKSysLogWarning("macro size greater than MACRO_T_MAX_SIZE, fixing it");
			pressedEvents.clear();
			releasedEvents.clear();
			this->fillInVectors(macro, pressedEvents, releasedEvents);
			this->fixMacroSize(pressedEvents, releasedEvents, macro);
		}
	}

	MacrosBanks::setMacro(_currentBankID, keyName, macro);
}

void MacrosManager::resetMacrosBanks(void)
{
	GK_LOG_FUNC

	this->setCurrentMacrosBankID(BankID::BANK_M0);
	for(auto & idBankPair : _macrosBanks) {
		GKLog2(trace, "clearing all macros for Memory Bank: ", idBankPair.first)
		this->resetMacrosBank(idBankPair.first);
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
	GK_LOG_FUNC

	/* fix missing release events */
	for(const auto & pressed : pressedEvents) {
		bool found = false;
		for(auto it = releasedEvents.begin(); it != releasedEvents.end(); ++it) {
			if(pressed.key.code == (*it).key.code) {
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
			buffer << "missing release event for index " << toUInt(pressed.index) << " - adding event";
			GKSysLogWarning(buffer.str());
			KeyEvent e = pressed.key;
			e.event = EventValue::EVENT_KEY_RELEASE;
			e.interval = 1;
			macro.push_back(e);
		}
	}

	/* remove redundant release events in reverse order */
	if( ! releasedEvents.empty() ) {
		GKLog2(trace, "some redundant release events were found : ", releasedEvents.size())

		std::reverse(releasedEvents.begin(), releasedEvents.end());

		for(const auto & redundant : releasedEvents) {
			GKLog2(trace, "erasing redundant release event at index : ", toUInt(redundant.index))

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
	GK_LOG_FUNC

	GKLog4(trace, "pressed : ", pressedEvents.size(), "released : ", releasedEvents.size())

	/* sanity check */
	if(pressedEvents.size() != releasedEvents.size()) {
		GKSysLogWarning("pressed and released events disparity :");
		std::ostringstream buffer(std::ios_base::ate);
		buffer << "pressed: " << pressedEvents.size() << " - released: " << releasedEvents.size();
		GKSysLogWarning(buffer.str());
	}

	std::vector<unsigned int> indexes;

	for(const auto & pressed : pressedEvents) {
		for(auto it = releasedEvents.begin(); it != releasedEvents.end(); ++it) {
			if(pressed.key.code == (*it).key.code) {
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

	GKLog2(trace, "indexes size : ", indexes.size())

	while( (indexes.size() > 1) and (macro.size() >= MACRO_T_MAX_SIZE) ) {
		auto & index = indexes.back();

		GKLog2(trace, "erasing index : ", index)

		auto it = std::next(macro.begin(), index);
		macro.erase(it);
		indexes.pop_back();

		index = indexes.back();

		GKLog2(trace, "erasing index : ", index)

		it = std::next(macro.begin(), index);
		macro.erase(it);
		indexes.pop_back();
	}

	/* sanity check */
	if( macro.size() >= MACRO_T_MAX_SIZE ) {
		GKLog2(trace, "macro size : ", macro.size())

		GKSysLogWarning("macro still greater than MACRO_T_MAX_SIZE, force resize it");
		macro.resize(MACRO_T_MAX_SIZE - 1);
	}

}

} // namespace GLogiK

