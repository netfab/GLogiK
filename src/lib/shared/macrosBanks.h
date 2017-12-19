/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2017  Fabrice Delliaux <netbox253@gmail.com>
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

#include "keyEvent.h"

namespace GLogiK
{

enum class MemoryBank : uint8_t
{
	MACROS_M0 = 0,
	MACROS_M1,
	MACROS_M2,
	MACROS_M3,
};

typedef std::map<const MemoryBank, std::map<const std::string, macro_t>> macros_map_t;

class MacrosBanks
{
	public:

		static const macro_t empty_macro_;
		static const macros_map_t empty_macros_profiles_;

		const macros_map_t & getMacrosProfiles(void) const { return this->macros_profiles_; };
		void setMacrosProfiles(const macros_map_t & macros) { this->macros_profiles_ = macros; };

		void setMacro(
			const uint8_t profile,
			const std::string & keyName,
			const macro_t & macro_array
		);
		void setMacro(
			const MemoryBank & profile,
			const std::string & keyName,
			const macro_t & macro_array
		);
		const macro_t & getMacro(
			const uint8_t profile,
			const std::string & keyName
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

		macros_map_t macros_profiles_ = {
			{ MemoryBank::MACROS_M0, {}},
			{ MemoryBank::MACROS_M1, {}},
			{ MemoryBank::MACROS_M2, {}},
			{ MemoryBank::MACROS_M3, {}}
		};

	private:

};

} // namespace GLogiK

#endif
