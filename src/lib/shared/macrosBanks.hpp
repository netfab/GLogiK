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

#ifndef SRC_LIB_SHARED_MACROS_BANKS_HPP_
#define SRC_LIB_SHARED_MACROS_BANKS_HPP_

#include <cstdint>

#include <vector>
#include <map>

#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

#include "include/base.hpp"

namespace GLogiK
{

class MacrosBanks
{
	public:
		static const macro_type emptyMacro;
		static const banksMap_type emptyMacrosBanks;

		void initMacrosBanks(
			const MKeysIDArray_type & MKeysIDArray,
			const GKeysIDArray_type & GKeysIDArray
		);

		const banksMap_type & getMacrosBanks(void) const;
		void setMacrosBanks(const banksMap_type & macrosBanks);
		void checkMacrosBanksKeys(void);

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

		void resetMacrosBank(const MKeysID bankID);

	protected:
		MacrosBanks(void);
		~MacrosBanks(void);

		friend class boost::serialization::access;

		template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
		{
			ar & _macrosBanks;
		}

		banksMap_type _macrosBanks;

	private:
		const MKeysID getBankID(const uint8_t num) const;

};

} // namespace GLogiK

#endif
