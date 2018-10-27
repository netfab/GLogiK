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

#ifndef SRC_LIB_SHARED_DEVICE_FILE_HPP_
#define SRC_LIB_SHARED_DEVICE_FILE_HPP_

#include <string>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/access.hpp>

namespace GLogiK
{

class DeviceFile
{
	public:
		DeviceFile();
		~DeviceFile();

		const std::string & getStatus(void) const;
		const std::string & getVendor(void) const;
		const std::string & getModel(void) const;
		const std::string & getConfigFilePath(void) const;

		void setStatus(const std::string & status);
		void setVendor(const std::string & vendor);
		void setModel(const std::string & model);
		void setConfigFilePath(const std::string & path);

	protected:

	private:
		std::string _status;
		std::string _vendor;
		std::string _model;
		std::string _filePath;

		friend class boost::serialization::access;

		template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
		{
			ar & _vendor;
			ar & _model;
		}
};

} // namespace GLogiK

#endif

