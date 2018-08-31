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

#ifndef SRC_BIN_DAEMON_DEVICE_ID_HPP_
#define SRC_BIN_DAEMON_DEVICE_ID_HPP_

#include <cstdint>
#include <string>

namespace GLogiK
{

class DeviceID {
	public:
		DeviceID(
			const std::string & n,
			const std::string & v,
			const std::string & p,
			const uint64_t c
		)	:	_name(n), _vendorID(v), _productID(p), _capabilities(c) {};
		~DeviceID(void) = default;

		const std::string & getName(void) const { return _name; }
		const std::string & getVendorID(void) const { return _vendorID; }
		const std::string & getProductID(void) const { return _productID; }
		const uint64_t & getCapabilities(void) const { return _capabilities; }

	protected:
		DeviceID(void) = default;

	private:
		friend class USBDevice;

		std::string _name;
		std::string _vendorID;
		std::string _productID;
		uint64_t _capabilities;
};

class BusNumDeviceID
	:	public DeviceID
{
	public:
		const uint8_t getBus(void) const { return _bus; }
		const uint8_t getNum(void) const { return _num; }

		static const std::string getDeviceID(const uint8_t bus, const uint8_t num)
		{
			std::string devID("[b");
			devID += std::to_string(bus);
			devID += 'd';
			devID += std::to_string(num);
			devID += ']';
			return devID;
		};

		const std::string & getID(void) const
		{
			return _devID;
		}

	protected:
		BusNumDeviceID(void) = default;
		BusNumDeviceID(
			const std::string & n,
			const std::string & v,
			const std::string & p,
			const uint64_t c,
			const uint8_t b,
			const uint8_t num
		)	:	DeviceID(n,v,p,c),
				_bus(b),
				_num(num),
				_devID(BusNumDeviceID::getDeviceID(b, num)) {};
		~BusNumDeviceID(void) = default;

	private:
		friend class USBDevice;

		uint8_t _bus;
		uint8_t _num;
		std::string _devID; /* [b1d3] */
};

} // namespace GLogiK

#endif
