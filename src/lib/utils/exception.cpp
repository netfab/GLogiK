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

#define UTILS_COMPILATION 1

#include "exception.hpp"

#undef UTILS_COMPILATION

namespace NSGKUtils
{

GLogiKFatalError::GLogiKFatalError( const std::string& msg ) : message(msg) {}
GLogiKFatalError::~GLogiKFatalError( void ) throw() {}

const char* GLogiKFatalError::what( void ) const throw()
{
    return message.c_str();
}

GLogiKExcept::GLogiKExcept( const std::string& msg ) : message(msg) {}
GLogiKExcept::~GLogiKExcept( void ) throw() {}

const char* GLogiKExcept::what( void ) const throw()
{
    return message.c_str();
}

/*
DisplayHelp::DisplayHelp( const std::string& msg ) : message(msg) {}
DisplayHelp::~DisplayHelp( void ) throw() {}

const char* DisplayHelp::what( void ) const throw()
{
    return message.c_str();
}
*/

} // namespace NSGKUtils

