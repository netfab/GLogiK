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
#include <utility>

#include "lib/utils/utils.hpp"

#include "glogik.hpp"
#include "macrosBanks.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

const std::string idToString(const MKeysID keyID)
{
	const std::string ret(std::to_string(static_cast<unsigned int>(keyID)));
	return ret;
}

const macro_type MacrosBanks::emptyMacro = {};
const banksMap_type MacrosBanks::emptyMacrosBanks = {};

MacrosBanks::MacrosBanks()
{
}

MacrosBanks::~MacrosBanks()
{
}

void MacrosBanks::initMacrosBanks(
	const uint8_t numBanks,
	const GKeysIDArray_type & keysID)
{
	GK_LOG_FUNC

	GKLog4(trace, "numBanks: ", toUInt(numBanks), "numKeys: ", keysID.size())

	for(unsigned int bankID = 0; bankID <= toUInt(numBanks); ++bankID) {
		try {
			const MKeysID id = this->getBankID(bankID);

			{
				/* XXX - c++17 structured bindings */
				typedef std::pair<banksMap_type::iterator, bool> bankInsRet;
				bankInsRet ret = _macrosBanks.insert( std::pair<const MKeysID, mBank_type>(id, {}) );
				if( ! ret.second )
					throw GLogiKExcept("bank insert failure");
			}

			unsigned short insertedKeys = 0;

			for(const auto & keyID : keysID) {
				/* XXX - c++17 structured bindings */
				typedef std::pair<mBank_type::iterator, bool> keyInsRet;

				auto insertStatus = [&insertedKeys] (const keyInsRet & r) -> void
				{
					if(r.second) {
						insertedKeys++;
						//GKLog2(trace, "inserted key: ", getGKeyName(r.first->first))
					}
					else {
						LOG(error) << "failed to insert key: " << getGKeyName(r.first->first);
					}
				};

				keyInsRet insKey = _macrosBanks[id].insert(
					std::pair<GKeysID, macro_type>(
						keyID, MacrosBanks::emptyMacro
					)
				);

				insertStatus(insKey);
			}

			GKLog4(trace, "bank id: ", bankID, "number of initialized G-keys: ", insertedKeys)
		}
		catch(const GLogiKExcept & e) {
			LOG(error) << "bank id: " << bankID << " - failed to initialize: " << e.what();
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

	GKLog(trace, "setting macros banks")

	_macrosBanks = macrosBanks;
}

void MacrosBanks::checkMacrosBanksKeys(void)
{
	for(auto bankIt = _macrosBanks.begin(); bankIt != _macrosBanks.end();) {
		try {
			// checking bankID
			this->getBankID(toEnumType(bankIt->first));
			//GKLog2(trace, "checked MKeyID: ", bankIt->first)
			++bankIt;
		}
		catch(const GLogiKExcept & e) {
			LOG(warning) << e.what()
				<< " - erasing whole bankID: "
				<< static_cast<unsigned int>(bankIt->first);
			bankIt = _macrosBanks.erase(bankIt);
		}
	}

	for(auto bankIt = _macrosBanks.begin(); bankIt != _macrosBanks.end(); bankIt++) {
		auto & mBank = bankIt->second;
		for(auto it = mBank.begin(); it != mBank.end();) {
			try {
				const uint8_t id = toEnumType(it->first);
				if(id > GLogiK::GKeysID::GKEY_G18)
					throw GLogiKExcept("wrong GKeysID value");

				//GKLog2(trace, "checked GKeyID: ", getGKeyName(it->first))
				++it;
			}
			catch(const GLogiKExcept & e) {
				LOG(warning) << e.what()
					<< " - erasing GKeyID: "
					<< static_cast<unsigned int>(it->first);
				it = mBank.erase(it);
			}
		}
	}
}

void MacrosBanks::clearMacro(
	const MKeysID bankID,
	const GKeysID keyID)
{
	GK_LOG_FUNC

	try {
		mBank_type & bank = _macrosBanks.at(bankID);

		try {
			LOG(info) << "macros bankID: M" << bankID
				<< " - Macro Key: " << getGKeyName(keyID)
				<< " - clearing macro";

			bank.at(keyID).clear();
		}
		catch(const std::out_of_range& oor) {
			this->throwWarn("wrong map key: ", getGKeyName(keyID), "can't set macro");
		}
	}
	catch (const std::out_of_range& oor) {
		this->throwWarn("wrong bankID: ", idToString(bankID), "can't clear macro");
	}
}

void MacrosBanks::setMacro(
	const MKeysID bankID,
	const GKeysID keyID,
	const macro_type & macro)
{
	GK_LOG_FUNC

	try {
		mBank_type & bank = _macrosBanks.at(bankID);

		try {
			LOG(info) << "macros bankID: M" << bankID
				<< " - Macro Key: " << getGKeyName(keyID)
				<< " - Macro Size: " << macro.size()
				<< " - setting macro";
			if( macro.size() >= MACRO_T_MAX_SIZE ) {
				LOG(warning) << "skipping macro - size >= MACRO_T_MAX_SIZE";
				throw GLogiKExcept("skipping macro");
			}

			bank.at(keyID) = macro;
		}
		catch(const std::out_of_range& oor) {
			this->throwWarn("wrong map key: ", getGKeyName(keyID), "can't set macro");
		}
	}
	catch (const std::out_of_range& oor) {
		this->throwWarn("wrong bankID: ", idToString(bankID), "can't set macro");
	}
}

const macro_type & MacrosBanks::getMacro(const MKeysID bankID, const GKeysID keyID)
{
	GK_LOG_FUNC

	try {
		mBank_type & bank = _macrosBanks.at(bankID);
		try {
			return bank.at(keyID);
		}
		catch(const std::out_of_range& oor) {
			this->throwWarn("wrong map key: ", getGKeyName(keyID), "can't get macro");
		}
	}
	catch (const std::out_of_range& oor) {
		this->throwWarn("wrong bankID: ", idToString(bankID), "can't get macro");
	}

	return MacrosBanks::emptyMacro;
}

void MacrosBanks::resetMacrosBank(const MKeysID bankID)
{
	try {
		for(auto & keyMacroPair : _macrosBanks.at(bankID)) {
			keyMacroPair.second.clear();
		}
	}
	catch (const std::out_of_range& oor) {
		this->throwWarn("wrong bankID: ", idToString(bankID), "can't reset macro bank");
	}
}

void MacrosBanks::throwWarn(
	const std::string & s1,
	const std::string & s2,
	const std::string & s3) const
{
	std::string warn(s1); warn += s2;
	GKSysLogWarning(warn);
	throw GLogiKExcept(s3);
}

const MKeysID MacrosBanks::getBankID(const uint8_t num) const
{
	if(num > MKeysID::MKEY_M3)
		throw GLogiKExcept("wrong bankID value");

	const MKeysID id = static_cast<MKeysID>(num);

	return id;
}

} // namespace GLogiK

