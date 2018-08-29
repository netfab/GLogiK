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

#include <cstdint>

#include "lib/utils/utils.h"

#include "GKDBusArgMacro.h"

namespace NSGKDBus
{

using namespace NSGKUtils;

/*
 * helper function rebuilding macro_type vector
 * mirror of GKDBusMessage::appendMacro
 */
const GLogiK::macro_type GKDBusArgumentMacro::getNextMacroArgument(const unsigned int macroSize) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "rebuilding macro from GKDBus values";
#endif
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
		LOG(WARNING) << "missing macro argument : " << e.what();
		throw GLogiKExcept("rebuilding macro failed");
	}

	if( ( macroSize == 0 ) and ( ! GKDBusArgumentUInt16::uint16Arguments.empty() ) ) { /* sanity check */
		LOG(WARNING) << "uint16 container not empty";
	}

#if DEBUGGING_ON
	LOG(DEBUG3) << "events array size : " << macro.size() << " - expected : " << macroSize;
#endif
	return macro;
}

} // namespace NSGKDBus

