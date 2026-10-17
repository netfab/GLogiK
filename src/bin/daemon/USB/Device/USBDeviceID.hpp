/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2026  Fabrice Delliaux <netbox253@gmail.com>
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

#pragma once

#include <cstdint>
#include <string>

namespace USB::Device
{

class USBDeviceID
{
	public:
		const std::string & getVendor() const
		{
			return _vendor;
		}
		const std::string & getProduct() const
		{
			return _product;
		}
		const std::string & getName() const
		{
			return _name;
		}
		const std::string & getFullName() const
		{
			return _fullname;
		};
		const std::string & getVendorID() const
		{
			return _vendorID;
		}
		const std::string & getProductID() const
		{
			return _productID;
		}
		const std::string & getDevnode() const
		{
			return _devnode;
		}
		const std::string & getDevpath() const
		{
			return _devpath;
		}
		const std::string & getUSec() const
		{
			return _usec;
		}

		std::uint64_t getCapabilities() const
		{
			return _capabilities;
		}

		std::uint16_t getDriverID() const
		{
			return _driverID;
		}

		std::uint8_t getBus() const
		{
			return _bus;
		}
		std::uint8_t getNum() const
		{
			return _num;
		}

		std::uint8_t getBConfigurationValue() const
		{
			return _bConfigurationValue;
		}
		std::uint8_t getBInterfaceNumber() const
		{
			return _bInterfaceNumber;
		}
		std::uint8_t getBAlternateSetting() const
		{
			return _bAlternateSetting;
		}
		std::uint8_t getBNumEndpoints() const
		{
			return _bNumEndpoints;
		}

		std::int8_t getKeysInterruptBufferMaxLength() const
		{
			return _keysInterruptBufferMaxLength;
		};

		std::int8_t getGKeysTransferLength() const
		{
			return _GKeysTransferLength;
		}
		std::int8_t getMediaKeysTransferLength() const
		{
			return _MediaKeysTransferLength;
		}
		std::int8_t getLCDKeysTransferLength() const
		{
			return _LCDKeysTransferLength;
		}

		static std::string getDeviceID(std::uint8_t bus, std::uint8_t num)
		{
			std::string devID("[b");
			devID += std::to_string(bus);
			devID += 'd';
			devID += std::to_string(num);
			devID += ']';
			return devID;
		};

		const std::string & getID() const
		{
			return _devID;
		}

		USBDeviceID() = default;
		USBDeviceID(
			const std::string & vendor,
			const std::string & product,
			const std::string & name,
			const std::string & vendorID,
			const std::string & productID,
			std::uint64_t capabilities,
			std::uint8_t bConfigurationValue,
			std::uint8_t bInterfaceNumber,
			std::uint8_t bAlternateSetting,
			std::uint8_t bNumEndpoints,
			std::int8_t bufferMaxLength,
			std::int8_t GKeysTransferLength,
			std::int8_t mediaKeysTransferLength,
			std::int8_t LCDKeysTransferLength
		);
		USBDeviceID(
			const USBDeviceID & device,
			const std::string & devnode,
			const std::string & devpath,
			const std::string & serial,
			const std::string & usec,
			std::uint16_t driverID,
			std::uint8_t bus,
			std::uint8_t num
		);
		USBDeviceID(const USBDeviceID & device) = default;
		~USBDeviceID() = default;

	protected:

	private:
		friend class USBDevice;

		std::string _vendor;
		std::string _product;
		std::string _name;
		std::string _fullname;
		std::string _vendorID;
		std::string _productID;
		std::string _devID; /* [b1d3] */

		std::string _devnode;
		std::string _devpath;
		std::string _serial;
		std::string _usec;

		std::uint64_t _capabilities;

		std::uint16_t _driverID;

		std::uint8_t _bus;
		std::uint8_t _num;

		/* USB_INTERFACE_DESCRIPTOR */
		std::uint8_t _bConfigurationValue;
		std::uint8_t _bInterfaceNumber;
		std::uint8_t _bAlternateSetting;
		std::uint8_t _bNumEndpoints;

		std::int8_t _keysInterruptBufferMaxLength;

		std::int8_t _GKeysTransferLength;
		std::int8_t _MediaKeysTransferLength;
		std::int8_t _LCDKeysTransferLength;
};

} // namespace USB::Device
