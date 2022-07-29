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

#ifndef SRC_BIN_SERVICE_GKEYS_EVENT_MANAGER_HPP_
#define SRC_BIN_SERVICE_GKEYS_EVENT_MANAGER_HPP_

#include <vector>
#include <string>

#include "lib/shared/GKeysBanksCapability.hpp"
#include "virtualKeyboard.hpp"

#include "include/base.hpp"

namespace GLogiK
{

class GKeysEventManager
{
	public:
		GKeysEventManager(void);
		~GKeysEventManager(void);

		void runMacro(
			const banksMap_type & GKeysBanks,
			const MKeysID bankID,
			const GKeysID keyID
		);

		void setMacro(
			banksMap_type & GKeysBanks,
			macro_type & macro,
			const MKeysID bankID,
			const GKeysID keyID
		);

		const macro_type & getMacro(
			const banksMap_type & GKeysBanks,
			const MKeysID bankID,
			const GKeysID keyID
		);

		void clearMacro(
			banksMap_type & GKeysBanks,
			const MKeysID bankID,
			const GKeysID keyID
		);

	protected:

	private:
		struct MacroEvent {
			public:
				MacroEvent(const GLogiK::KeyEvent & k, const unsigned int i)
					:	key(k), index(i) {}

				GLogiK::KeyEvent key;
				unsigned int index;

			private:
				MacroEvent(void) = delete;
		};

		VirtualKeyboard _virtualKeyboard;

		void setMacro(
			banksMap_type & GKeysBanks,
			const MKeysID bankID,
			const GKeysID keyID,
			const macro_type & macro
		);

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
