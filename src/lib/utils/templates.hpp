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

#include <cstddef>
#include <cstdint>

#include <charconv>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <type_traits>

#include "exception.hpp"

namespace NSGKUtils
{

template <typename T>
constexpr typename std::underlying_type<T>::type toEnumType(T obj) noexcept
{
	return static_cast<typename std::underlying_type<T>::type>(obj);
}

template <typename Iterator>
const std::size_t safeAdvance(Iterator & it, const Iterator & end, const std::size_t n)
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

template <typename IntegerType>
constexpr std::string_view getTypeName()
{
	if constexpr (std::is_same_v<IntegerType, std::uint8_t>)
		return "std::uint8_t";
	else if constexpr (std::is_same_v<IntegerType, std::uint16_t>)
		return "std::uint16_t";
	else if constexpr (std::is_same_v<IntegerType, std::uint32_t>)
		return "std::uint32_t";
	else if constexpr (std::is_same_v<IntegerType, std::uint64_t>)
		return "std::uint64_t";
	return "unknown";
}

template <typename IntegerType, typename Container>
const IntegerType castContainerSizeToInteger(const Container & container)
{
	auto size = container.size();
	if(size > static_cast<std::size_t>(std::numeric_limits<IntegerType>::max()))
		throw std::overflow_error(
			"container size exceeds maximum value for " + std::string(getTypeName<IntegerType>())
		);
	return static_cast<IntegerType>(size);
}

template <typename T>
const T toInteger(std::string_view s, int base = 10)
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

} // namespace NSGKUtils
