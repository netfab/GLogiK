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

#ifndef __GLOGIK_KEY_EVENT_H__
#define __GLOGIK_KEY_EVENT_H__

#include <cstdint>

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

struct KeyEvent {
	public:
		uint8_t event_code;
		EventValue event;
		uint16_t interval;

		KeyEvent(uint8_t c=KEY_UNKNOWN, EventValue e=EventValue::EVENT_KEY_UNKNOWN, uint16_t i=0)
			: event_code(c), event(e), interval(i) {}

	private:
		friend class boost::serialization::access;

		template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
		{
			ar & this->event_code;
			ar & this->event;
			ar & this->interval;
		}
};

enum class MemoryBank : uint8_t
{
	BANK_M0 = 0,
	BANK_M1,
	BANK_M2,
	BANK_M3,
};

typedef std::vector<KeyEvent> macro_t;
typedef std::map<const std::string, macro_t> macros_bank_t;
typedef std::map<const MemoryBank, macros_bank_t> macros_map_t;

} // namespace GLogiK

#endif
