
#include "include/log.h"
#include "keyboard_driver.h"

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

