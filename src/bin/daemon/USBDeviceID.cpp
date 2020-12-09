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

#include "lib/utils/utils.hpp"

#include "USBDeviceID.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

USBDeviceID::USBDeviceID(
			const std::string & vendor,
			const std::string & product,
			const std::string & name,
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
		)	:	_vendor(vendor),
				_product(product),
				_name(name),
				_vendorID(vendorID),
				_productID(productID),
				_capabilities(capabilities),
				_bConfigurationValue(bConfigurationValue),
				_bInterfaceNumber(bInterfaceNumber),
				_bAlternateSetting(bAlternateSetting),
				_bNumEndpoints(bNumEndpoints),
				_MacrosKeysLength(macrosKeysLength),
				_MediaKeysLength(mediaKeysLength),
				_LCDKeysLength(LCDKeysLength)
{
	_devpath = _devnode = _serial = _usec = "";
	_state = USBDeviceState::USBDEVCLEAN;
	_driverID = _bus = _num = 0;

	_fullname  = vendor;
	_fullname += " ";
	_fullname += product;
	_fullname += " ";
	_fullname += name;

	_keysInterruptBufferMaxLength = bufferMaxLength;

	if( bufferMaxLength > KEYS_BUFFER_LENGTH ) {
		GKSysLog(LOG_WARNING, WARNING, "interrupt read length too large, set it to max buffer length");
		_keysInterruptBufferMaxLength = KEYS_BUFFER_LENGTH;
	}
}

USBDeviceID::USBDeviceID(
	const USBDeviceID & device,
	const std::string & devnode,
	const std::string & devpath,
	const std::string & serial,
	const std::string & usec,
	const uint16_t driverID,
	const uint8_t bus,
	const uint8_t num
	)	:	_devID(USBDeviceID::getDeviceID(bus, num)),
			_devnode(devnode),
			_devpath(devpath),
			_serial(serial),
			_usec(usec),
			_driverID(driverID),
			_bus(bus),
			_num(num)
{
	_vendor							= device._vendor;
	_product						= device._product;
	_name							= device._name;
	_fullname						= device._fullname;
	_vendorID						= device._vendorID;
	_productID						= device._productID;
	_capabilities					= device._capabilities;
	_state							= device._state;
	_bConfigurationValue			= device._bConfigurationValue;
	_bInterfaceNumber				= device._bInterfaceNumber;
	_bAlternateSetting				= device._bAlternateSetting;
	_bNumEndpoints					= device._bNumEndpoints;
	_keysInterruptBufferMaxLength	= device._keysInterruptBufferMaxLength;
	_MacrosKeysLength				= device._MacrosKeysLength;
	_MediaKeysLength				= device._MediaKeysLength;
	_LCDKeysLength					= device._LCDKeysLength;
}

void USBDeviceID::setDirtyFlag(void)
{
	_state = USBDeviceState::USBDEVDIRTY;
}

void USBDeviceID::setResetFlag(void)
{
	_state = USBDeviceState::USBDEVRESET;
}

const bool USBDeviceID::isDirty(void) const
{
	return (_state == USBDeviceState::USBDEVDIRTY);
}

} // namespace GLogiK

