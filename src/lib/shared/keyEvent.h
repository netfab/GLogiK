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

#ifndef __GLOGIK_KEY_EVENT_H__
#define __GLOGIK_KEY_EVENT_H__

#include <cstdint>
#include <linux/input-event-codes.h>

namespace GLogiK
{

enum class EventValue : int8_t
{
	EVENT_KEY_RELEASE = 0,
	EVENT_KEY_PRESS,
	EVENT_KEY_UNKNOWN,
};

struct KeyEvent {
	unsigned char event_code;
	EventValue event;
	uint16_t interval;

	KeyEvent(unsigned char c=KEY_UNKNOWN, EventValue e=EventValue::EVENT_KEY_UNKNOWN, uint16_t i=0)
		: event_code(c), event(e), interval(i) {}
};

} // namespace GLogiK

#endif
