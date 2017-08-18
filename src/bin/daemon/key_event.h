/*
 *
 *	This file is part of GLogiK project.
 *	GLogiKd, daemon to handle special features on gaming keyboards
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

#ifndef __GLOGIKD_KEY_EVENT_H__
#define __GLOGIKD_KEY_EVENT_H__

#include <linux/input-event-codes.h>

struct KeyEvent {
	unsigned char event_code;
	unsigned short event;
	uint64_t pressed_keys;

	// FIXME e=3
	KeyEvent(unsigned char c=KEY_UNKNOWN, unsigned short e=3, uint64_t p = 0)
		: event_code(c), event(e), pressed_keys(p) {}
};

#endif
