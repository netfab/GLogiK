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

#include <config.h>
#include <algorithm>

#define UTILS_COMPILATION 1

#include "log.h"
#include "exception.h"
#include "randomGenerator.h"

#undef UTILS_COMPILATION

namespace GLogiK
{

RandomGenerator::RandomGenerator(const std::vector<char> & charset)
	: rng_(std::random_device{}())
{
	size_t size = charset.size();
	if( size < 1 ) {
#if DEBUGGING_ON
		LOG(ERROR) << "charset vector is empty";
#endif
		throw GLogiKExcept("RandomGenerator : charset vector is empty !");
	}

	this->charset_ = charset;
	std::uniform_int_distribution<> dist(0, size-1);
	this->dist_ = dist;
}

RandomGenerator::~RandomGenerator() {
}

std::string RandomGenerator::getString(size_t length) {
	std::string ret(length,0);
	auto & c = this->charset_;
	auto & d = this->dist_;
	auto & r = this->rng_;
	auto randChar = [ c, &d, &r ](){ return c[d(r)];	};
	std::generate_n(ret.begin(), length, randChar);
	return ret;
}

} // namespace GLogiK

