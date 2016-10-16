
#include <cstring>

#include "include/log.h"
#include "keyboard_driver.h"

namespace GLogiKd
{

KeyboardDriver::KeyboardDriver() {
}

KeyboardDriver::~KeyboardDriver() {
	LOG(DEBUG3) << "KeyboardDriver destructor";
}

bool KeyboardDriver::searchDevice(const char* vendor_id, const char* product_id) {
	for( device_iterator_ = supported_devices_.begin();
	     device_iterator_ != supported_devices_.end(); ++device_iterator_) {
		if( std::strcmp( (*device_iterator_).vendor_id, vendor_id) == 0 )
			if( std::strcmp( (*device_iterator_).product_id, product_id) == 0 ) {
				return true;
			}
	}
	return false;
}

} // namespace GLogiKd

