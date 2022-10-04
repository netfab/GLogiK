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

#ifndef SRC_INCLUDE_MBANK_HPP_
#define SRC_INCLUDE_MBANK_HPP_

#include "base.hpp"

namespace GLogiK
{

enum class GKeyEventType : uint8_t
{
	GKEY_INACTIVE = 0,
	GKEY_MACRO,
};

class GKeysEvent {
	public:
		GKeysEvent(void) :
			_GKeyEventType(GKeyEventType::GKEY_INACTIVE),
			_GKeyMacro({})
		{}

		GKeysEvent(const macro_type & macro) :
			_GKeyEventType(GKeyEventType::GKEY_MACRO),
			_GKeyMacro(macro)
		{}

		void clearMacro(void) {
			_GKeyMacro.clear();
			_GKeyEventType = GKeyEventType::GKEY_INACTIVE;
		}
		const macro_type & getMacro(void) const { return _GKeyMacro; }

	private:
		GKeyEventType _GKeyEventType;
		macro_type _GKeyMacro;

		friend class boost::serialization::access;

		template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
		{
			ar & _GKeyEventType;
			ar & _GKeyMacro;
		}
};

// MBank map container
typedef std::map<GKeysID, GKeysEvent> mBank_type;
// top container
typedef std::map<MKeysID, mBank_type> banksMap_type;

} // namespace GLogiK

#endif
