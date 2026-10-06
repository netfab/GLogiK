/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2026  Fabrice Delliaux <netbox253@gmail.com>
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

#pragma once

#if !defined (UTILS_INSIDE_UTILS_H) && !defined (UTILS_COMPILATION)
#error "Only "utils/utils.hpp" can be included directly, this file may disappear or change contents."
#endif

#include <charconv>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <chrono>
#include <limits>

#include "exception.hpp"

namespace NSGKUtils
{

const std::string toString(const char* s);
const std::wstring toWString(const wchar_t* s);

const std::string getErrnoString(const int errnum);

constexpr int toInt(const std::uint8_t c) noexcept
{
	return static_cast<int>(c);
}

constexpr unsigned int toUInt(const std::uint8_t c) noexcept
{
	return static_cast<unsigned int>(c);
}

template <typename Integer, typename Container>
void checkSizeMax(const Container& container, std::string_view typeName)
{
	if(container.size() > static_cast<std::size_t>(std::numeric_limits<Integer>::max()))
		throw std::overflow_error(
			"container size exceeds maximum value for " + std::string(typeName)
		);
}

template <typename T>
const T toInteger(const std::string & s, int base = 10)
{
	T value{};

	const auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value, base);

	if (ec == std::errc::invalid_argument)
		throw GLogiKExcept("std::from_chars invalid argument");
	if (ec == std::errc::result_out_of_range)
		throw GLogiKExcept("std::from_chars out of range");

	if (ptr != s.data() + s.size())
		throw GLogiKExcept("std::from_chars invalid character");

	return value;
}

const std::uint8_t toUInt8(const std::string & s, int base = 10);
const std::uint16_t toUInt16(const std::string & s, int base = 10);
//const std::uint32_t toUInt32(const std::string & s, int base = 10);
const std::uint64_t toUInt64(const std::string & s, int base = 10);

const std::string getHexRGB(
	const std::uint8_t red,
	const std::uint8_t green,
	const std::uint8_t blue
);

void yield_for(std::chrono::microseconds us);

} // namespace NSGKUtils
