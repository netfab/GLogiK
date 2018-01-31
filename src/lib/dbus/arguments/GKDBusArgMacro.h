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

#ifndef __GLOGIK_GKDBUS_ARG_MACRO_H__
#define __GLOGIK_GKDBUS_ARG_MACRO_H__

#include "GKDBusArgByte.h"
#include "GKDBusArgUInt16.h"

#include "lib/shared/keyEvent.h"

namespace NSGKDBus
{

class GKDBusArgumentMacro
	:	virtual private GKDBusArgumentByte,
		virtual private GKDBusArgumentUInt16
{
	public:
		static const GLogiK::macro_t getNextMacroArgument(const unsigned int macroSize = 0);

	protected:
		GKDBusArgumentMacro(void) = default;
		~GKDBusArgumentMacro(void) = default;

	private:

};

} // namespace NSGKDBus

#endif
