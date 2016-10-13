
#include <libudev.h>

#include <string>

#include "devices_manager.h"
#include "include/log.h"

#include "exception.h"
#include "daemon.h"

namespace GLogiKd
{

DevicesManager::DevicesManager() : udev(NULL), monitor(NULL), fd_(-1) {
	this->fds[0].fd = -1;
	this->fds[0].events = POLLIN;
}

DevicesManager::~DevicesManager() {
	if(this->monitor) {
		LOG(DEBUG3) << "unref monitor object";
		udev_monitor_unref(this->monitor);
	}
	if(this->udev) {
		LOG(DEBUG3) << "unref udev context";
		udev_unref(this->udev);
	}
}

void DevicesManager::startMonitoring(void) {
	LOG(DEBUG2) << "starting DevicesManager::startMonitoring()";

	this->udev = udev_new();
	if ( this->udev == NULL )
		throw GLogiKExcept("udev context init failure");

	this->monitor = udev_monitor_new_from_netlink(this->udev, "udev");
	if( this->monitor == NULL )
		throw GLogiKExcept("allocating udev monitor failure");

	if( udev_monitor_filter_add_match_subsystem_devtype(this->monitor, "hidraw", NULL) < 0 )
		throw GLogiKExcept("hidraw monitor filtering init failure");

	if( udev_monitor_filter_add_match_subsystem_devtype(this->monitor, "input", NULL) < 0 )
		throw GLogiKExcept("input monitor filtering init failure");

	if( udev_monitor_enable_receiving(this->monitor) < 0 )
		throw GLogiKExcept("monitor enabling failure");

	this->fd_ = udev_monitor_get_fd(this->monitor);
	if( this->fd_ < 0 )
		throw GLogiKExcept("can't get the monitor file descriptor");

	this->fds[0].fd = this->fd_;
	this->fds[0].events = POLLIN;

	struct udev_device *dev = NULL;
	int ret = 0;

	std::string str;
	while( GLogiKDaemon::is_daemon_enabled() ) {
		ret = poll(this->fds, 1, 6000);
		// receive data ?
		if( ret > 0 ) {
			dev = udev_monitor_receive_device(this->monitor);
			if(dev) {
				#if 0
				const char* p1 = udev_device_get_devnode(dev);
				const char* p2 = udev_device_get_subsystem(dev);
				const char* p3 = udev_device_get_devtype(dev);
				const char* p4 = udev_device_get_action(dev);
				LOG(DEBUG3) << "Got device";
				str = ( p1 != NULL ) ? p1 : "(null)";
				LOG(DEBUG3) << "	Node : " << str;
				str = ( p2 != NULL ) ? p2 : "(null)";
				LOG(DEBUG3) << "	Subsystem : " << str;
				str = ( p3 != NULL ) ? p3 : "(null)";
				LOG(DEBUG3) << "	Devtype : " << str;
				str = ( p4 != NULL ) ? p4 : "(null)";
				LOG(DEBUG3) << "	Action : " << str;
				#endif

				udev_device_unref(dev);
			}
			else {
				LOG(DEBUG3) << "No Device from receive_device(). An error occured";
			}
		}
	}

}

} // namespace GLogiKd

