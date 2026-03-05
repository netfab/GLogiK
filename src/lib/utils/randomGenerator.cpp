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

#include <algorithm>

#define UTILS_COMPILATION 1

#include "exception.hpp"
#include "randomGenerator.hpp"

#undef UTILS_COMPILATION

namespace NSGKUtils
{

const std::vector<char> RandomGenerator::defaultCharset =
{
	'0','1','2','3','4','5','6','7','8','9',
	'a','b','c','d','e','f','g','h','i','j',
	'k','l','m','n','o','p','q','r','s','t',
	'u','v','w','x','y','z'
};

RandomGenerator::RandomGenerator(const std::vector<char> & charset)
	: _rng(std::random_device{}())
{
	const std::size_t size = charset.size();

	if( size < 1 )
		throw GLogiKExcept("RandomGenerator : charset vector is empty !");

	_charset = charset;
	std::uniform_int_distribution<> dist(0, size-1);
	_dist = dist;
}

RandomGenerator::~RandomGenerator()
{
}

const std::string RandomGenerator::getString(std::size_t length)
{
	std::string ret(length,0);
	auto & c = _charset;
	auto & d = _dist;
	auto & r = _rng;
	auto randChar = [ c, &d, &r ](){ return c[d(r)]; };
	std::generate_n(ret.begin(), length, randChar);
	return ret;
}

} // namespace NSGKUtils

