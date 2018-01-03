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

#define UTILS_COMPILATION 1

#include "functions.h"
#include "exception.h"

#undef UTILS_COMPILATION

namespace GLogiK
{

const std::string to_string(const char* s) {
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
};

} // namespace GLogiK

