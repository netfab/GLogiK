

#include <cstring>

#include <string>

#include <libudev.h>

#include "devices_manager.h"
#include "include/log.h"

#include "exception.h"
#include "daemon.h"

#include "logitech_g15.h"

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

	for(std::vector<KeyboardDriver*>::iterator it = this->drivers.begin(); it != this->drivers.end(); ++it) {
		delete (*it);
	}
	this->drivers.clear();
}

void DevicesManager::searchSupportedDevices(void) {

	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;
	struct udev_device *dev;

	enumerate = udev_enumerate_new(this->udev);
	if ( enumerate == NULL )
		throw GLogiKExcept("udev enumerate object creation failure");

	try {
		if( udev_enumerate_add_match_subsystem(enumerate, "hidraw") < 0 )
			throw GLogiKExcept("hidraw enumerate filtering init failure");

		if( udev_enumerate_add_match_subsystem(enumerate, "input") < 0 )
			throw GLogiKExcept("input enumerate filtering init failure");

		if( udev_enumerate_scan_devices(enumerate) < 0 )
			throw GLogiKExcept("enumerate_scan_devices failure");

		devices = udev_enumerate_get_list_entry(enumerate);
		if( devices == NULL )
			throw GLogiKExcept("devices empty list or failure");

		udev_list_entry_foreach(dev_list_entry, devices) {
			// Get the filename of the /sys entry for the device
			// and create a udev_device object (dev) representing it
			const char* path = udev_list_entry_get_name(dev_list_entry);
			if( path == NULL )
				throw GLogiKExcept("entry_get_name failure");

			dev = udev_device_new_from_syspath(this->udev, path);
			if( dev == NULL )
				throw GLogiKExcept("new_from_syspath failure");

			const char* devss = udev_device_get_subsystem(dev);
			if( devss == NULL )
				throw GLogiKExcept("get_subsystem failure");

			if( std::strcmp(devss, "hidraw") == 0 ) {
				// path to the device node in /dev
				const char* devnode = udev_device_get_devnode(dev);
				if( devnode == NULL )
					throw GLogiKExcept("get_devnode failure");

				dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
				if( dev == NULL )
					throw GLogiKExcept("unable to find parent usb device");

				const char* vendor_id    = udev_device_get_sysattr_value(dev,"idVendor");
				const char* product_id   = udev_device_get_sysattr_value(dev,"idProduct");
				const char* manufacturer = udev_device_get_sysattr_value(dev,"manufacturer");
				const char* product      = udev_device_get_sysattr_value(dev,"product");
				const char* serial       = udev_device_get_sysattr_value(dev,"serial");

				vendor_id = (vendor_id == NULL) ? "(null)" : vendor_id;
				product_id = (product_id == NULL) ? "(null)" : product_id;
				manufacturer = (manufacturer == NULL) ? "(null)" : manufacturer;
				product = (product == NULL) ? "(null)" : product;
				serial = (serial == NULL) ? "(null)" : serial;

				for(std::vector<KeyboardDriver*>::iterator drivers_it = this->drivers.begin();
					drivers_it != this->drivers.end(); ++drivers_it) {
					for(std::vector<device>::iterator it = (*drivers_it)->getSupportedDevicesFirst();
						it != (*drivers_it)->getSupportedDevicesEnd(); ++it) {
						if( std::strcmp( (*it).vendor_id, vendor_id ) == 0 )
							if( std::strcmp( (*it).product_id, product_id ) == 0 ) {
								#if GLOGIKD_DEVICES_MANAGER_DEBUG
								LOG(DEBUG3)	<< "Device found !\n"
										<< "	Path		: " << path << "\n"
										<< "	Subsystem	: " << devss << "\n"
										<< "	Device Node	: " << devnode << "\n"
										<< "	Vendor ID	: " << vendor_id << "\n"
										<< "	Product ID	: " << product_id << "\n"
										<< "	Manufacturer	: " << manufacturer << "\n"
										<< "	Product		: " << product << "\n"
										<< "	Serial		: " << serial << "\n";
								#endif
							}
					}
				}
			}

			udev_device_unref(dev);

		}

	}
	catch ( const GLogiKExcept & e ) {
		// Free the enumerator object
		udev_enumerate_unref(enumerate);
		throw;
	}

	// Free the enumerator object
	udev_enumerate_unref(enumerate);
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

	this->drivers.push_back( new LogitechG15() );

	this->searchSupportedDevices();

	while( GLogiKDaemon::is_daemon_enabled() ) {
		ret = poll(this->fds, 1, 6000);
		// receive data ?
		if( ret > 0 ) {
			dev = udev_monitor_receive_device(this->monitor);
			if( dev == NULL )
				throw GLogiKExcept("no device from receive_device(), something is wrong");

			const char* action = udev_device_get_action(dev);
			if( action == NULL )
				throw GLogiKExcept("device_get_action() failure");

			if( std::strcmp(action, "add") == 0 ) {
				this->searchSupportedDevices();
			}

			udev_device_unref(dev);
		}
	}

}

} // namespace GLogiKd

