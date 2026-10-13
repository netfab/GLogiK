/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2026  Fabrice Delliaux <netbox253@gmail.com>
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

#pragma once

#include "lib/shared/GKeysBanksCapability.hpp"
#include "lib/shared/GKeysMacro.hpp"
#include "virtualKeyboard.hpp"

#include "include/base.hpp"
#include "include/MBank.hpp"

namespace GLogiK
{

class GKeysEventManager
	:	private LIBShared::GKeysMacro
{
	private:
		using macro_type = GLogiK::macro_type;

	public:
		GKeysEventManager();
		~GKeysEventManager();

		void runEvent(
			const banksMap_type & GKeysBanks,
			MKeysID bankID,
			GKeysID keyID
		);

		void setMacro(
			banksMap_type & GKeysBanks,
			macro_type macro,
			MKeysID bankID,
			GKeysID keyID
		);

		bool clearMacro(
			banksMap_type & GKeysBanks,
			MKeysID bankID,
			GKeysID keyID
		);

	protected:

	private:
		VirtualKeyboard _virtualKeyboard;

		void setMacro(
			banksMap_type & GKeysBanks,
			MKeysID bankID,
			GKeysID keyID,
			const macro_type & macro
		);

		void spawnProcess(const std::string & command);
};

} // namespace GLogiK
