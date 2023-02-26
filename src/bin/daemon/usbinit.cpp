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

#include "lib/utils/utils.hpp"

#include "usbinit.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

libusb_context * USBInit::pContext = nullptr;
uint8_t USBInit::counter = 0;
bool USBInit::status = false;

const std::string USBInit::getLibUSBVersion(void)
{
	const struct libusb_version* version = libusb_get_version();

	std::string ret;

	ret += std::to_string(version->major);
	ret += ".";
	ret += std::to_string(version->minor);
	ret += ".";
	ret += std::to_string(version->micro);

	return ret;
}

/*
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 *
 *  === protected === protected === protected === protected ===
 *
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 */

USBInit::USBInit(void)
{
	if( ! USBInit::status ) {
		GKLog(trace, "initializing libusb")

		int ret = libusb_init( &(USBInit::pContext) );
		if ( this->USBError(ret) ) {
			throw GLogiKExcept("libusb initialization failure");
		}

		USBInit::status = true;
	}

	USBInit::counter++;
}

USBInit::~USBInit(void)
{
	USBInit::counter--;

	if (USBInit::status and USBInit::counter == 0) {
		GKLog(trace, "closing libusb")

		libusb_exit(USBInit::pContext);
		USBInit::status = false;
	}
}

const int USBInit::getUSBDevicePortNumbers(
	USBDevice & device,
	USBPortNumbers_type & port_numbers)
{
	/* throws on failure */
	this->seekUSBDevice(device);

	int num_ports = libusb_get_port_numbers(device._pUSBDevice, &port_numbers[0], port_numbers.size());

	if(num_ports == LIBUSB_ERROR_OVERFLOW) {
		throw GLogiKExcept("array overflow error");
	}

	device._pUSBDevice = nullptr;

	GKLog2(trace, "number of ports : ", num_ports)

	return num_ports;
}

int USBInit::USBError(int errorCode) noexcept
{
	switch(errorCode) {
		case LIBUSB_SUCCESS:
			break;
		default:
			std::ostringstream buffer(std::ios_base::app);
			buffer	<< "libusb error (" << libusb_error_name(errorCode) << ") : "
					<< libusb_strerror( (libusb_error)errorCode );
			GKSysLogError(buffer.str());
			break;
	}

	return errorCode;
}

void USBInit::seekUSBDevice(USBDevice & device)
{
	libusb_device **list;
	int numDevices = libusb_get_device_list(USBInit::pContext, &(list));
	if( numDevices < 0 ) {
		this->USBError(numDevices);
		throw GLogiKExcept("error getting USB devices list");
	}

	for (int i = 0; i < numDevices; ++i) {
		device._pUSBDevice = list[i];
		if( libusb_get_bus_number(device._pUSBDevice) == device.getBus() and
			libusb_get_device_address(device._pUSBDevice) == device.getNum() ) {
			break;
		}
		device._pUSBDevice = nullptr;
	}

	if( device._pUSBDevice == nullptr ) {
		std::ostringstream buffer(std::ios_base::app);
		buffer	<< "libusb cannot find device " << device.getNum()
				<< " on bus " << device.getBus();
		libusb_free_device_list(list, 1);
		throw GLogiKExcept(buffer.str());
	}

	libusb_free_device_list(list, 1);
}

} // namespace GLogiK

