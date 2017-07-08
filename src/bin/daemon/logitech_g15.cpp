
#include <sstream>

#include <config.h>

#include "include/log.h"
#include <syslog.h>

#include "logitech_g15.h"

#include <libg15.h>

namespace GLogiKd
{

LogitechG15::LogitechG15() : initialized(false) {
	this->supported_devices_ = {
		// name, vendor_id, product_id
		{ "Logitech G510", VENDOR_LOGITECH, "c22d" },
		//{ "Logitech G510", VENDOR_LOGITECH, "c30f" }
		};

#ifdef DEBUGGING_ON
	libg15Debug(G15_LOG_INFO);
#endif
}

LogitechG15::~LogitechG15() {
	LOG(DEBUG3) << "exiting libg15";
	if( this->initialized )
		this->closeDevice();
}

void LogitechG15::init(const char* vendor_id, const char* product_id) {
	LOG(DEBUG3) << "initializing libg15 device Vid:Pid - "
				<< vendor_id << ":" << product_id;

	// FIXME
	if( this->initialized ) {
		LOG(WARNING) << "Device already initialized";
		return;
	}

	unsigned int vendor = std::stoul(vendor_id, nullptr, 16);
	unsigned int product = std::stoul(product_id, nullptr, 16);

	int ret = setupLibG15(vendor, product, 0);

	// FIXME exception
	if ( ret != G15_NO_ERROR )
		this->logLibG15Error("setupLibG15() failure. Return code : ", ret);
	else
		this->initialized = true;
}

void LogitechG15::logLibG15Error(const char* msg, int ret) {
		std::ostringstream buff(msg, std::ios_base::app);
		buff << ret;
		syslog( LOG_ERR, buff.str().c_str() );
		LOG(ERROR) << buff.str();
}

void LogitechG15::closeDevice() {
	int ret = exitLibG15();
	if ( ret != G15_NO_ERROR )
		this->logLibG15Error("exitLibG15() failure. Return code : ", ret);
	LOG(DEBUG3) << "closed device";
	this->initialized = false;
}

} // namespace GLogiKd

