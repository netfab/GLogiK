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

#include "lib/shared/glogik.hpp"
#include "lib/utils/utils.hpp"

#include "GKeysEventManager.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

GKeysEventManager::GKeysEventManager(void)
{
}

GKeysEventManager::~GKeysEventManager(void)
{
}

/* run a macro on the virtual keyboard */
void GKeysEventManager::runMacro(
	const banksMap_type & GKeysBanks,
	const MKeysID bankID,
	const GKeysID keyID)
{
	GK_LOG_FUNC

	if(keyID == GKeyID_INV) {
		LOG(error) << "invalid GKeyID";
		return;
	}

	try {
		const macro_type & macro = this->getMacro(GKeysBanks, bankID, keyID);
		if(macro.size() == 0) {
#if DEBUGGING_ON
			if(GKLogging::GKDebug) {
				LOG(trace)	<< "MBank: " << bankID
							<< " - GKey: " << getGKeyName(keyID)
							<< " - no macro recorded";
			}
#endif
			return;
		}

#if DEBUGGING_ON
		if(GKLogging::GKDebug) {
			LOG(trace)	<< "MBank: " << bankID
						<< " - GKey: " << getGKeyName(keyID)
						<< " - running macro";
		}
#endif
		for(const auto & key : macro) {
			_virtualKeyboard.sendKeyEvent(key);
		}
	}
	catch (const GLogiKExcept & e) {
		LOG(warning) << "macro key container not found";
	}
}

void GKeysEventManager::setMacro(
	banksMap_type & GKeysBanks,
	macro_type macro,
	const MKeysID bankID,
	const GKeysID keyID)
{
	GK_LOG_FUNC

	if(keyID == GKeyID_INV) {
		LOG(error) << "invalid GKeyID";
		return;
	}

	this->checkMacro(macro);

	this->setMacro(GKeysBanks, bankID, keyID, macro);
}

const macro_type & GKeysEventManager::getMacro(
	const banksMap_type & GKeysBanks,
	const MKeysID bankID,
	const GKeysID keyID)
{
	GK_LOG_FUNC

	if(keyID == GKeyID_INV) {
		LOG(error) << "invalid GKeyID";
		throw GLogiKExcept("get macro failed");
	}

	try {
		const mBank_type & bank = GKeysBanks.at(bankID);
		try {
			return bank.at(keyID).getMacro();
		}
		catch(const std::out_of_range& oor) {
			LOG(warning) << "wrong GKeyID: " << keyID;
			throw GLogiKExcept("get macro failed");
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(warning) << "wrong bankID: " << bankID;
		throw GLogiKExcept("get macro failed");
	}

	return GKeysMacro::emptyMacro;
}

void GKeysEventManager::clearMacro(
	banksMap_type & GKeysBanks,
	const MKeysID bankID,
	const GKeysID keyID)
{
	GK_LOG_FUNC

	if(keyID == GKeyID_INV) {
		LOG(error) << "invalid GKeyID";
		throw GLogiKExcept("clear macro failed");
	}

	try {
		mBank_type & bank = GKeysBanks.at(bankID);

		try {
			LOG(info) << "MBank: " << bankID
				<< " - GKey: " << getGKeyName(keyID)
				<< " - clearing macro";

			bank.at(keyID).clearMacro();
		}
		catch(const std::out_of_range& oor) {
			LOG(warning) << "wrong GKeyID: " << keyID;
			throw GLogiKExcept("clear macro failed");
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(warning) << "wrong bankID: " << bankID;
		throw GLogiKExcept("clear macro failed");
	}
}

void GKeysEventManager::setMacro(
	banksMap_type & GKeysBanks,
	const MKeysID bankID,
	const GKeysID keyID,
	const macro_type & macro)
{
	GK_LOG_FUNC

	if(keyID == GKeyID_INV) {
		LOG(error) << "invalid GKeyID";
		throw GLogiKExcept("set macro failed");
	}

	try {
		mBank_type & bank = GKeysBanks.at(bankID);

		try {
			LOG(info) << "MBank: " << bankID
				<< " - GKey: " << getGKeyName(keyID)
				<< " - Macro Size: " << macro.size()
				<< " - setting macro";
			if( macro.size() >= MACRO_T_MAX_SIZE ) {
				LOG(warning) << "skipping macro - size >= MACRO_T_MAX_SIZE";
				throw GLogiKExcept("skipping macro");
			}

			bank.at(keyID) = GKeysEvent(macro);
		}
		catch(const std::out_of_range& oor) {
			LOG(warning) << "wrong GKeyID: " << keyID;
			throw GLogiKExcept("set macro failed");
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(warning) << "wrong bankID: " << bankID;
		throw GLogiKExcept("set macro failed");
	}
}

} // namespace GLogiK

