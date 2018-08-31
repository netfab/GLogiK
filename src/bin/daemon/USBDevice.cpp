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

#include <iterator>
#include <algorithm>
#include <new>

#include "lib/utils/utils.hpp"

#include "USBDevice.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

USBDevice::USBDevice(
	const std::string & name,
	const std::string & vendorID,
	const std::string & productID,
	const uint64_t capabilities,
	uint8_t bus,
	uint8_t num,
	const std::string & stringID)
		:	BusNumDeviceID(name, vendorID, productID, capabilities, bus, num),
			_fatalErrors(0),
			_pMacrosManager(nullptr),
			_pressedRKeysMask(0),
			_banksLedsMask(0),
			_exitMacroRecordMode(false),
			_stringID(stringID),
			_lastKeysInterruptTransferLength(0),
			_lastLCDInterruptTransferLength(0),
			_keysEndpoint(0),
			_pUSBDevice(nullptr),
			_pUSBDeviceHandle(nullptr),
			_threadsStatus(true)
{
	std::fill_n(_pressedKeys, KEYS_BUFFER_LENGTH, 0);
	std::fill_n(_previousPressedKeys, KEYS_BUFFER_LENGTH, 0);
	this->setRGBBytes(0xFF, 0xFF, 0xFF);
}

void USBDevice::operator=(const USBDevice& dev)
{
	_name			= dev.getName();
	_vendorID		= dev.getVendorID();
	_productID		= dev.getProductID();
	_capabilities	= dev.getCapabilities();
	_bus			= dev.getBus();
	_num			= dev.getNum();
	/* end friendship members */

	/* public */
	_fatalErrors					= dev._fatalErrors;
	_keysThreadID					= dev._keysThreadID;
	_LCDThreadID					= dev._LCDThreadID;
	_pMacrosManager					= dev._pMacrosManager;
	_pressedRKeysMask				= dev._pressedRKeysMask;
	_banksLedsMask					= static_cast<uint8_t>(dev._banksLedsMask);
	_exitMacroRecordMode			= static_cast<bool>(dev._exitMacroRecordMode);
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
	_macroKey			= dev._macroKey;
	_mediaKey			= dev._mediaKey;
	_newMacro			= dev._newMacro;
	_lastTimePoint		= dev._lastTimePoint;

	/* private */
	_stringID			= dev.getStringID();
	this->setRGBBytes(dev._RGB[0], dev._RGB[1], dev._RGB[2]);
	_lastKeysInterruptTransferLength	= dev.getLastKeysInterruptTransferLength();
	_lastLCDInterruptTransferLength		= dev.getLastLCDInterruptTransferLength();
	_keysEndpoint		= dev._keysEndpoint;
	_LCDEndpoint		= dev._LCDEndpoint;
	_pUSBDevice			= dev._pUSBDevice;
	_pUSBDeviceHandle	= dev._pUSBDeviceHandle;
	_toRelease			= dev._toRelease;
	_toAttach			= dev._toAttach;

	_threadsStatus		= static_cast<bool>(dev._threadsStatus);
}

void USBDevice::initializeMacrosManager(
	const char* virtualKeyboardName,
	const std::vector<std::string> & keysNames)
{
	try {
		_pMacrosManager = new MacrosManager(virtualKeyboardName, keysNames);
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		throw GLogiKBadAlloc("macros manager allocation failure");
	}
}

void USBDevice::destroyMacrosManager(void) noexcept {
	if( _pMacrosManager ) {
		delete _pMacrosManager;
		_pMacrosManager = nullptr;
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

