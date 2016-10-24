
#include "include/log.h"
#include "logitech_g15.h"

namespace GLogiKd
{

LogitechG15::LogitechG15() {
	// extended initializer lists only available with -std=c++11 or -std=gnu++11
	this->supported_devices_ = {
		// name, vendor_id, product_id
		{ "Logitech G510", VENDOR_LOGITECH, "c22d" }
		};
}

LogitechG15::~LogitechG15() {
}

void LogitechG15::init() {
	LOG(DEBUG2) << "starting LogitechG15::init()";
}

} // namespace GLogiKd

