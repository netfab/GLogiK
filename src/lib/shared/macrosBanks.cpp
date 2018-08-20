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
#include <utility>

#include "lib/utils/utils.h"

#include "macrosBanks.h"

namespace GLogiK
{

using namespace NSGKUtils;

const macro_type MacrosBanks::emptyMacro = {};
const macros_map_type MacrosBanks::emptyMacrosBanks = {};

MacrosBanks::MacrosBanks() {
}

MacrosBanks::~MacrosBanks() {
}

void MacrosBanks::initMacrosProfiles(const std::vector<std::string> & keysNames) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "initialize " << keysNames.size() << " macro keys";
#endif
	macro_type macro;
	for( const auto & name : keysNames ) {
		for(auto & bank : _macrosBanks) {
			bank.second.insert( std::pair<const std::string, macro_type>(name, macro));
		}
	}
}

const macros_map_type & MacrosBanks::getMacrosProfiles(void) const
{
	return _macrosBanks;
}

void MacrosBanks::setMacrosProfiles(const macros_map_type & macrosBanks)
{
#if DEBUGGING_ON
	LOG(DEBUG3) << "setting macros banks";
#endif
	_macrosBanks = macrosBanks;
}

void MacrosBanks::clearMacro(
	const MacrosBank & bank,
	const std::string & keyName)
{
	try {
		LOG(INFO) << "macros bank: M" << bank
			<< " - Macro Key: " << keyName
			<< " - clearing macro";

		_macrosBanks[bank].at(keyName).clear();
	}
	catch (const std::out_of_range& oor) {
		std::string warn("wrong map key : ");
		warn += keyName;
		GKSysLog(LOG_WARNING, WARNING, warn);
		throw GLogiKExcept("macro not cleared");
	}
}

void MacrosBanks::clearMacro(
	const uint8_t bank,
	const std::string & keyName)
{
	if(bank > MacrosBank::BANK_M3)
		throw GLogiKExcept("wrong bank value");

	const MacrosBank current_bank = static_cast<MacrosBank>(bank);

	this->clearMacro(current_bank, keyName);
}

void MacrosBanks::setMacro(
	const MacrosBank & bank,
	const std::string & keyName,
	const macro_type & macroArray)
{
	try {
		LOG(INFO) << "macros bank: M" << bank
			<< " - Macro Key: " << keyName
			<< " - Macro Size: " << macroArray.size()
			<< " - setting macro";
		if( macroArray.size() >= MACRO_T_MAX_SIZE ) {
			LOG(WARNING) << "skipping macro - size >= MACRO_T_MAX_SIZE";
			throw GLogiKExcept("skipping macro");
		}

		_macrosBanks[bank].at(keyName) = macroArray;
	}
	catch (const std::out_of_range& oor) {
		std::string warn("wrong map key : ");
		warn += keyName;
		GKSysLog(LOG_WARNING, WARNING, warn);
		throw GLogiKExcept("macro not updated");
	}
}

void MacrosBanks::setMacro(
	const uint8_t bank,
	const std::string & keyName,
	const macro_type & macroArray)
{
	if(bank > MacrosBank::BANK_M3)
		throw GLogiKExcept("wrong bank value");

	const MacrosBank current_bank = static_cast<MacrosBank>(bank);

	this->setMacro(current_bank, keyName, macroArray);
}

const macro_type & MacrosBanks::getMacro(const uint8_t bank, const std::string & keyName)
{
	if(bank > MacrosBank::BANK_M3)
		throw GLogiKExcept("wrong bank value");

	const MacrosBank current_bank = static_cast<MacrosBank>(bank);
	try {
		return _macrosBanks[current_bank].at(keyName);
	}
	catch (const std::out_of_range& oor) {
		std::string warn("wrong map key : ");
		warn += keyName;
		GKSysLog(LOG_WARNING, WARNING, warn);
		throw GLogiKExcept("can't get macro");
	}

	return MacrosBanks::emptyMacro;
}

} // namespace GLogiK

