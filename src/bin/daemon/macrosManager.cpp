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

#include <stdexcept>
#include <iostream>

#include <config.h>

#include "lib/utils/utils.h"

#include "macrosManager.h"

namespace GLogiK
{

MacrosManager::MacrosManager(const char* virtual_keyboard_name)
	: buffer_("", std::ios_base::app), currentActiveProfile_(MemoryBank::MACROS_M0),
	virtual_keyboard(virtual_keyboard_name)
{
}

MacrosManager::~MacrosManager()
{
}

/* returns true if a macro is defined for this key on the current profile */
const bool MacrosManager::macroDefined(const std::string & macro_key_name) {
	const macro_t & macro = this->macros_profiles_[this->currentActiveProfile_].at(macro_key_name);
	return (macro.size() > 0);
}

/* run a macro on the virtual keyboard */
void MacrosManager::runMacro(const std::string & macro_key_name) {
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

/* set a macro on current profile, can be used by keyboard driver after MacroRecordMode */
void MacrosManager::setMacro(const std::string & macro_key_name, macro_t & macro) {
	try {
#if DEBUGGING_ON
		LOG(INFO) << "Macros Profile: " << to_uint(this->currentActiveProfile_)
			<< " - Macro Key: " << macro_key_name << " - setting macro";
#endif
		this->macros_profiles_[this->currentActiveProfile_].at(macro_key_name) = macro;
	}
	catch (const std::out_of_range& oor) {
		GKSysLog(LOG_WARNING, WARNING, "macro profile wrong map key : macro not recorded");
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

/*
 * declare and initialize a macro key
 * could be used by any keyboard driver on device initialization
 */
void MacrosManager::initializeMacroKey(const char* name) {
	for(auto & profile : this->macros_profiles_) {
		macro_t macro;
		profile.second.insert( std::pair<const std::string, macro_t>(name, macro));
	}
}

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

/* clear all macros profiles */
void MacrosManager::clearMacroProfiles(void) {
	this->setCurrentActiveProfile(MemoryBank::MACROS_M0);
	for(auto & profile_pair : this->macros_profiles_) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "clearing MemoryBank: " << to_uint(profile_pair.first);
#endif
		profile_pair.second.clear();
	}
}

} // namespace GLogiK

