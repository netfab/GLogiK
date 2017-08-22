/*
 *
 *	This file is part of GLogiK project.
 *	GLogiKd, daemon to handle special features on gaming keyboards
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

#include <iostream>

#include <stdexcept>

#include "include/log.h"
#include <syslog.h>

#include "globals.h"
#include "macros_manager.h"

namespace GLogiKd
{

MacrosManager::MacrosManager(const char* virtual_keyboard_name)
	: buffer_("", std::ios_base::app), currentActiveProfile_(MemoryBank::MACROS_M0),
	virtual_keyboard(virtual_keyboard_name)
{
}

MacrosManager::~MacrosManager()
{
}

void MacrosManager::setMacro(const std::string &macro_key_name, std::vector<KeyEvent> & macro) {
	try {
		this->macros_profiles_[this->currentActiveProfile_].at(macro_key_name) = macro;
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("warning : macro profile wrong map key. macro not recorded.");
		LOG(WARNING) << this->buffer_.str();
		syslog(LOG_WARNING, this->buffer_.str().c_str());
	}
}

void MacrosManager::logProfiles(void) {
	for(const auto & profile : this->macros_profiles_) {
		LOG(DEBUG2) << "MemoryBank: " << to_uint(profile.first);
		for(const auto & macro_key : profile.second) {
			LOG(DEBUG3) << "MacroKey: " << macro_key.first << " MacroLength: " << macro_key.second.size();
		}
	}
}

void MacrosManager::initializeMacroKey(const char* name) {
	for(auto & profile : this->macros_profiles_) {
		std::vector<KeyEvent> macro;
		profile.second.insert( std::pair<const std::string, std::vector<KeyEvent>>(name, macro));
	}
}

void MacrosManager::setCurrentActiveProfile(MemoryBank bank)
{
	LOG(DEBUG) << "setting current active macros profile : " << to_uint(bank);
	this->currentActiveProfile_ = bank;
	// TODO reload conf
}

} // namespace GLogiKd

