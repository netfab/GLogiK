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

#include <new>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <thread>

#define UTILS_COMPILATION 1

#include "functions.h"
#include "exception.h"

#undef UTILS_COMPILATION

namespace NSGKUtils
{

const std::string to_string(const char* s)
{
	if(s == nullptr)
		return "";

	try {
		const std::string ret(s);
		return ret;
	}
	catch (const std::bad_alloc& e) {
		throw GLogiKBadAlloc( e.what() );
	}
	catch (const std::length_error& e) {
		std::string error("string length error : ");
		error += e.what();
		throw GLogiKExcept(error);
	}
}

const unsigned int to_uint(const std::string & s)
{
	unsigned int ret = 0;
	try {
		ret = std::stoi(s);
	}
	catch (const std::invalid_argument& ia) {
		throw GLogiKExcept("stoi invalid argument");
	}
	catch (const std::out_of_range& oor) {
		throw GLogiKExcept("stoi out of range");
	}

	return ret;
}

const unsigned long long to_ull(const std::string & s)
{
	unsigned long long ret = 0;
	try {
		ret = std::stoull(s);
	}
	catch (const std::invalid_argument& ia) {
		throw GLogiKExcept("stoull invalid argument");
	}
	catch (const std::out_of_range& oor) {
		throw GLogiKExcept("stoull out of range");
	}

	return ret;
}

const std::string getHexRGB(
	const uint8_t red,
	const uint8_t green,
	const uint8_t blue)
{
	std::ostringstream ret("", std::ios_base::app);
	ret << std::hex << std::setfill('0')
		<< std::setw(2) << to_uint(red) << " "
		<< std::setw(2) << to_uint(green) << " "
		<< std::setw(2) << to_uint(blue);
	return ret.str();
}

void yield_for(std::chrono::microseconds us)
{
	auto start = std::chrono::high_resolution_clock::now();
	auto end = start + us;
	do {
		std::this_thread::yield();
	} while (std::chrono::high_resolution_clock::now() < end);
}

} // namespace NSGKUtils

