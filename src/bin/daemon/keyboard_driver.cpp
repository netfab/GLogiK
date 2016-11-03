
#include "exception.h"
#include "include/log.h"
#include "keyboard_driver.h"
#include "daemon_control.h"

namespace GLogiKd
{

KeyboardDriver::KeyboardDriver() {
	LOG(DEBUG2) << "KeyboardDriver:: constructor";
}

KeyboardDriver::~KeyboardDriver() {
	LOG(DEBUG2) << "KeyboardDriver:: destructor";
}

std::vector<KeyboardDevice> KeyboardDriver::getSupportedDevices(void) const {
	return this->supported_devices_;
}

} // namespace GLogiKd

