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

#include "lib/utils/utils.hpp"

#include "GKDBusArgGKeysIDArray.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

/*
 * helper function rebuilding GKeysIDArray
 * mirror of GKDBusMessage::appendGKeysIDArray
 */
const GLogiK::GKeysIDArray_type GKDBusArgumentGKeysIDArray::getNextGKeysIDArrayArgument(void)
{
	GK_LOG_FUNC

	GKLog(trace, "rebuilding GKeysIDArray from GKDBus values")

	GLogiK::GKeysIDArray_type ret;

	try {
		const uint8_t size = GKDBusArgumentByte::getNextByteArgument();

		bool nextRun = true;
		do {
			ret.push_back(GKDBusArgumentGKeysID::getNextGKeysIDArgument());

			if( ret.size() == size )
				nextRun = false;
		}
		while( nextRun );
	}
	catch( const EmptyContainer & e ) {
		LOG(warning) << "missing GKeysID argument : " << e.what();
		throw GLogiKExcept("rebuilding GKeysIDArray failed");
	}

	GKLog2(trace, "GKeysIDArray size: ", ret.size())

	return ret;
}

} // namespace NSGKDBus

