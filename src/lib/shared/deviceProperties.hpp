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

#ifndef SRC_LIB_SHARED_DEVICE_PROPERTIES_HPP_
#define SRC_LIB_SHARED_DEVICE_PROPERTIES_HPP_

#include <cstdint>

#include <string>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/access.hpp>
//#include <boost/serialization/version.hpp>

#include "glogik.hpp"
#include "macrosBanks.hpp"
#include "deviceFile.hpp"

namespace GLogiK
{

class DeviceProperties
	:	public MacrosBanks,
		public DeviceFile
{
	public:
		DeviceProperties(void);
		~DeviceProperties(void);

		const uint64_t getCapabilities(void) const;

		const uint64_t getLCDPluginsMask1(void) const;
		void setLCDPluginsMask(
			const uint8_t maskID,
			const uint64_t mask
		);

		const int getWatchDescriptor(void) const;
		void setWatchDescriptor(int wd);

		void setProperties(
			const std::string & vendor,
			const std::string & model,
			const uint64_t capabilities
		);
		void setProperties(const DeviceProperties & dev);

		void setRGBBytes(const uint8_t r, const uint8_t g, const uint8_t b);
		void getRGBBytes(uint8_t & r, uint8_t & g, uint8_t & b) const;

	protected:

	private:
		uint64_t _capabilities;
		int _watchedDescriptor;

		uint8_t _backlightRed;
		uint8_t _backlightGreen;
		uint8_t _backlightBlue;

		uint64_t _LCDPluginsMask1;

		friend class boost::serialization::access;

		template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
		{
			//if(version > 0)
			ar & boost::serialization::base_object<DeviceFile>(*this);

			ar & _backlightRed;
			ar & _backlightGreen;
			ar & _backlightBlue;
			ar & _LCDPluginsMask1;

			// serialize base class information
			ar & boost::serialization::base_object<MacrosBanks>(*this);
		}
};

} // namespace GLogiK

//BOOST_CLASS_VERSION(GLogiK::DeviceProperties, 1)

#endif
