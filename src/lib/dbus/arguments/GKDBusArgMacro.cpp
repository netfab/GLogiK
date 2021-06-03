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

#include <cstdint>

#include "lib/utils/utils.hpp"

#include "GKDBusArgMacro.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

/*
 * helper function rebuilding macro_type vector
 * mirror of GKDBusMessage::appendMacro
 */
const GLogiK::macro_type GKDBusArgumentMacro::getNextMacroArgument(const unsigned int macroSize)
{
	GK_LOG_FUNC

	GKLog(trace, "rebuilding macro from GKDBus values")

	GLogiK::macro_type macro;
	try {
		bool nextRun = true;
		do {
			GLogiK::KeyEvent e;

			e.code = GKDBusArgumentByte::getNextByteArgument();

			const uint8_t value = GKDBusArgumentByte::getNextByteArgument();
			if(value > GLogiK::EventValue::EVENT_KEY_UNKNOWN)
				throw GLogiKExcept("wrong event value for enum conversion");

			e.event		 = static_cast<GLogiK::EventValue>(value);
			e.interval	 = GKDBusArgumentUInt16::getNextUInt16Argument();

			macro.push_back(e);

			if( macroSize > 0 ) {
				if( macro.size() == macroSize )
					nextRun = false;
			}
			else {
				if( GKDBusArgumentByte::byteArguments.empty() )
					nextRun = false;
			}
		}
		while( nextRun );
	}
	catch ( const EmptyContainer & e ) {
		LOG(warning) << "missing macro argument : " << e.what();
		throw GLogiKExcept("rebuilding macro failed");
	}

	if( ( macroSize == 0 ) and ( ! GKDBusArgumentUInt16::uint16Arguments.empty() ) ) { /* sanity check */
		LOG(warning) << "uint16 container not empty";
	}

	GKLog4(trace, "events array size : ", macro.size(), " - expected : ", macroSize)

	return macro;
}

} // namespace NSGKDBus

