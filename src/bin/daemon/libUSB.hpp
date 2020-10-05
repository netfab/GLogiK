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

#ifndef SRC_BIN_DAEMON_LIBUSB_HPP_
#define SRC_BIN_DAEMON_LIBUSB_HPP_

#include <cstdint>

#include <libusb-1.0/libusb.h>

#include "USBDevice.hpp"

namespace GLogiK
{

class LibUSB
{
	public:

	protected:
		LibUSB(void);
		~LibUSB(void);

		void openUSBDevice(USBDevice & device);
		void closeUSBDevice(USBDevice & device) noexcept;

		void sendControlRequest(
			USBDevice & device,
			uint16_t wValue,
			uint16_t wIndex,
			unsigned char * data,
			uint16_t wLength
		);

		int performKeysInterruptTransfer(
			USBDevice & device,
			unsigned int timeout
		);

		int performLCDScreenInterruptTransfer(
			USBDevice & device,
			unsigned char* buffer,
			int bufferLength,
			unsigned int timeout
		);

	private:
		static bool status;				/* is libusb initialized ? */
		static uint8_t counter;			/* initialized drivers counter */
		static libusb_context * pContext;

		void setUSBDeviceActiveConfiguration(USBDevice & device);
		void findUSBDeviceInterface(USBDevice & device);
		void releaseUSBDeviceInterfaces(USBDevice & device) noexcept;

		void detachKernelDriverFromUSBDeviceInterface(USBDevice & device, int numInt);
		void attachUSBDeviceInterfacesToKernelDrivers(USBDevice & device) noexcept;

		int USBError(int errorCode) noexcept;

};

} // namespace GLogiK

#endif
