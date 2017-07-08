
#include "exception.h"
#include "include/log.h"
#include "keyboard_driver.h"
#include "daemon_control.h"

namespace GLogiKd
{

KeyboardDriver::KeyboardDriver() {
}

KeyboardDriver::~KeyboardDriver() {
}

std::vector<KeyboardDevice> KeyboardDriver::getSupportedDevices(void) const {
	return this->supported_devices_;
}

} // namespace GLogiKd

