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

#include <cstdint>

#include "lib/utils/utils.hpp"

#include "GKDBusArgGKeysID.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

/*
 * helper function to get GKeysID
 * see also GKDBusMessage::appendGKeysID
 */
const GLogiK::GKeysID GKDBusArgumentGKeysID::getNextGKeysIDArgument(void)
{
	GK_LOG_FUNC

	GLogiK::GKeysID id = GLogiK::GKeysID::GKEY_G0;

	try {
		const uint8_t value = GKDBusArgumentByte::getNextByteArgument();

		if(value > GLogiK::GKeyID_MAX)
			throw GLogiKExcept("wrong GKeysID value");

		id = static_cast<GLogiK::GKeysID>(value);
	}
	catch ( const EmptyContainer & e ) {
		LOG(warning) << "missing argument : " << e.what();
		throw GLogiKExcept("get GKeysID argument failed");
	}

	return id;
}

} // namespace NSGKDBus
