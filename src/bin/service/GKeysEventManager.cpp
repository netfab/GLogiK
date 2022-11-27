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

#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>

#include "lib/shared/glogik.hpp"
#include "lib/utils/utils.hpp"

#include <boost/process.hpp>
#include <boost/process/search_path.hpp>

#include "GKeysEventManager.hpp"

namespace bp = boost::process;

namespace GLogiK
{

using namespace NSGKUtils;

GKeysEventManager::GKeysEventManager(void)
{
}

GKeysEventManager::~GKeysEventManager(void)
{
}

void GKeysEventManager::runEvent(
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
		const mBank_type & bank = GKeysBanks.at(bankID);
		try {
			const GKeysEvent & event = bank.at(keyID);
			GKLog4(trace, "MBank: ", bankID, "GKey: ", getGKeyName(keyID))

			if(event.getEventType() == GKeyEventType::GKEY_INACTIVE) {
				GKLog(trace, "inactive event")
			}
			else if(event.getEventType() == GKeyEventType::GKEY_MACRO) {
				const macro_type & macro = event.getMacro();
				if( ! macro.empty() ) {
					GKLog(trace, "running macro")
					for(const auto & key : macro) {
						_virtualKeyboard.sendKeyEvent(key);
					}
				}

			}
			else if(event.getEventType() == GKeyEventType::GKEY_RUNCMD) {
				this->spawnProcess(event.getCommand());
			}
			else if(event.getEventType() == GKeyEventType::GKEY_INVALID) {
				LOG(error) << "invalid event type";
				return;
			}
		}
		catch(const std::out_of_range& oor) {
			LOG(warning) << "wrong GKeyID: " << keyID;
			throw GLogiKExcept("run event failed");
		}
	}
	catch (const std::out_of_range& oor) {
		LOG(warning) << "wrong bankID: " << bankID;
		throw GLogiKExcept("run event failed");
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

/* return true if macro was not empty and has been cleared */
const bool GKeysEventManager::clearMacro(
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
		GKeysEvent & event = bank.at(keyID);

		try {
			if( ! event.getMacro().empty() ) {
				LOG(info) << "MBank: " << bankID
					<< " - GKey: " << getGKeyName(keyID)
					<< " - clearing macro";

				event.clearMacro();
				event.setEventType(GKeyEventType::GKEY_INACTIVE);
				return true;
			}
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

	return false;
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

void GKeysEventManager::spawnProcess(const std::string & command)
{
	GK_LOG_FUNC

	GKLog2(trace, "command line: ", command)

	std::string exe;
	std::vector<std::string> args;
	{
		std::istringstream tmpstream(command);
		std::string tmpstring;
		std::getline(tmpstream, exe, ' ');
		while(std::getline(tmpstream, tmpstring, ' '))
		{
			args.push_back(tmpstring);
		}
	}

	try {
		auto p = bp::search_path(exe);
		if( p.empty() ) {
			LOG(error) << exe << " executable not found in PATH";
			return;
		}

		GKLog4(trace, "spawning: ", exe, "with args size: ", args.size())

		bp::group g;
		bp::spawn(p, bp::args(args), g);
		g.wait();
	}
	catch (const bp::process_error & e) {
		LOG(error) << "exception catched while trying to spawn process: " << command;
		LOG(error) << e.what();
	}
}

} // namespace GLogiK

