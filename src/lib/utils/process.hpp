/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_LIB_UTILS_PROCESS_HPP_
#define SRC_LIB_UTILS_PROCESS_HPP_

#if !defined (UTILS_INSIDE_UTILS_H) && !defined (UTILS_COMPILATION)
#error "Only "utils/utils.hpp" can be included directly, this file may disappear or change contents."
#endif

#include <cstdint>
#include <sys/types.h>

#include "templates.hpp"

namespace NSGKUtils
{

class process
{
	typedef void (*__signal_handler_t) (int);

	public:
		enum class mask : uint8_t
		{
			PROCESS_LOG_ENTRIES = 1 << 0,
			PROCESS_CLOSE_DESCRIPTORS = 1 << 1,
		};

		static const pid_t detach(void);
		static const pid_t deamonize(void);
		static void setSignalHandler(int signum, __signal_handler_t __handler);
		static void resetSignalHandler(int signum);

	protected:

	private:
		process(void) = delete;
		~process(void) = delete;

		static uint8_t options;

		static const pid_t newPID(void);
};

inline const uint8_t operator & (const uint8_t value, const process::mask option)
{
	uint8_t ret(value);
	ret &= toEnumType(option);
	return ret;
}

inline uint8_t& operator |= (uint8_t& value, const process::mask option)
{
	return value=(value|toEnumType(option));
}

} // namespace NSGKUtils

#endif
