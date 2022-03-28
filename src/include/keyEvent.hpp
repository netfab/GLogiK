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

#ifndef SRC_INCLUDE_KEY_EVENT_HPP_
#define SRC_INCLUDE_KEY_EVENT_HPP_

#include <cstdint>

#include <sstream>
#include <string>
#include <vector>
#include <map>

#include <linux/input-event-codes.h>

#include <boost/serialization/access.hpp>

//
// Macro Size Limit
//
/* upper size limit, above this size events
 * that are not key release will be ignored */
#define MACRO_T_KEYPRESS_MAX_SIZE 16
/* above this size, events are simply ignored */
#define MACRO_T_MAX_SIZE 24

namespace GLogiK
{

enum class EventValue : uint8_t
{
	EVENT_KEY_RELEASE = 0,
	EVENT_KEY_PRESS,
	EVENT_KEY_UNKNOWN,
};

inline std::ostream & operator << (std::ostream & stream, const EventValue & event)
{
	stream << static_cast<unsigned int>(event);
	return stream;
}

inline const bool operator > (const uint8_t value, const EventValue & event)
{
	return ((static_cast<unsigned int>(value)) > (static_cast<unsigned int>(event)));
}

struct KeyEvent {
	public:
		uint8_t code;
		EventValue event;
		uint16_t interval;

		KeyEvent(uint8_t c=KEY_UNKNOWN, EventValue e=EventValue::EVENT_KEY_UNKNOWN, uint16_t i=0)
			: code(c), event(e), interval(i) {}

	private:
		friend class boost::serialization::access;

		template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
		{
			ar & code;
			ar & event;
			ar & interval;
		}
};

enum class MKeysID : uint8_t
{
	MKEY_M0 = 0,
	MKEY_M1,
	MKEY_M2,
	MKEY_M3,
};

enum class GKeysID : uint8_t
{
	GKEY_G0 = 0,
	GKEY_G1,
	GKEY_G2,
	GKEY_G3,
	GKEY_G4,
	GKEY_G5,
	GKEY_G6,
	GKEY_G7,
	GKEY_G8,
	GKEY_G9,
	GKEY_G10,
	GKEY_G11,
	GKEY_G12,
	GKEY_G13,
	GKEY_G14,
	GKEY_G15,
	GKEY_G16,
	GKEY_G17,
	GKEY_G18,
};

const GKeysID GKeyID_MAX = GKeysID::GKEY_G18;
const GKeysID GKeyID_INV = GKeysID::GKEY_G0; // invalid
const MKeysID MKeyID_MAX = MKeysID::MKEY_M3;

// GKeysIDArray
typedef std::vector<GKeysID> GKeysIDArray_type;

inline std::ostream & operator << (std::ostream & stream, const MKeysID keyID)
{
	stream << static_cast<unsigned int>(keyID);
	return stream;
}

inline const bool operator > (const uint8_t value, const GKeysID keyID)
{
	return ((static_cast<unsigned int>(value)) > (static_cast<unsigned int>(keyID)));
}

inline const bool operator > (const uint8_t value, const MKeysID keyID)
{
	return ((static_cast<unsigned int>(value)) > (static_cast<unsigned int>(keyID)));
}

// macro
typedef std::vector<KeyEvent> macro_type;
// macros bank
typedef std::map<GKeysID, macro_type> mBank_type;
// banks map container
typedef std::map<MKeysID, mBank_type> banksMap_type;

} // namespace GLogiK

#endif
