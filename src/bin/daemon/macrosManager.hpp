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

#ifndef SRC_BIN_DAEMON_MACROS_MANAGER_HPP_
#define SRC_BIN_DAEMON_MACROS_MANAGER_HPP_

#include <cstdint>

#include <vector>
#include <string>

#include "lib/shared/macrosBanks.hpp"
#include "virtualKeyboard.hpp"

namespace GLogiK
{

struct MacroEvent {
	public:
		MacroEvent(const GLogiK::KeyEvent & k, const unsigned int i)
			:	key(k), index(i) {}

		GLogiK::KeyEvent key;
		unsigned int index;

	private:
		MacroEvent(void) = delete;
};

class MacrosManager : public MacrosBanks
{
	public:
		MacrosManager(
			const char* virtualKeyboardName,
			const uint8_t numBanks,
			const std::vector<std::string> & keysNames
		);
		~MacrosManager();

		void setCurrentMacrosBankID(BankID bankID);
		const BankID getCurrentMacrosBankID(void) const;

		const bool macroDefined(const std::string & keyName);
		void runMacro(const std::string & keyName);

		void setMacro(
			const std::string & keyName,
			macro_type & macro
		);

		void resetMacrosBanks(void);

	protected:

	private:
		VirtualKeyboard _virtualKeyboard;
		BankID _currentBankID;

		void fillInVectors(
			const macro_type & macro,
			std::vector<MacroEvent> & pressedEvents,
			std::vector<MacroEvent> & releasedEvents
		);
		void fixMacroReleaseEvents(
			const std::vector<MacroEvent> & pressedEvents,
			std::vector<MacroEvent> & releasedEvents,
			macro_type & macro
		);
		void fixMacroSize(
			const std::vector<MacroEvent> & pressedEvents,
			std::vector<MacroEvent> & releasedEvents,
			macro_type & macro
		);
};

} // namespace GLogiK

#endif
