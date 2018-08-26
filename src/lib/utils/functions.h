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

#ifndef SRC_LIB_UTILS_FUNCTIONS_HPP_
#define SRC_LIB_UTILS_FUNCTIONS_HPP_

#if !defined (UTILS_INSIDE_UTILS_H) && !defined (UTILS_COMPILATION)
#error "Only "utils/utils.h" can be included directly, this file may disappear or change contents."
#endif

#include <cstdint>
#include <string>
#include <type_traits>
#include <chrono>

namespace NSGKUtils
{

template <typename T>
constexpr typename std::underlying_type<T>::type to_type(T obj) noexcept {
	return static_cast<typename std::underlying_type<T>::type>(obj);
}

const std::string to_string(const char* s);

constexpr int to_int(const uint8_t c)
{
	return static_cast<int>(c);
}

constexpr unsigned int to_uint(const uint8_t c)
{
	return static_cast<unsigned int>(c);
}

const unsigned int to_uint(const std::string & s);
const unsigned long long to_ull(const std::string & s);

const std::string getHexRGB(
	const uint8_t red,
	const uint8_t green,
	const uint8_t blue);

void yield_for(std::chrono::microseconds us);

} // namespace NSGKUtils

#endif
