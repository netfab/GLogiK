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

#ifndef __GLOGIK_GKDBUS_ARG_MACROSBANK_H__
#define __GLOGIK_GKDBUS_ARG_MACROSBANK_H__

#include "lib/shared/keyEvent.h"

#include "GKDBusArgString.h"
#include "GKDBusArgByte.h"
#include "GKDBusArgMacro.h"

namespace NSGKDBus
{

class GKDBusArgumentMacrosBank
	:	virtual public GKDBusArgumentString,
		virtual public GKDBusArgumentByte,
		public GKDBusArgumentMacro
{
	public:
		static const GLogiK::macros_bank_t getNextMacrosBankArgument(void);

	protected:
		GKDBusArgumentMacrosBank(void) = default;
		~GKDBusArgumentMacrosBank(void) = default;

	private:

};

} // namespace NSGKDBus

#endif
