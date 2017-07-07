
#include <sstream>

#include <config.h>

#include "include/log.h"
#include <syslog.h>

#include "logitech_g15.h"

#include <libg15.h>

namespace GLogiKd
{

LogitechG15::LogitechG15() {
	this->supported_devices_ = {
		// name, vendor_id, product_id
		{ "Logitech G510", VENDOR_LOGITECH, "c22d" },
		//{ "Logitech G510", VENDOR_LOGITECH, "c30f" }
		};

#ifdef DEBUGGING_ON
	libg15Debug(G15_LOG_WARN);
#endif
}

LogitechG15::~LogitechG15() {
	int ret = exitLibG15();
	if ( ret != G15_NO_ERROR ) {
		this->logLibG15Error( "LogitechG15::exitLibG15() failure return code : ", ret);
	}
	LOG(DEBUG3) << "LogitechG15 cleaned";
}

void LogitechG15::init(const char* vendor_id, const char* product_id) {
	LOG(DEBUG2) << "LogitechG15::init() Vid:Pid = "
				<< vendor_id << ":" << product_id;

	unsigned int vendor = std::stoul(vendor_id, nullptr, 16);
	unsigned int product = std::stoul(product_id, nullptr, 16);

	int ret = setupLibG15(vendor, product, 0);

	if ( ret != G15_NO_ERROR ) {
		this->logLibG15Error( "LogitechG15::setupLibG15() failure return code : ", ret);
	}
}

void LogitechG15::logLibG15Error(const char* msg, int ret) {
		std::ostringstream buff(msg, std::ios_base::app);
		buff << ret;
		syslog( LOG_ERR, buff.str().c_str() );
		LOG(ERROR) << buff.str();
}

} // namespace GLogiKd

