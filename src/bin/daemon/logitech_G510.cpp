
#include <config.h>

#include "exception.h"
#include "include/log.h"
#include <syslog.h>

#include "logitech_G510.h"

namespace GLogiKd
{

LogitechG510::LogitechG510() {
	this->supported_devices_ = {
		// name, vendor_id, product_id
		{ "Logitech G510/G510s", VENDOR_LOGITECH, "c22d" },
		};
}

LogitechG510::~LogitechG510() {
}

void LogitechG510::initializeDevice(const KeyboardDevice & device) {
	LOG(DEBUG3) << "Trying to initialize " << device.name << "("
				<< device.vendor_id << ":" << device.product_id << ")";
	this->initializeLibusb();
}

void LogitechG510::closeDevice() {
	LOG(DEBUG3) << "closing G510 device";
	this->closeLibusb();
}

} // namespace GLogiKd

