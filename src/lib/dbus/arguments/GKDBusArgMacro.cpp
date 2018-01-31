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
 * helper function rebuilding macro_t vector
 * mirror of GKDBusMessage::appendMacro
 */
const GLogiK::macro_t GKDBusArgumentMacro::getNextMacroArgument(const unsigned int macro_size) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "rebuilding macro from GKDBus values";
#endif
	GLogiK::macro_t macro_array;
	try {
		bool nextrun = true;
		do {
			GLogiK::KeyEvent e;

			e.event_code = GKDBusArgumentByte::getNextByteArgument();

			const uint8_t value = GKDBusArgumentByte::getNextByteArgument();
			if( to_uint(value) > to_uint(GLogiK::EventValue::EVENT_KEY_UNKNOWN) )
				throw GLogiKExcept("wrong event value for enum conversion");

			e.event		 = static_cast<GLogiK::EventValue>(value);
			e.interval	 = GKDBusArgumentUInt16::getNextUInt16Argument();

			macro_array.push_back(e);

			if( macro_size > 0 ) {
				if( macro_array.size() == macro_size )
					nextrun = false;
			}
			else {
				if( GKDBusArgumentByte::byte_arguments_.empty() )
					nextrun = false;
			}
		}
		while( nextrun );
	}
	catch ( const EmptyContainer & e ) {
		LOG(WARNING) << "missing macro argument : " << e.what();
		throw GLogiKExcept("rebuilding macro failed");
	}

	if( ( macro_size == 0 ) and ( ! GKDBusArgumentUInt16::uint16_arguments_.empty() ) ) { /* sanity check */
		LOG(WARNING) << "uint16 container not empty";
	}

#if DEBUGGING_ON
	LOG(DEBUG3) << "events array size : " << macro_array.size() << " - expected : " << macro_size;
#endif
	return macro_array;
}

} // namespace NSGKDBus

