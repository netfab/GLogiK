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
	const macro_t & macro_array
)	{
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

} // namespace GLogiK

