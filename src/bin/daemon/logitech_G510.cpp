
#include <config.h>

#include "exception.h"
#include "include/log.h"
#include <syslog.h>

#include "logitech_G510.h"

namespace GLogiKd
{

LogitechG510::LogitechG510() : buffer_("", std::ios_base::app) {
	this->supported_devices_ = {
		// name, vendor_id, product_id
		{ "Logitech G510/G510s", VENDOR_LOGITECH, "c22d" },
		};
}

LogitechG510::~LogitechG510() {
}

void LogitechG510::initializeDevice(const char* vendor_id, const char* product_id) {
	LOG(DEBUG3) << "initializing G510 device Vid:Pid - "
				<< vendor_id << ":" << product_id;

}

void LogitechG510::closeDevice() {
	LOG(DEBUG3) << "closing G510 device";
}

} // namespace GLogiKd

