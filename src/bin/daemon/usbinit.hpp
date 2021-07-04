/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_BIN_DAEMON_USBINIT_HPP_
#define SRC_BIN_DAEMON_USBINIT_HPP_

#include <cstddef>
#include <cstdint>

#include <array>

#include <libusb-1.0/libusb.h>

#include "USBDevice.hpp"

namespace GLogiK
{

/* As per the USB 3.0 specs, the current maximum limit for the depth is 7. */
const std::size_t PORT_NUMBERS_LEN = 7;

typedef std::array<uint8_t, PORT_NUMBERS_LEN> USBPortNumbers_type;

class USBInit
{
	public:
		USBInit(void);
		~USBInit(void);

		const int getUSBDevicePortNumbers(
			USBDevice & device,
			USBPortNumbers_type & port_numbers
		);

	protected:
		int USBError(int errorCode) noexcept;
		void seekUSBDevice(USBDevice & device);

	private:
		static libusb_context * pContext;
		static uint8_t counter;			/* initialized drivers counter */
		static bool status;				/* is libusb initialized ? */
};

} // namespace GLogiK

#endif
