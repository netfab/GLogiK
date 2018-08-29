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

enum class BankID : uint8_t
{
	BANK_M0 = 0,
	BANK_M1,
	BANK_M2,
	BANK_M3,
};

inline std::ostream & operator << (std::ostream & stream, const BankID & bankID)
{
	stream << static_cast<unsigned int>(bankID);
	return stream;
}

inline const bool operator > (const uint8_t value, const BankID & bankID)
{
	return ((static_cast<unsigned int>(value)) > (static_cast<unsigned int>(bankID)));
}

typedef std::vector<KeyEvent> macro_type;
typedef std::map<const std::string, macro_type> mBank_type;		/* macros bank */
typedef std::map<const BankID, mBank_type> banksMap_type;		/* banks map container */

} // namespace GLogiK

#endif
