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

#include <cstddef> // std::size_t

#include <algorithm>

#define UTILS_COMPILATION 1

#include "exception.hpp"
#include "randomGenerator.hpp"

#undef UTILS_COMPILATION

namespace NSGKUtils
{

RandomGenerator::RandomGenerator()
	:	_rng(std::random_device{}())
{
	if(RandomGenerator::defaultCharset.empty())
		throw GLogiKExcept("RandomGenerator : charset array is empty !");

	std::uniform_int_distribution<std::size_t> dist(0, RandomGenerator::defaultCharset.size()-1);
	_dist = dist;
}

RandomGenerator::~RandomGenerator()
{
}

std::string RandomGenerator::getString(const std::size_t length)
{
	std::string ret(length,0);

	// lambda
	auto randChar = [this]()
	{
		return RandomGenerator::defaultCharset[_dist(_rng)];
	};

	std::generate_n(ret.begin(), length, randChar);

	return ret;
}

} // namespace NSGKUtils

