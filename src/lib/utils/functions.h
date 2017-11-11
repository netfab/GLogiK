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

#ifndef __GLOGIK_FUNCTIONS_H__
#define __GLOGIK_FUNCTIONS_H__

#if !defined (UTILS_INSIDE_UTILS_H) && !defined (UTILS_COMPILATION)
#error "Only "utils/utils.h" can be included directly, this file may disappear or change contents."
#endif

#include <string>
#include <type_traits>

namespace GLogiK
{

template <typename T>
constexpr unsigned int to_uint(T obj) {
	return static_cast<unsigned int>(obj);
}

template <typename T>
constexpr typename std::underlying_type<T>::type to_type(T obj) noexcept {
	return static_cast<typename std::underlying_type<T>::type>(obj);
}

std::string to_string(const char* s);

} // namespace GLogiK

#endif
