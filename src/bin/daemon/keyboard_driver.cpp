
#include "include/log.h"
#include "keyboard_driver.h"

namespace GLogiKd
{

KeyboardDriver::KeyboardDriver() {
}

KeyboardDriver::~KeyboardDriver() {
}

std::vector<device>::iterator KeyboardDriver::getSupportedDevicesFirst(void) {
	return supported_devices_.begin();
}

std::vector<device>::iterator KeyboardDriver::getSupportedDevicesEnd(void) {
	return supported_devices_.end();
}

} // namespace GLogiKd

