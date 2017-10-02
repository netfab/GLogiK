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

#ifndef __GLOGIK_EXCEPTION_H__
#define __GLOGIK_EXCEPTION_H__

#if !defined (UTILS_INSIDE_UTILS_H) && !defined (UTILS_COMPILATION)
#error "Only "utils/utils.h" can be included directly, this file may disappear or change contents."
#endif

#include <exception>
#include <string>

namespace GLogiK
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

class DisplayHelp : public std::exception
{
	public :
		DisplayHelp( const std::string& msg = "" );

		virtual ~DisplayHelp( void ) throw();
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

class GKDBusOOMWrongBuild : public GLogiKExcept
{
	public:
		GKDBusOOMWrongBuild( const std::string& msg = "" ) : GLogiKExcept(msg) {};
		virtual ~GKDBusOOMWrongBuild( void ) throw() {};
};

} // namespace GLogiK

#endif

