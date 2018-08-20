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

#ifndef SRC_LIB_SHARED_DEVICE_PROPERTIES_HPP_
#define SRC_LIB_SHARED_DEVICE_PROPERTIES_HPP_

#include <cstdint>

#include <string>
#include <map>
#include <utility>
#include <initializer_list>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/map.hpp>
//#include <boost/serialization/version.hpp>

#include "glogik.h"
#include "macrosBanks.h"

namespace GLogiK
{

class DeviceProperties : public MacrosBanks
{
	public:
		DeviceProperties(void);
		~DeviceProperties(void);

		const std::string & getVendor(void) const;
		const std::string & getModel(void) const;
		const uint64_t getCapabilities(void) const;
		const std::string getMediaCommand(const std::string & mediaEvent) const;
		const std::map<const std::string, std::string> & getMediaCommands(void) const;

		const std::string & getConfigFileName(void) const;
		void setConfigFileName(const std::string & fileName);

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
		std::string _vendor;
		std::string _model;
		uint64_t _capabilities;
		std::string _configFileName;
		int _watchedDescriptor;

		uint8_t _backlightRed;
		uint8_t _backlightGreen;
		uint8_t _backlightBlue;

		std::initializer_list<std::pair<const std::string, std::string>> _il = {
			{XF86_AUDIO_NEXT, ""},
			{XF86_AUDIO_PREV, ""},
			{XF86_AUDIO_STOP, ""},
			{XF86_AUDIO_PLAY, ""},
			{XF86_AUDIO_MUTE, ""},
			{XF86_AUDIO_RAISE_VOLUME, ""},
			{XF86_AUDIO_LOWER_VOLUME, ""}
		};
		std::map<const std::string, std::string> _mediaCommands{_il};

		friend class boost::serialization::access;

		template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
		{
			//if(version > 0)
			ar & _vendor;
			ar & _model;
			ar & _backlightRed;
			ar & _backlightGreen;
			ar & _backlightBlue;
			ar & _mediaCommands;

			// serialize base class information
			ar & boost::serialization::base_object<MacrosBanks>(*this);
		}
};

} // namespace GLogiK

//BOOST_CLASS_VERSION(GLogiK::DeviceProperties, 1)

#endif
