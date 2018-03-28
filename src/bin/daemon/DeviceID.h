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

#ifndef __GLOGIKD_DEVICE_ID_H__
#define __GLOGIKD_DEVICE_ID_H__

#include <cstdint>
#include <string>

namespace GLogiK
{

class DeviceID {
	public:
		DeviceID(
			const std::string & n,
			const std::string & v,
			const std::string & p
		)	:	name(n), vendor_id(v), product_id(p) {};
		~DeviceID(void) = default;

		const std::string & getName(void) const { return this->name; }
		const std::string & getVendorID(void) const { return this->vendor_id; }
		const std::string & getProductID(void) const { return this->product_id; }

	protected:
		DeviceID(void) = default;

	private:
		friend class USBDevice;

		std::string name;
		std::string vendor_id;
		std::string product_id;
};

class BusNumDeviceID
	:	public DeviceID
{
	public:
		const uint8_t getBus(void) const { return this->bus; }
		const uint8_t getNum(void) const { return this->num; }

	protected:
		BusNumDeviceID(void) = default;
		BusNumDeviceID(
			const std::string & n,
			const std::string & v,
			const std::string & p,
			const uint8_t b,
			const uint8_t nu
		)	:	DeviceID(n,v,p), bus(b), num(nu) {};
		~BusNumDeviceID(void) = default;

	private:
		friend class USBDevice;

		uint8_t bus;
		uint8_t num;
};

} // namespace GLogiK

#endif
