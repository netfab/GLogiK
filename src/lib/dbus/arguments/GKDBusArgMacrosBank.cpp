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

#include "lib/utils/utils.h"

#include "GKDBusArgMacrosBank.h"

namespace NSGKDBus
{

using namespace NSGKUtils;

/*
 * helper function rebuilding macros_bank_type map
 */
const GLogiK::macros_bank_type GKDBusArgumentMacrosBank::getNextMacrosBankArgument(void) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "rebuilding macros bank from GKDBus values";
#endif
	GLogiK::macros_bank_type bank;

	try {
		do {
			const std::string key = GKDBusArgumentString::getNextStringArgument();
			const uint8_t size = GKDBusArgumentByte::getNextByteArgument();
			const GLogiK::macro_type macroArray = GKDBusArgumentMacro::getNextMacroArgument(size);

			bank[key] = macroArray;
		}
		while( ! GKDBusArgumentString::stringArguments.empty() );
	}
	catch ( const EmptyContainer & e ) {
		LOG(WARNING) << "missing argument : " << e.what();
		throw GLogiKExcept("rebuilding MacrosBank failed");
	}

	if( ! GKDBusArgumentByte::byteArguments.empty() ) { /* sanity check */
		LOG(WARNING) << "byte container not empty";
	}

#if DEBUGGING_ON
	LOG(DEBUG3) << "macros bank size : " << bank.size();
#endif
	return bank;
}

} // namespace NSGKDBus

