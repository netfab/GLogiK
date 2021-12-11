/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
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

#include "lib/utils/utils.hpp"

#include "GKDBusArgMacrosBank.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

/*
 * helper function rebuilding mBank_type map
 */
const GLogiK::mBank_type GKDBusArgumentMacrosBank::getNextMacrosBankArgument(void)
{
	GK_LOG_FUNC

	GKLog(trace, "rebuilding macros bank from GKDBus values")

	GLogiK::mBank_type bank;

	try {
		do {
			const std::string key = GKDBusArgumentString::getNextStringArgument();
			const uint8_t size = GKDBusArgumentByte::getNextByteArgument();
			const GLogiK::macro_type macro = GKDBusArgumentMacro::getNextMacroArgument(size);

			bank[key] = macro;
		}
		while( ! GKDBusArgumentString::stringArguments.empty() );
	}
	catch ( const EmptyContainer & e ) {
		LOG(warning) << "missing argument : " << e.what();
		throw GLogiKExcept("rebuilding MacrosBank failed");
	}

	if( ! GKDBusArgumentByte::byteArguments.empty() ) { /* sanity check */
		LOG(warning) << "byte container not empty";
	}

	GKLog2(trace, "macros bank size : ", bank.size())

	return bank;
}

} // namespace NSGKDBus

