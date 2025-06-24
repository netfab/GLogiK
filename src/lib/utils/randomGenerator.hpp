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

#ifndef SRC_LIB_UTILS_RANDOM_GENERATOR_HPP_
#define SRC_LIB_UTILS_RANDOM_GENERATOR_HPP_

#if !defined (UTILS_INSIDE_UTILS_H) && !defined (UTILS_COMPILATION)
#error "Only "utils/utils.hpp" can be included directly, this file may disappear or change contents."
#endif

#include <vector>
#include <string>
#include <random>

namespace NSGKUtils
{

class RandomGenerator
{
	public:
		RandomGenerator(void) : RandomGenerator(_defaultCharset) {};
		RandomGenerator(const std::vector<char> & charset);
		~RandomGenerator(void);

		const std::string getString(std::size_t length);

	protected:

	private:
		std::vector<char> _charset;
		std::default_random_engine _rng;
		std::uniform_int_distribution<> _dist;

		const std::vector<char> _defaultCharset = {
			'0','1','2','3','4','5','6','7','8','9',
			'a','b','c','d','e','f','g','h','i','j',
			'k','l','m','n','o','p','q','r','s','t',
			'u','v','w','x','y','z'
		};
};

} // namespace NSGKUtils

#endif
