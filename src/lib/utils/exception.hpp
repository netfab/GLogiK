/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_LIB_UTILS_GLOGIK_EXCEPTION_HPP_
#define SRC_LIB_UTILS_GLOGIK_EXCEPTION_HPP_

#if !defined (UTILS_INSIDE_UTILS_H) && !defined (UTILS_COMPILATION)
#error "Only "utils/utils.hpp" can be included directly, this file may disappear or change contents."
#endif

#include <exception>
#include <string>

namespace NSGKUtils
{

class GLogiKExcept : public std::exception
{
	public :
		GLogiKExcept( const std::string& msg = "" );

		virtual ~GLogiKExcept( void ) throw();
		virtual const char* what( void ) const throw();

	protected :
		std::string message;
};

class EmptyContainer : public GLogiKExcept
{
	public:
		EmptyContainer( const std::string& msg = "" ) : GLogiKExcept(msg) {};
		virtual ~EmptyContainer( void ) throw() {};
};

class GKDBusRemoteCallNoReply : public GLogiKExcept
{
	public:
		GKDBusRemoteCallNoReply( const std::string& msg = "" ) : GLogiKExcept(msg) {};
		virtual ~GKDBusRemoteCallNoReply( void ) throw() {};
};

class GKDBusMessageWrongBuild : public GLogiKExcept
{
	public:
		GKDBusMessageWrongBuild( const std::string& msg = "" ) : GLogiKExcept(msg) {};
		virtual ~GKDBusMessageWrongBuild( void ) throw() {};
};

class GLogiKBadAlloc : public GLogiKExcept
{
	public:
		GLogiKBadAlloc(const std::string & msg = "caught bad_alloc, do not like that") : GLogiKExcept(msg) {};
		virtual ~GLogiKBadAlloc( void ) throw() {};
};

} // namespace NSGKUtils

#endif

