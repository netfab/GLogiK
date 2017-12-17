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

#include "lib/utils/utils.h"

#include "macrosBanks.h"

namespace GLogiK
{

MacrosBanks::MacrosBanks() {
}

MacrosBanks::~MacrosBanks() {
}

void MacrosBanks::updateMacro(
	const uint8_t profile,
	const std::string & keyName,
	const std::vector<KeyEvent> & macro_array)
{
	if( to_uint(profile) > to_uint(MemoryBank::MACROS_M3) )
		throw GLogiKExcept("wrong profile value");

	const MemoryBank current_profile = static_cast<MemoryBank>(profile);

	try {
#if DEBUGGING_ON
		LOG(DEBUG2) << "macros profile: " << to_uint(profile)
			<< " - Macro Key: " << keyName << " - setting macro";
#endif
		this->macros_profiles_[current_profile].at(keyName) = macro_array;
	}
	catch (const std::out_of_range& oor) {
		std::string warn("wrong map key : ");
		warn += keyName;
		GKSysLog(LOG_WARNING, WARNING, warn);
		throw GLogiKExcept("macro not updated");
	}
}

const std::vector<KeyEvent> MacrosBanks::getMacro(const uint8_t profile, const std::string & keyName)
{
	if( to_uint(profile) > to_uint(MemoryBank::MACROS_M3) )
		throw GLogiKExcept("wrong profile value");

	const MemoryBank current_profile = static_cast<MemoryBank>(profile);
	try {
		return this->macros_profiles_[current_profile].at(keyName);
	}
	catch (const std::out_of_range& oor) {
		std::string warn("wrong map key : ");
		warn += keyName;
		GKSysLog(LOG_WARNING, WARNING, warn);
		throw GLogiKExcept("can't get macro");
	}

	std::vector<KeyEvent> empty;
	return empty;
}

} // namespace GLogiK

