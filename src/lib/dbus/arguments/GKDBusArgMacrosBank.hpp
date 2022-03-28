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

#ifndef SRC_LIB_DBUS_ARG_GKDBUS_ARG_MACROS_BANK_HPP_
#define SRC_LIB_DBUS_ARG_GKDBUS_ARG_MACROS_BANK_HPP_

#include "GKDBusArgByte.hpp"
#include "GKDBusArgGKeysID.hpp"
#include "GKDBusArgMacro.hpp"

#include "include/keyEvent.hpp"

namespace NSGKDBus
{

class GKDBusArgumentMacrosBank
	:	virtual private GKDBusArgumentByte,
		virtual private GKDBusArgumentGKeysID,
		private GKDBusArgumentMacro
{
	public:
		static const GLogiK::mBank_type getNextMacrosBankArgument(void);

	protected:
		GKDBusArgumentMacrosBank(void) = default;
		~GKDBusArgumentMacrosBank(void) = default;

	private:

};

} // namespace NSGKDBus

#endif
