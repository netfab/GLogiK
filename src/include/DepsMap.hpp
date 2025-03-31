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

#ifndef SRC_INCLUDE_DEPENDENCIES_MAP_HPP_
#define SRC_INCLUDE_DEPENDENCIES_MAP_HPP_

#include <cstdint>

#include <map>
#include <vector>
#include <string>

namespace GLogiK
{

enum class GKBinary : uint8_t
{
	GK_INVALID = 0,
	GK_DAEMON,
	GK_DESKTOP_SERVICE,
	GK_GUI_QT,
};

class GKDependency
{
	public:
		GKDependency(
			const std::string & dependency,
			const std::string & compileTimeVersion,
			const std::string & runTimeVersion = "-") :
				_dependency(dependency),
				_compileTimeVersion(compileTimeVersion),
				_runTimeVersion(runTimeVersion) {}
		GKDependency(void) = delete;
		~GKDependency(void) = default;

		const std::string & getDependency(void) const { return _dependency; }
		const std::string & getCompileTimeVersion(void) const { return _compileTimeVersion; }
		const std::string & getRunTimeVersion(void) const { return _runTimeVersion; }

	protected:
	private:
		std::string _dependency;
		std::string _compileTimeVersion;
		std::string _runTimeVersion;
};

typedef std::vector<GKDependency> GKDepsArray_type;
typedef std::map<GKBinary, GKDepsArray_type> GKDepsMap_type;

} // namespace GLogiK

#endif
