
#include <unistd.h>
#include <fcntl.h>

#include "exception.h"
#include "include/log.h"
#include "keyboard_driver.h"
#include "daemon_control.h"

namespace GLogiKd
{

KeyboardDriver::KeyboardDriver() {
	LOG(DEBUG2) << "KeyboardDriver:: constructor";
	this->connected_ = false;
}

KeyboardDriver::~KeyboardDriver() {
	LOG(DEBUG2) << "KeyboardDriver:: destructor";
	if( this->monitorThread_.joinable() )
		this->monitorThread_.join();
	this->closeDevNode();
}

std::vector<KeyboardDevice> KeyboardDriver::getSupportedDevices(void) const {
	return this->supported_devices_;
}

bool KeyboardDriver::isConnected(void) const {
	return this->connected_;
}

void KeyboardDriver::disconnectDevice(void) {
	LOG(DEBUG2) << "KeyboardDriver:disconnectDevice()";
	this->connected_ = false;
}

void KeyboardDriver::connectDevice(const char* hidraw_dev_node) {
	LOG(DEBUG2) << "KeyboardDriver::connectDevice()";
	try {
		this->openDevNode(hidraw_dev_node);
		this->monitorThread_ = std::thread(&KeyboardDriver::monitorDevice, this);
	}
	catch ( const GLogiKExcept & e ) {
		LOG(DEBUG3) << "Fails to open device node";
	}

}

void KeyboardDriver::monitorDevice(void) {
	LOG(DEBUG2) << "KeyboardDriver::monitorDevice()";
	while( this->isConnected() and DaemonControl::is_daemon_enabled() ) {
		poll(this->fds, 1, 6000);

		// check if device has been disconnected
		if((this->fds[0].revents & POLLHUP) or (this->fds[0].revents & POLLERR)) {
			this->closeDevNode();
			this->disconnectDevice();
		}
	}
}

void KeyboardDriver::openDevNode(const char* hidraw_dev_node) {
	LOG(DEBUG2) << "KeyboardDriver::openDevNode()";
	int fd = open(hidraw_dev_node, O_RDWR | O_NONBLOCK);
	if( fd == -1 )
		throw GLogiKExcept("opening HIDRAW dev node failure");

	this->fds[0].fd = fd;
	this->connected_ = true;
}

void KeyboardDriver::closeDevNode(void) {
	LOG(DEBUG2) << "KeyboardDriver::closeDevNode()";
	if( this->fds[0].fd != -1 ) {
		close(this->fds[0].fd);
		this->fds[0].fd = -1;
	}
}

} // namespace GLogiKd

