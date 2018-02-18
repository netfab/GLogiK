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

#ifndef __GLOGIKD_MACROS_MANAGER_H__
#define __GLOGIKD_MACROS_MANAGER_H__

#include <cstdint>

#include <vector>
#include <string>
#include <sstream>

#include "lib/shared/macrosBanks.h"
#include "virtualKeyboard.h"

namespace GLogiK
{

struct MacroEvent {
	public:
		MacroEvent(const GLogiK::KeyEvent & k, const unsigned int i)
			:	key(k), index(i) {}

		GLogiK::KeyEvent key;
		unsigned int index;
};

class MacrosManager : public MacrosBanks
{
	public:
		MacrosManager(
			const char* virtual_keyboard_name,
			const std::vector<std::string> & keys_names
		);
		~MacrosManager();

		void setCurrentActiveProfile(MemoryBank bank);
		const MemoryBank getCurrentActiveProfile(void) const;

		const bool macroDefined(const std::string & macro_key_name);
		void runMacro(const std::string & macro_key_name);

		void setMacro(
			const std::string & keyName,
			macro_t & macro_array
		);

		void clearMacroProfiles(void);

		//void logProfiles(void);

	protected:

	private:
		std::ostringstream buffer_;

		MemoryBank currentActiveProfile_;
		VirtualKeyboard virtual_keyboard;

		void fixMacroReleaseEvents(
			const std::vector<MacroEvent> & pressedEvents,
			std::vector<MacroEvent> & releasedEvents,
			macro_t & macro_array
		);
};

} // namespace GLogiK

#endif
