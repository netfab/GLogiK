/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2019  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_INCLUDE_LCD_PLUGIN_PROPERTIES_HPP_
#define SRC_INCLUDE_LCD_PLUGIN_PROPERTIES_HPP_

#include <cstdint>
#include <vector>
#include <string>

namespace GLogiK
{

class LCDPluginProperties
{
	public:
		LCDPluginProperties()
			:	_ID(0),
				_name("unknown"),
				_desc("unknown") {};
		LCDPluginProperties(const uint64_t i, const std::string & n, const std::string & d)
			{ _ID = i; _name = n; _desc = d; }
		~LCDPluginProperties() = default;

		const uint64_t getID(void) const { return _ID; }
		const std::string getName(void) const { return _name; }
		const std::string getDesc(void) const { return _desc; }

		void setID(const uint64_t id) { _ID = id; }
		void setName(const std::string & name) { _name = name; }
		void setDesc(const std::string & desc) { _desc = desc; }

	private:
		uint64_t _ID;
		std::string _name;
		std::string _desc;
};

typedef std::vector<LCDPluginProperties> LCDPluginsPropertiesArray_type;

} // namespace GLogiK

#endif
