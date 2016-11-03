
#include "include/log.h"
#include "logitech_g15.h"

namespace GLogiKd
{

LogitechG15::LogitechG15() {
	this->supported_devices_ = {
		// name, vendor_id, product_id
		{ "Logitech G510", VENDOR_LOGITECH, "c22d" },
		//{ "Logitech G510", VENDOR_LOGITECH, "c30f" }
		};
}

LogitechG15::~LogitechG15() {
}

void LogitechG15::init(const char* vendor_id, const char* product_id) {
	LOG(DEBUG2) << "LogitechG15::init() Vid:Pid = "
				<< vendor_id << ":" << product_id;
}

} // namespace GLogiKd

