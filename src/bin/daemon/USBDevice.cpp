/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2022  Fabrice Delliaux <netbox253@gmail.com>
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

#include <iterator>
#include <algorithm>
#include <new>

#include "lib/utils/utils.hpp"

#include "USBDevice.hpp"

namespace GLogiK
{

using namespace NSGKUtils;


USBDevice::USBDevice(const USBDeviceID & device)
		:	USBDeviceID(device),
			_pressedRKeysMask(0),
			_LCDPluginsMask1(0),
			_pMacrosManager(nullptr),
			_pLCDPluginsManager(nullptr),
#if GKLIBUSB
			_pUSBDevice(nullptr),
			_pUSBDeviceHandle(nullptr),
#elif GKHIDAPI
			_pHIDDevice(nullptr),
#endif
			_MxKeysLedsMask(0),
			_exitMacroRecordMode(false),
			_threadsStatus(true),
			_USBRequestsStatus(true),
			_lastKeysInterruptTransferLength(0),
			_lastLCDInterruptTransferLength(0),
#if GKLIBUSB
			_fatalErrors(0),
			_keysEndpoint(0),
			_LCDEndpoint(0),
#elif GKHIDAPI
			_fatalErrors(0),
#endif
			_GKeyID(GKeysID::GKEY_G0)
{
	std::fill_n(_pressedKeys, KEYS_BUFFER_LENGTH, 0);
	std::fill_n(_previousPressedKeys, KEYS_BUFFER_LENGTH, 0);
	this->setRGBBytes(0xFF, 0xFF, 0xFF);
	_lastTimePoint = std::chrono::steady_clock::now();
}

void USBDevice::operator=(const USBDevice& dev)
{
	_vendor			= dev._vendor;
	_product		= dev._product;
	_name			= dev._name;
	_fullname		= dev._fullname;
	_vendorID		= dev._vendorID;
	_productID		= dev._productID;
	_capabilities	= dev._capabilities;
	_bus			= dev._bus;
	_num			= dev._num;
	_devID			= dev._devID;

	_devnode		= dev._devnode;
	_serial			= dev._serial;
	_usec			= dev._usec;
	_driverID		= dev._driverID;

	/* USB_INTERFACE_DESCRIPTOR */
	_bConfigurationValue	= dev._bConfigurationValue;
	_bInterfaceNumber		= dev._bInterfaceNumber;
	_bAlternateSetting		= dev._bAlternateSetting;
	_bNumEndpoints			= dev._bNumEndpoints;

	_keysInterruptBufferMaxLength	= dev._keysInterruptBufferMaxLength;
	_MacrosKeysLength				= dev._MacrosKeysLength;
	_MediaKeysLength				= dev._MediaKeysLength;
	_LCDKeysLength					= dev._LCDKeysLength;

	/* end friendship members */

	/* public */
	_fatalErrors					= dev._fatalErrors;
	_keysThreadID					= dev._keysThreadID;
	_LCDThreadID					= dev._LCDThreadID;
	_pressedRKeysMask				= dev._pressedRKeysMask;
	_MxKeysLedsMask					= static_cast<uint8_t>(dev._MxKeysLedsMask);
	_exitMacroRecordMode			= static_cast<bool>(dev._exitMacroRecordMode);
	_LCDPluginsMask1				= dev._LCDPluginsMask1;
	std::copy(
		std::begin(dev._pressedKeys),
		std::end(dev._pressedKeys),
		std::begin(_pressedKeys)
	);
	std::copy(
		std::begin(dev._previousPressedKeys),
		std::end(dev._previousPressedKeys),
		std::begin(_previousPressedKeys)
	);
	_GKeyID				= dev._GKeyID;
	_mediaKey			= dev._mediaKey;
	_newMacro			= dev._newMacro;
	_lastTimePoint		= dev._lastTimePoint;

	/* private */
	_pMacrosManager					= dev._pMacrosManager;
	_pLCDPluginsManager				= dev._pLCDPluginsManager;

	this->setRGBBytes(dev._RGB[0], dev._RGB[1], dev._RGB[2]);
	_lastKeysInterruptTransferLength	= dev._lastKeysInterruptTransferLength;
	_lastLCDInterruptTransferLength		= dev._lastLCDInterruptTransferLength;
	_threadsStatus		= static_cast<bool>(dev._threadsStatus);
	_USBRequestsStatus	= static_cast<bool>(dev._USBRequestsStatus);

#if GKLIBUSB
	_pUSBDevice			= dev._pUSBDevice;
	_pUSBDeviceHandle	= dev._pUSBDeviceHandle;

	_keysEndpoint		= dev._keysEndpoint;
	_LCDEndpoint		= dev._LCDEndpoint;

	_toRelease			= dev._toRelease;
	_toAttach			= dev._toAttach;
#elif GKHIDAPI
	_pHIDDevice			= dev._pHIDDevice;
#endif
}

void USBDevice::destroyMacrosManager(void) noexcept {
	if( _pMacrosManager ) {
		delete _pMacrosManager;
		_pMacrosManager = nullptr;
	}
}

void USBDevice::destroyLCDPluginsManager(void) noexcept
{
	if( _pLCDPluginsManager ) {
		delete _pLCDPluginsManager;
		_pLCDPluginsManager = nullptr;
	}
}

void USBDevice::setRGBBytes(const uint8_t r, const uint8_t g, const uint8_t b)
{
	_RGB[0] = r;
	_RGB[1] = g;
	_RGB[2] = b;
}

void USBDevice::getRGBBytes(uint8_t & r, uint8_t & g, uint8_t & b) const
{
	r = _RGB[0];
	g = _RGB[1];
	b = _RGB[2];
}

} // namespace GLogiK

