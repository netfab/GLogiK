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
#if DEBUGGING_ON
		LOG(DEBUG3) << "initializing libusb";
#endif
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
#if DEBUGGING_ON
		LOG(DEBUG3) << "closing libusb";
#endif
		libusb_exit(USBInit::pContext);
		USBInit::status = false;
	}
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
			GKSysLog(LOG_ERR, ERROR, buffer.str());
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

