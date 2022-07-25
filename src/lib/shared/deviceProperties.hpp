/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2020  Fabrice Delliaux <netbox253@gmail.com>
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

#include "include/DeviceID.hpp"
#include "include/LCDPluginProperties.hpp"

#include "glogik.hpp"
#include "GKeysBanksCapability.hpp"

namespace GLogiK
{

class BacklightCapability
{
	public:
		void setRGBBytes(const uint8_t r, const uint8_t g, const uint8_t b);
		void getRGBBytes(uint8_t & r, uint8_t & g, uint8_t & b) const;

	protected:
		BacklightCapability(void);
		virtual ~BacklightCapability(void) = 0;

		uint8_t _red;
		uint8_t _green;
		uint8_t _blue;

	private:
		friend class boost::serialization::access;

		template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
		{
			//if(version > 0)
			ar & _red;
			ar & _green;
			ar & _blue;
		}
};

class LCDScreenCapability
{
	public:
		static const LCDPluginsPropertiesArray_type _LCDPluginsPropertiesEmptyArray;

		const LCDPluginsPropertiesArray_type & getLCDPluginsProperties(void) const;
		void setLCDPluginsProperties(const LCDPluginsPropertiesArray_type & props);

		const uint64_t getLCDPluginsMask1(void) const;
		void setLCDPluginsMask(
			const uint8_t maskID,
			const uint64_t mask
		);

	protected:
		LCDScreenCapability(void);
		virtual ~LCDScreenCapability(void) = 0;

		uint64_t _LCDPluginsMask1;

	private:

		LCDPluginsPropertiesArray_type _LCDPluginsProperties;

		friend class boost::serialization::access;

		template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
		{
			ar & _LCDPluginsMask1;
		}
};

class clientDevice
	:	public BacklightCapability,
		public LCDScreenCapability,
		public DeviceID
{
	public:
		clientDevice(void);
		~clientDevice(void);

		const uint64_t getCapabilities(void) const;

		void setProperties(
			const std::string & vendor,
			const std::string & product,
			const std::string & name,
			const uint64_t capabilities
		);

	protected:
		uint64_t _capabilities;

	private:

		friend class boost::serialization::access;

		template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
		{
			//if(version > 0)
			ar & boost::serialization::base_object<DeviceID>(*this);
			ar & boost::serialization::base_object<BacklightCapability>(*this);
			ar & boost::serialization::base_object<LCDScreenCapability>(*this);
		}
};

class DeviceProperties
	:	public clientDevice,
		public GKeysBanksCapability
{
	public:
		DeviceProperties(void);
		~DeviceProperties(void);

		const int getWatchDescriptor(void) const;
		void setWatchDescriptor(int wd);

		void setProperties(
			const std::string & vendor,
			const std::string & product,
			const std::string & name,
			const uint64_t capabilities)
		{
			clientDevice::setProperties(vendor, product, name, capabilities);
		}

		void setProperties(const DeviceProperties & dev);

	protected:
	private:
		int _watchedDescriptor;

		friend class boost::serialization::access;

		template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
		{
			//if(version > 0)
			ar & boost::serialization::base_object<clientDevice>(*this);
			ar & boost::serialization::base_object<GKeysBanksCapability>(*this);
		}
};

} // namespace GLogiK

//BOOST_CLASS_VERSION(GLogiK::DeviceProperties, 1)

#endif
