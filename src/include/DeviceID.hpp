/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_INCLUDE_DEVICE_ID_HPP_
#define SRC_INCLUDE_DEVICE_ID_HPP_

#include <string>
#include <map>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/access.hpp>

#define DEVICE_ID_NUM_PROPERTIES 5

namespace GLogiK
{

class DeviceID
{
	public:
		DeviceID(
			const std::string & vendor, const std::string & product, const std::string & name,
			const std::string & status, const std::string & path) :
				_vendor(vendor), _product(product), _name(name),
				_status(status), _filePath(path)
			{}
		~DeviceID(void) = default;

		const std::string & getVendor(void) const { return _vendor; };
		const std::string & getProduct(void) const { return _product; };
		const std::string & getName(void) const { return _name; };
		const std::string & getStatus(void) const { return _status; };
		const std::string & getConfigFilePath(void) const { return _filePath; };

		void setVendor(const std::string & vendor) { _vendor = vendor; };
		void setProduct(const std::string & product) { _product = product; };
		void setName(const std::string & name) { _name = name; };
		void setStatus(const std::string & status) { _status = status; };
		void setConfigFilePath(const std::string & path) { _filePath = path; };

	protected:

	private:
		std::string _vendor;
		std::string _product;
		std::string _name;
		std::string _status;
		std::string _filePath;

		friend class boost::serialization::access;

		template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
		{
			ar & _vendor;
			ar & _product;
			ar & _name;
		}
};

typedef std::map<std::string, DeviceID> DevicesMap_type;

} // namespace GLogiK

#endif

