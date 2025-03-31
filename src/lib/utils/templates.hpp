/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2025  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_LIB_UTILS_TEMPLATES_HPP_
#define SRC_LIB_UTILS_TEMPLATES_HPP_

#if !defined (UTILS_INSIDE_UTILS_H) && !defined (UTILS_COMPILATION)
#error "Only "utils/utils.hpp" can be included directly, this file may disappear or change contents."
#endif

#include <cstddef>

#include <type_traits>

//#include "GKLogging.hpp"

namespace NSGKUtils
{

template <typename T>
constexpr typename std::underlying_type<T>::type toEnumType(T obj) noexcept
{
	return static_cast<typename std::underlying_type<T>::type>(obj);
}

template <typename Iterator>
const std::size_t safeAdvance(Iterator & it, const Iterator & end,
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

} // namespace NSGKUtils

#endif
