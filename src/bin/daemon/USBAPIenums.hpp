/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2025  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_BIN_DAEMON_USBAPI_ENUMS_HPP_
#define SRC_BIN_DAEMON_USBAPI_ENUMS_HPP_

#include <cstdint>

#include <config.h>

#if GKLIBUSB
#include <libusb-1.0/libusb.h>
#endif

namespace GLogiK
{

enum class USBAPIKeysTransferStatus : int8_t
{
	TRANSFER_ERROR = -1,
#if GKLIBUSB
	TRANSFER_TIMEOUT = LIBUSB_ERROR_TIMEOUT,
#elif GKHIDAPI
	TRANSFER_TIMEOUT = -7,
#endif
};

} // namespace GLogiK

#endif
