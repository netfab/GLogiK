
#include "include/log.h"
#include "logitech_g15.h"

namespace GLogiKd
{

LogitechG15::LogitechG15() {
	/*
	// extended initializer lists only available with -std=c++11 or -std=gnu++11
	this->supported_devices_ = {
		// name, vendor_id, product_id
		{ "Logitech G510", VENDOR_LOGITECH, "c22d" }
		};
	*/

	/*
	this->supported_devices_.push_back( device() );
	this->supported_devices_[0].name = "Logitech G510";
	this->supported_devices_[0].vendor_id = VENDOR_LOGITECH;
	this->supported_devices_[0].product_id = "c22d";
	*/

	/*
	device a = { "Logitech G510", VENDOR_LOGITECH, "c22d" };
	this->supported_devices_.push_back( a );
	*/
	this->supported_devices_.push_back( device("Logitech G510", VENDOR_LOGITECH, "c22d") );

}

LogitechG15::~LogitechG15() {
}

void LogitechG15::init() {
	LOG(DEBUG3) << "init G15 !";
}

} // namespace GLogiKd

