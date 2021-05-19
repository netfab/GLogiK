/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
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

#include "lib/utils/utils.hpp"

#include "macrosBanks.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

const macro_type MacrosBanks::emptyMacro = {};
const banksMap_type MacrosBanks::emptyMacrosBanks = {};

MacrosBanks::MacrosBanks() {
}

MacrosBanks::~MacrosBanks() {
}

void MacrosBanks::initMacrosBanks(const std::vector<std::string> & keysNames) {
	GK_LOG_FUNC

#if DEBUGGING_ON
	LOG(DEBUG2) << "initialize " << keysNames.size() << " macro keys";
#endif
	macro_type macro;
	for( const auto & name : keysNames ) {
		for(auto & idBankPair : _macrosBanks) {
			idBankPair.second.insert( std::pair<const std::string, macro_type>(name, macro));
		}
	}
}

const banksMap_type & MacrosBanks::getMacrosBanks(void) const
{
	return _macrosBanks;
}

void MacrosBanks::setMacrosBanks(const banksMap_type & macrosBanks)
{
	GK_LOG_FUNC

#if DEBUGGING_ON
	LOG(DEBUG3) << "setting macros banks";
#endif
	_macrosBanks = macrosBanks;
}

void MacrosBanks::clearMacro(
	const BankID bankID,
	const std::string & keyName)
{
	GK_LOG_FUNC

	try {
		LOG(INFO) << "macros bankID: M" << bankID
			<< " - Macro Key: " << keyName
			<< " - clearing macro";

		_macrosBanks[bankID].at(keyName).clear();
	}
	catch (const std::out_of_range& oor) {
		std::string warn("wrong map key : ");
		warn += keyName;
		GKSysLog(LOG_WARNING, WARNING, warn);
		throw GLogiKExcept("macro not cleared");
	}
}

void MacrosBanks::clearMacro(
	const uint8_t bankID,
	const std::string & keyName)
{
	if(bankID > BankID::BANK_M3)
		throw GLogiKExcept("wrong bankID value");

	const BankID id = static_cast<BankID>(bankID);

	this->clearMacro(id, keyName);
}

void MacrosBanks::setMacro(
	const BankID bankID,
	const std::string & keyName,
	const macro_type & macro)
{
	GK_LOG_FUNC

	try {
		LOG(INFO) << "macros bankID: M" << bankID
			<< " - Macro Key: " << keyName
			<< " - Macro Size: " << macro.size()
			<< " - setting macro";
		if( macro.size() >= MACRO_T_MAX_SIZE ) {
			LOG(WARNING) << "skipping macro - size >= MACRO_T_MAX_SIZE";
			throw GLogiKExcept("skipping macro");
		}

		_macrosBanks[bankID].at(keyName) = macro;
	}
	catch (const std::out_of_range& oor) {
		std::string warn("wrong map key : ");
		warn += keyName;
		GKSysLog(LOG_WARNING, WARNING, warn);
		throw GLogiKExcept("macro not updated");
	}
}

void MacrosBanks::setMacro(
	const uint8_t bankID,
	const std::string & keyName,
	const macro_type & macro)
{
	if(bankID > BankID::BANK_M3)
		throw GLogiKExcept("wrong bankID value");

	const BankID id = static_cast<BankID>(bankID);

	this->setMacro(id, keyName, macro);
}

const macro_type & MacrosBanks::getMacro(const uint8_t bankID, const std::string & keyName)
{
	GK_LOG_FUNC

	if(bankID > BankID::BANK_M3)
		throw GLogiKExcept("wrong bankID value");

	const BankID id = static_cast<BankID>(bankID);
	try {
		return _macrosBanks[id].at(keyName);
	}
	catch (const std::out_of_range& oor) {
		std::string warn("wrong map key : ");
		warn += keyName;
		GKSysLog(LOG_WARNING, WARNING, warn);
		throw GLogiKExcept("can't get macro");
	}

	return MacrosBanks::emptyMacro;
}

void MacrosBanks::resetMacrosBank(const uint8_t bankID)
{
	if(bankID > BankID::BANK_M3)
		throw GLogiKExcept("wrong bankID value");

	const BankID id = static_cast<BankID>(bankID);

	this->resetMacrosBank(id);
}

void MacrosBanks::resetMacrosBank(const BankID bankID)
{
	for(auto & keyMacroPair : _macrosBanks[bankID]) {
		keyMacroPair.second.clear();
	}
}

} // namespace GLogiK

