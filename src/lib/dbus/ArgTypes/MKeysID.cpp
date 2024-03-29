/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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

#include "MKeysID.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

void TypeMKeysID::appendMKeysID(const GLogiK::MKeysID keyID)
{
	const uint8_t value = toEnumType(keyID);
	this->appendUInt8(value);
}

/*
 * helper function to get MKeysID
 * see also TypeMKeysID::appendMKeysID
 */
const GLogiK::MKeysID ArgMKeysID::getNextMKeysIDArgument(void)
{
	GK_LOG_FUNC

	GLogiK::MKeysID id = GLogiK::MKeysID::MKEY_M0;

	try {
		const uint8_t value = ArgUInt8::getNextByteArgument();

		if(value > GLogiK::MKeyID_MAX)
			throw GLogiKExcept("wrong MKeysID value");

		id = static_cast<GLogiK::MKeysID>(value);
	}
	catch ( const EmptyContainer & e ) {
		LOG(warning) << "missing argument : " << e.what();
		throw GLogiKExcept("get MKeysID argument failed");
	}

	return id;
}

} // namespace NSGKDBus
