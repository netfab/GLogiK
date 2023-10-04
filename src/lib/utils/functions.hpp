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

#ifndef SRC_LIB_UTILS_FUNCTIONS_HPP_
#define SRC_LIB_UTILS_FUNCTIONS_HPP_

#if !defined (UTILS_INSIDE_UTILS_H) && !defined (UTILS_COMPILATION)
#error "Only "utils/utils.hpp" can be included directly, this file may disappear or change contents."
#endif

#include <sys/types.h>
#include <cstdint>
#include <string>
#include <type_traits>
#include <chrono>

//#include "GKLogging.hpp"

namespace NSGKUtils
{

template <typename T>
constexpr typename std::underlying_type<T>::type toEnumType(T obj) noexcept
{
	return static_cast<typename std::underlying_type<T>::type>(obj);
}

template <typename Iterator>
const size_t safeAdvance(Iterator & it, const Iterator & end,
	const std::size_t n)
{
	std::size_t i = 0;
	for(; i != n ; ++i)
	{
		it++;
		if(it == end)
			break;
	}
	//GKLog2(trace, "undone steps: ", (n - i))
	return (n - i);
}

const std::string toString(const char* s);
const std::wstring toWString(const wchar_t* s);

constexpr int toInt(const uint8_t c) noexcept
{
	return static_cast<int>(c);
}

constexpr unsigned int toUInt(const uint8_t c) noexcept
{
	return static_cast<unsigned int>(c);
}

const unsigned int toUInt(const std::string & s);
const unsigned short toUShort(const std::string & s, int base = 10);

const unsigned long toUL(const std::string & s, int base = 10);
const unsigned long long toULL(const std::string & s);

const std::string getHexRGB(
	const uint8_t red,
	const uint8_t green,
	const uint8_t blue);

void yield_for(std::chrono::microseconds us);

pid_t detachProcess(const bool closeDescriptors=false);

} // namespace NSGKUtils

#endif
