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

#ifndef __GLOGIK_DEVICE_PROPERTIES_H__
#define __GLOGIK_DEVICE_PROPERTIES_H__

#include <string>

#include <boost/serialization/access.hpp>
//#include <boost/serialization/version.hpp>

#include "macrosBanks.h"

namespace GLogiK
{

enum class DeviceState : uint8_t
{
	STATE_STARTED = 0,
	STATE_STOPPED,
	STATE_UNKNOWN,
};

class DeviceProperties : public MacrosBanks
{
	public:
		DeviceProperties(void);
		~DeviceProperties(void);

		std::string vendor_;
		std::string model_;
		std::string conf_file_;

		/* accessors to purely logical state */
		const bool started(void) const;
		const bool stopped(void) const;
		void start(void);
		void stop(void);

	protected:

	private:
		DeviceState state_;

		friend class boost::serialization::access;

		template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
		{
			// TODO catch exception ?
			//if(version > 0)
			ar & this->vendor_;
			ar & this->model_;
		}
};

} // namespace GLogiK

//BOOST_CLASS_VERSION(GLogiK::DeviceProperties, 1)

#endif
