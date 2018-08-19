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

#ifndef __GLOGIK_MACROS_BANKS_H__
#define __GLOGIK_MACROS_BANKS_H__

#include <cstdint>

#include <vector>
#include <map>
#include <string>

#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

#include "include/keyEvent.h"

namespace GLogiK
{

class MacrosBanks
{
	public:

		static const macro_type empty_macro_;
		static const macros_map_type empty_macros_profiles_;

		void initMacrosProfiles(const std::vector<std::string> & keys_names);

		const macros_map_type & getMacrosProfiles(void) const;
		void setMacrosProfiles(const macros_map_type & macros);

		void clearMacro(
			const uint8_t profile,
			const std::string & keyName
		);
		void setMacro(
			const uint8_t profile,
			const std::string & keyName,
			const macro_type & macro_array
		);
		const macro_type & getMacro(
			const uint8_t profile,
			const std::string & keyName
		);

		void clearMacro(
			const MemoryBank & profile,
			const std::string & keyName
		);

		void setMacro(
			const MemoryBank & profile,
			const std::string & keyName,
			const macro_type & macro_array
		);

	protected:
		MacrosBanks(void);
		~MacrosBanks(void);

		friend class boost::serialization::access;

		template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
		{
			ar & this->macros_profiles_;
		}

		macros_map_type macros_profiles_ = {
			{ MemoryBank::BANK_M0, {}},
			{ MemoryBank::BANK_M1, {}},
			{ MemoryBank::BANK_M2, {}},
			{ MemoryBank::BANK_M3, {}}
		};

	private:
};

} // namespace GLogiK

#endif
