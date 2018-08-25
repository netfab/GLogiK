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

#ifndef __GLOGIKD_LIBUSB_H__
#define __GLOGIKD_LIBUSB_H__

#include <cstdint>
#include <sstream>

#include <libusb-1.0/libusb.h>

#include "USBDevice.h"

namespace GLogiK
{

struct DescriptorValues {
	uint8_t b_configuration_value;
	uint8_t b_interface_number;
	uint8_t b_alternate_setting;
	uint8_t b_num_endpoints;
};

class LibUSB
{
	public:

	protected:
		LibUSB(const int key_read_length, const DescriptorValues & values);
		~LibUSB(void);

		std::ostringstream buffer_;
		int interrupt_buffer_max_length_;

		int USBError(int error_code);

		void openUSBDevice(USBDevice & device);
		void closeUSBDevice(USBDevice & device);

		void setUSBDeviceActiveConfiguration(USBDevice & device);
		void findUSBDeviceInterface(USBDevice & device);
		void releaseUSBDeviceInterfaces(USBDevice & device);

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
			int buffer_length,
			unsigned int timeout
		);

	private:
		static bool libusb_status_;		/* is libusb initialized ? */
		static uint8_t drivers_cnt_;	/* initialized drivers counter */
		static libusb_context *context_;

		const DescriptorValues expected_usb_descriptors_;

		void detachKernelDriverFromUSBDeviceInterface(USBDevice & device, int numInt);
		void attachUSBDeviceInterfacesToKernelDrivers(USBDevice & device);

};

} // namespace GLogiK

#endif
