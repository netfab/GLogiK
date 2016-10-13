
#include <libudev.h>

#include "devices_manager.h"
#include "include/log.h"

#include "exception.h"
#include "daemon.h"

namespace GLogiKd
{

DevicesManager::DevicesManager() {
}

DevicesManager::~DevicesManager() {
	if(this->mon) {
		LOG(DEBUG3) << "unref monitor object";
		udev_monitor_unref(this->mon);
	}
	if(this->udev) {
		LOG(DEBUG3) << "unref udev context";
		udev_unref(this->udev);
	}
}

void DevicesManager::monitor(void) {

	LOG(DEBUG2) << "starting DevicesManager::monitor()";

	this->udev = udev_new();
	if ( this->udev == NULL )
		throw GLogiKExcept("udev context init failure");

	this->mon = udev_monitor_new_from_netlink(this->udev, "udev");
	if( this->mon == NULL )
		throw GLogiKExcept("allocating udev monitor failure");

	if( udev_monitor_filter_add_match_subsystem_devtype(this->mon, "hidraw", NULL) < 0 )
		throw GLogiKExcept("hidraw monitor filtering init failure");

	if( udev_monitor_enable_receiving(mon) < 0 )
		throw GLogiKExcept("monitor enabling failure");

	while( GLogiKDaemon::is_daemon_enabled() ) {
		sleep(1);
	}

}

} // namespace GLogiKd

