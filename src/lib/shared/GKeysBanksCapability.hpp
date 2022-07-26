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

#ifndef SRC_LIB_SHARED_GKEYS_BANKS_CAPABILITY_HPP_
#define SRC_LIB_SHARED_GKEYS_BANKS_CAPABILITY_HPP_

#include <cstdint>

#include <vector>
#include <map>

#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

#include "include/base.hpp"

namespace GLogiK
{

class GKeysBanksCapability
{
	public:
		static const macro_type emptyMacro;
		static const banksMap_type emptyGKeysBanks;

		void initBanks(
			const MKeysIDArray_type & MKeysIDArray,
			const GKeysIDArray_type & GKeysIDArray
		);

		const banksMap_type & getBanks(void) const;
		void setBanks(const banksMap_type & GKeysBanks);
		void checkBanksKeys(void);

		const macro_type & getMacro(
			const MKeysID bankID,
			const GKeysID keyID
		);

		void clearMacro(
			const MKeysID bankID,
			const GKeysID keyID
		);

		void setMacro(
			const MKeysID bankID,
			const GKeysID keyID,
			const macro_type & macro
		);

		void resetBank(const MKeysID bankID);

	protected:
		GKeysBanksCapability(void);
		virtual ~GKeysBanksCapability(void) = 0;

		friend class boost::serialization::access;

		template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
		{
			ar & _GKeysBanks;
		}

		banksMap_type _GKeysBanks;

	private:
		const MKeysID getBankID(const uint8_t num) const;

};

} // namespace GLogiK

#endif