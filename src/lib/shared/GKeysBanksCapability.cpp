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
#include "GKeysBanksCapability.hpp"
#include "GKeysMacro.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

const banksMap_type GKeysBanksCapability::emptyGKeysBanks = {};

GKeysBanksCapability::GKeysBanksCapability(void)
	:	_currentBankID(MKeysID::MKEY_M0)
{
}

GKeysBanksCapability::~GKeysBanksCapability(void)
{
}

void GKeysBanksCapability::initBanks(
	const MKeysIDArray_type & MKeysIDArray,
	const GKeysIDArray_type & GKeysIDArray)
{
	GK_LOG_FUNC

	GKLog4(trace, "numBanks: ", MKeysIDArray.size(), "numKeys: ", GKeysIDArray.size())

	if( MKeysIDArray.empty() ) {
		LOG(error) << "empty MKeysID array !";
		return;
	}

	MKeysIDArray_type MKeysIDArr(MKeysIDArray);
	MKeysIDArr.emplace(MKeysIDArr.begin(), MKeysID::MKEY_M0);

	for(const MKeysID & id : MKeysIDArr) {
		try {
			{
				/* XXX - c++17 structured bindings */
				typedef std::pair<banksMap_type::iterator, bool> bankInsRet;
				bankInsRet ret = _GKeysBanks.insert( std::pair<const MKeysID, mBank_type>(id, {}) );
				if( ! ret.second )
					throw GLogiKExcept("bank insert failure");
			}

			unsigned short insertedKeys = 0;

			for(const auto & keyID : GKeysIDArray) {
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

				keyInsRet insKey = _GKeysBanks[id].insert(
					std::pair<GKeysID, GKeysEvent>( keyID, GKeysMacro::emptyMacro )
				);

				insertStatus(insKey);
			}

			GKLog4(trace, "bank id: ", id, "number of initialized G-Keys: ", insertedKeys)
		}
		catch(const GLogiKExcept & e) {
			LOG(error) << "bank id: " << id << " - failed to initialize: " << e.what();
		}
	}
}

banksMap_type & GKeysBanksCapability::getBanks(void)
{
	return _GKeysBanks;
}

const banksMap_type & GKeysBanksCapability::getBanks(void) const
{
	return _GKeysBanks;
}

void GKeysBanksCapability::setBanks(const banksMap_type & GKeysBanks)
{
	GK_LOG_FUNC

	GKLog(trace, "setting G-Keys banks")

	_GKeysBanks = GKeysBanks;
}

void GKeysBanksCapability::checkBanksKeys(void)
{
	for(auto bankIt = _GKeysBanks.begin(); bankIt != _GKeysBanks.end();) {
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
			bankIt = _GKeysBanks.erase(bankIt);
		}
	}

	for(auto bankIt = _GKeysBanks.begin(); bankIt != _GKeysBanks.end(); bankIt++) {
		auto & mBank = bankIt->second;
		for(auto it = mBank.begin(); it != mBank.end();) {
			try {
				if(it->first == GKeyID_INV)
					throw GLogiKExcept("invalid value");

				const uint8_t id = toEnumType(it->first);

				if(id > GLogiK::GKeyID_MAX)
					throw GLogiKExcept("wrong GKeyID value");

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

void GKeysBanksCapability::setCurrentBankID(MKeysID bankID)
{
	GK_LOG_FUNC

	GKLog2(trace, "setting current bank ID : ", bankID)

	_currentBankID = bankID;
}

const MKeysID GKeysBanksCapability::getCurrentBankID(void) const
{
	return _currentBankID;
}

void GKeysBanksCapability::resetBank(const MKeysID bankID)
{
	try {
		for(auto & keyEventPair : _GKeysBanks.at(bankID)) {
			keyEventPair.second.clearMacro();
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(warning) << "wrong bankID: " << bankID;
		throw GLogiKExcept("reset bank failed");
	}
}

const MKeysID GKeysBanksCapability::getBankID(const uint8_t num) const
{
	if(num > GLogiK::MKeyID_MAX)
		throw GLogiKExcept("wrong bankID value");

	const MKeysID id = static_cast<MKeysID>(num);

	return id;
}

} // namespace GLogiK

