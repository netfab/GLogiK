
#include <config.h>

#include "exception.h"
#include "include/log.h"
#include "keyboard_driver.h"


namespace GLogiKd
{

bool KeyboardDriver::libusb_status_ = false;
unsigned int KeyboardDriver::drivers_cnt_ = 0;

KeyboardDriver::KeyboardDriver() : buffer_("", std::ios_base::app), context_(nullptr) {
	KeyboardDriver::drivers_cnt_++;
}

KeyboardDriver::~KeyboardDriver() {
	KeyboardDriver::drivers_cnt_--;
	if (KeyboardDriver::libusb_status_ and KeyboardDriver::drivers_cnt_ == 0)
		this->closeLibusb();
}

std::vector<KeyboardDevice> KeyboardDriver::getSupportedDevices(void) const {
	return this->supported_devices_;
}

void KeyboardDriver::initializeLibusb(void) {
	LOG(DEBUG4) << "initializing libusb";
	int ret_value = libusb_init( &(this->context_) );
	this->handleLibusbError(ret_value, "libusb initialization failure");

	KeyboardDriver::libusb_status_ = true;

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
	LOG(DEBUG4) << "closing libusb";
	libusb_exit(this->context_);
	KeyboardDriver::libusb_status_ = false;
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

