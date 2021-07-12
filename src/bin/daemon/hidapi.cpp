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

#include <sstream>
#include <iomanip>

#include "lib/utils/utils.hpp"

#include "usbinit.hpp"
#include "hidapi.hpp"

#include "USBAPIenums.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

hidapi::hidapi()
{
	LOG(INFO) << "initializing HIDAPI version " << hid_version_str();

	if(hid_init() != 0)
		throw GLogiKExcept("initializing HIDAPI failure");
}

hidapi::~hidapi()
{
#if DEBUGGING_ON
	LOG(DEBUG3) << "closing HIDAPI";
#endif

	if(hid_exit() != 0) {
		GKSysLog(LOG_ERR, ERROR, "exiting HIDAPI failure");
	}
}

void hidapi::openUSBDevice(USBDevice & device)
{
	auto make_hidapi_path = [&device] () -> const std::string {
		std::ostringstream os;

		/* hidapi versions 0.10.0 and 0.10.1 */
		if( (hid_version()->major == 0) and
			(hid_version()->minor == 10) ) {
			/* 0003:0002:01 */
			os	<< std::setfill('0')
				<< std::setw(4) << std::hex << toUInt(device.getBus()) << ":"
				<< std::setw(4) << toUInt(device.getNum()) << ":"
				<< std::setw(2) << toUInt(device.getBInterfaceNumber());
		}
		else {
			/* 3-2:1.1 */
			USBInit dev;

			USBPortNumbers_type port_numbers{0, 0, 0, 0, 0, 0, 0};

			const int num = dev.getUSBDevicePortNumbers(device, port_numbers);

			if(num < 1)
				throw GLogiKExcept("wrong number of ports");

			os << toUInt(device.getBus()) << "-" << toUInt(port_numbers[0]);

			for(int i = 1; i < num; i++) {
				os << "." << toUInt(port_numbers[i]);
			}

			os	<< ":" << toUInt(device.getBConfigurationValue())
				<< "." << toUInt(device.getBInterfaceNumber());
		}

		return os.str();
	};

	const std::string searchedPath(make_hidapi_path());
#if DEBUGGING_ON
	LOG(DEBUG1) << "searched path: " << searchedPath;
#endif

	/* getVendorID() and getProductID() return strings in hexadecimal format */
	const unsigned short vendor_id  = toUShort( device.getVendorID(),  16 );
	const unsigned short product_id = toUShort( device.getProductID(), 16 );

	struct hid_device_info *devs, *cur_dev = nullptr;

	devs = hid_enumerate(vendor_id, product_id);
	cur_dev = devs;

	while( cur_dev ) {
		if ((cur_dev->vendor_id == vendor_id) and
			(cur_dev->product_id == product_id)) {

			const std::string currentPath(cur_dev->path);
#if DEBUGGING_ON
			LOG(DEBUG2) << "detected HIDAPI device path: " << currentPath;
#endif
			if(currentPath == searchedPath) {
#if DEBUGGING_ON
				LOG(INFO) << device.getID() << " found HIDAPI device: " << searchedPath;
#endif
				device._pHIDDevice = hid_open_path(searchedPath.c_str());

				/* open_path failure */
				if(device._pHIDDevice == nullptr) {
					this->error(nullptr);
					hid_free_enumeration(devs); /* free */
					throw GLogiKExcept("opening HIDAPI USB device failure");
				}
#if DEBUGGING_ON
				LOG(DEBUG3) << device.getID() << " opened HIDAPI USB device";
#endif
				break;
			}
		}

		cur_dev = cur_dev->next;
	}

	hid_free_enumeration(devs); /* free */

	/* searched path was not found in hid_enumerate results */
	if(device._pHIDDevice == nullptr) {
		throw GLogiKExcept("device not found by hidapi enumerate");
	}
}

void hidapi::closeUSBDevice(USBDevice & device) noexcept
{
	if(device._pHIDDevice != nullptr) {
		hid_close(device._pHIDDevice);
#if DEBUGGING_ON
		LOG(DEBUG3) << device.getID() << " closed HIDAPI USB device";
#endif
	}
}

void hidapi::sendControlRequest(
	USBDevice & device,
	const unsigned char * data,
	uint16_t wLength)
{
	if( ! device.getUSBRequestsStatus() ) {
		GKSysLog(LOG_WARNING, WARNING, "skip device feature report sending, would probably fail");
		return;
	}

	int ret = hid_send_feature_report(device._pHIDDevice, data, wLength);
	if( ret == -1 ) {
		GKSysLog(LOG_ERR, ERROR, "error sending feature report");
		this->error(device._pHIDDevice);
	}
#if DEBUGGING_ON
	else {
		LOG(DEBUG2) << "sent HIDAPI feature report: " << ret << " bytes - expected: " << wLength;
	}
#endif
}

int hidapi::performKeysInterruptTransfer(
	USBDevice & device,
	unsigned int timeout)
{
	if( ! device.getUSBRequestsStatus() ) {
		GKSysLog(LOG_WARNING, WARNING, "skip device hid_read_timeout, would probably fail");
		return 0;
	}

	device._lastKeysInterruptTransferLength = 0;

	int ret = hid_read_timeout(
		device._pHIDDevice,
		static_cast<unsigned char*>(device._pressedKeys),
		device.getKeysInterruptBufferMaxLength(),
		timeout
	);

	/* nothing to read, reached timeout */
	if(ret == 0)
		return toEnumType(USBAPIKeysTransferStatus::TRANSFER_TIMEOUT);

	/* error */
	if(ret == -1) {
		GKSysLog(LOG_ERR, ERROR, "hid_read_timeout error");
		this->error(device._pHIDDevice);
		return toEnumType(USBAPIKeysTransferStatus::TRANSFER_ERROR);
	}

	/* actual number of bytes read */
	device._lastKeysInterruptTransferLength = ret;
	return 0;
}

int hidapi::performLCDScreenInterruptTransfer(
	USBDevice & device,
	const unsigned char * buffer,
	int bufferLength,
	unsigned int timeout)
{
	if( ! device.getUSBRequestsStatus() ) {
		GKSysLog(LOG_WARNING, WARNING, "skip device hid_write, would probably fail");
		return 0;
	}

	int ret = hid_write(device._pHIDDevice, buffer, bufferLength);

	/* error */
	if(ret == -1) {
		GKSysLog(LOG_ERR, ERROR, "hid_write error");
		this->error(device._pHIDDevice);
		return toEnumType(USBAPIKeysTransferStatus::TRANSFER_ERROR);
	}

	/* actual number of bytes written */
	device._lastLCDInterruptTransferLength = ret;
	return 0;
}

void hidapi::error(hid_device *dev) noexcept
{
	/* dev may be NULL for hid_open error */
	const std::wstring ws( toWString(hid_error(dev)) );
	const std::string error( ws.begin(), ws.end() );

	std::ostringstream buffer(std::ios_base::app);
	buffer	<< "HIDAPI error : " << error;
	GKSysLog(LOG_ERR, ERROR, buffer.str());
}

} // namespace GLogiK

