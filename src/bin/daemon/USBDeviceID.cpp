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
			const std::string & n,
			const std::string & v,
			const std::string & p,
			const uint64_t c,
			const uint8_t b,
			const uint8_t num,
			const uint8_t C,
			const uint8_t I,
			const uint8_t A,
			const uint8_t N,
			const int8_t bufferMaxLength,
			const int8_t macrosKeysLength,
			const int8_t mediaKeysLength,
			const int8_t LCDKeysLength
		)	:	_name(n),
				_vendorID(v),
				_productID(p),
				_devID(USBDeviceID::getDeviceID(b, num)),
				_capabilities(c),
				_bus(b),
				_num(num),
				_bConfigurationValue(C),
				_bInterfaceNumber(I),
				_bAlternateSetting(A),
				_bNumEndpoints(N),
				_MacrosKeysLength(macrosKeysLength),
				_MediaKeysLength(mediaKeysLength),
				_LCDKeysLength(LCDKeysLength)
{
	_keysInterruptBufferMaxLength = bufferMaxLength;

	if( bufferMaxLength > KEYS_BUFFER_LENGTH ) {
		GKSysLog(LOG_WARNING, WARNING, "interrupt read length too large, set it to max buffer length");
		_keysInterruptBufferMaxLength = KEYS_BUFFER_LENGTH;
	}
}

} // namespace GLogiK

