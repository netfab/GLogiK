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

#ifndef SRC_BIN_DAEMON_USB_DEVICE_ID_HPP_
#define SRC_BIN_DAEMON_USB_DEVICE_ID_HPP_

#include <cstdint>
#include <string>

#define KEYS_BUFFER_LENGTH 16

namespace GLogiK
{

class USBDeviceID
{
	public:
		const std::string & getVendor(void) const { return _vendor; }
		const std::string & getProduct(void) const { return _product; }
		const std::string & getVendorID(void) const { return _vendorID; }
		const std::string & getProductID(void) const { return _productID; }
		const std::string & getDevnode(void) const { return _devnode; }
		const std::string & getUSec(void) const { return _usec; }

		const uint64_t getCapabilities(void) const { return _capabilities; }

		const uint16_t getDriverID(void) const { return _driverID; }

		const uint8_t getBus(void) const { return _bus; }
		const uint8_t getNum(void) const { return _num; }

		const uint8_t getBConfigurationValue(void) const { return _bConfigurationValue; }
		const uint8_t getBInterfaceNumber(void) const { return _bInterfaceNumber; }
		const uint8_t getBAlternateSetting(void) const { return _bAlternateSetting; }
		const uint8_t getBNumEndpoints(void) const { return _bNumEndpoints; }

		const int8_t getKeysInterruptBufferMaxLength(void) const
		{
			return _keysInterruptBufferMaxLength;
		};

		const int8_t getMacrosKeysLength(void) const { return _MacrosKeysLength; }
		const int8_t getMediaKeysLength(void) const { return _MediaKeysLength; }
		const int8_t getLCDKeysLength(void) const { return _LCDKeysLength; }

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

		USBDeviceID(void) = default;
		USBDeviceID(
			const std::string & vendor,
			const std::string & product,
			const std::string & vendorID,
			const std::string & productID,
			const uint64_t capabilities,
			const uint8_t bConfigurationValue,
			const uint8_t bInterfaceNumber,
			const uint8_t bAlternateSetting,
			const uint8_t bNumEndpoints,
			const int8_t bufferMaxLength,
			const int8_t macrosKeysLength,
			const int8_t mediaKeysLength,
			const int8_t LCDKeysLength
		);
		USBDeviceID(
			const USBDeviceID & device,
			const std::string & node,
			const std::string & serial,
			const std::string & usec,
			const uint16_t driverID,
			const uint8_t bus,
			const uint8_t num
		);
		USBDeviceID(const USBDeviceID & device) = default;
		~USBDeviceID(void) = default;

	protected:

	private:
		friend class USBDevice;

		std::string _vendor;
		std::string _product;
		std::string _vendorID;
		std::string _productID;
		std::string _devID; /* [b1d3] */

		std::string _devnode;
		std::string _serial;
		std::string _usec;

		uint64_t _capabilities;

		uint16_t _driverID;

		uint8_t _bus;
		uint8_t _num;

		/* USB_INTERFACE_DESCRIPTOR */
		uint8_t _bConfigurationValue;
		uint8_t _bInterfaceNumber;
		uint8_t _bAlternateSetting;
		uint8_t _bNumEndpoints;

		int8_t _keysInterruptBufferMaxLength;

		int8_t _MacrosKeysLength;
		int8_t _MediaKeysLength;
		int8_t _LCDKeysLength;
};

} // namespace GLogiK

#endif
