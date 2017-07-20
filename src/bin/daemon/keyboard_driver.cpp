
#include <config.h>

#include "exception.h"
#include "include/log.h"
#include "keyboard_driver.h"
#include "daemon_control.h"


namespace GLogiKd
{

KeyboardDriver::KeyboardDriver() : buffer_("", std::ios_base::app), context_(nullptr) {
}

KeyboardDriver::~KeyboardDriver() {
}

std::vector<KeyboardDevice> KeyboardDriver::getSupportedDevices(void) const {
	return this->supported_devices_;
}

void KeyboardDriver::initializeLibusb(void) {
	int ret_value = libusb_init( &(this->context_) );
	this->handleLibusbError(ret_value, "libusb initialization failure");
	//if (this->handleLibusbError(ret_value) != LIBUSB_SUCCESS)
	//	throw GLogiKExcept("libusb initialization failure");

#ifdef DEBUGGING_ON
	libusb_set_debug(this->context_, LIBUSB_LOG_LEVEL_WARNING);
#endif

	libusb_device **list;
	int num_devices = libusb_get_device_list(this->context_, &(list));
	if( num_devices <= 0 )
		this->handleLibusbError(num_devices, "error getting USB devices list (or zero!)");

	libusb_free_device_list(list, 1);
}

void KeyboardDriver::closeLibusb(void) {
	libusb_exit(this->context_);
}

int KeyboardDriver::handleLibusbError(int error_code, const char* except_msg) {
	switch(error_code) {
		case LIBUSB_SUCCESS:
			break;
		default:
			LOG(DEBUG4) << "handleLibusbError: (" <<  libusb_error_name(error_code) << ") "
						<< libusb_strerror((libusb_error)error_code);
			throw GLogiKExcept(except_msg);
			break;
	}

	return error_code;
}

} // namespace GLogiKd

