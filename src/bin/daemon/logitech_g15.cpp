
#include "include/log.h"
#include "logitech_g15.h"

namespace GLogiKd
{

LogitechG15::LogitechG15() {
	// extended initializer lists only available with -std=c++11 or -std=gnu++11
	this->supported_devices_ = {
		// name, vendor_id, product_id
		{ "Logitech G510", VENDOR_LOGITECH, "c22e" },
		{ "Logitech G510", VENDOR_LOGITECH, "c30f" }
		};
}

LogitechG15::~LogitechG15() {
}

void LogitechG15::init(const char* vendor_id, const char* product_id, const char* hidraw_dev_node) {
	LOG(DEBUG2) << "starting LogitechG15::init()";

	this->connectDevice(hidraw_dev_node);
}

} // namespace GLogiKd

