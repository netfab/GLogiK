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

#ifndef __GLOGIK_DEVICE_PROPERTIES_H__
#define __GLOGIK_DEVICE_PROPERTIES_H__

#include <cstdint>

#include <string>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/access.hpp>
//#include <boost/serialization/version.hpp>

#include "macrosBanks.h"

namespace GLogiK
{

class DeviceProperties : public MacrosBanks
{
	public:
		DeviceProperties(void);
		~DeviceProperties(void);

		const std::string & getVendor(void) const { return this->vendor_; }
		const std::string & getModel(void) const { return this->model_; }
		const std::string & getConfFile(void) const { return this->conf_file_; }

		void setVendor(const std::string & vendor);
		void setModel(const std::string & model);
		void setConfFile(const std::string & conf_file);

		void setBLColor_R(uint8_t r) { this->backlight_color_R_ = r & 0xFF; }
		void setBLColor_G(uint8_t g) { this->backlight_color_G_ = g & 0xFF; }
		void setBLColor_B(uint8_t b) { this->backlight_color_B_ = b & 0xFF; }

		const uint8_t & getBLColor_R(void) const { return this->backlight_color_R_; }
		const uint8_t & getBLColor_G(void) const { return this->backlight_color_G_; }
		const uint8_t & getBLColor_B(void) const { return this->backlight_color_B_; }

	protected:

	private:
		std::string vendor_;
		std::string model_;
		std::string conf_file_;
		uint8_t backlight_color_R_;
		uint8_t backlight_color_G_;
		uint8_t backlight_color_B_;

		friend class boost::serialization::access;

		template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
		{
			//if(version > 0)
			ar & this->vendor_;
			ar & this->model_;
			ar & this->backlight_color_R_;
			ar & this->backlight_color_G_;
			ar & this->backlight_color_B_;

			// serialize base class information
			ar & boost::serialization::base_object<MacrosBanks>(*this);
		}
};

} // namespace GLogiK

//BOOST_CLASS_VERSION(GLogiK::DeviceProperties, 1)

#endif
