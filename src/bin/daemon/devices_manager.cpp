

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

DevicesManager::DevicesManager() {
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

	for(std::vector<KeyboardDriver*>::iterator it = this->drivers_.begin(); it != this->drivers_.end(); ++it) {
		delete (*it);
	}
	this->drivers_.clear();
}

void DevicesManager::initializeDrivers(void) {
	for(std::vector<DetectedDevice>::iterator d_it = this->detected_devices_.begin();
		d_it != this->detected_devices_.end(); ++d_it) {

		bool initializing = true;

		for(std::vector<DetectedDevice>::iterator i_it = this->initialized_devices_.begin();
			i_it != this->initialized_devices_.end(); ++i_it) {
				if( (std::strcmp( (*d_it).vendor_id, (*i_it).vendor_id ) == 0) and
					(std::strcmp( (*d_it).product_id, (*i_it).product_id ) == 0) and
					(std::strcmp( (*d_it).hidraw_dev_node, (*i_it).hidraw_dev_node ) == 0) ) {
						LOG(DEBUG3) << "Device "  << (*d_it).vendor_id << ":" << (*d_it).product_id
									<< " - " << (*d_it).hidraw_dev_node << " already initialized";
						initializing = false;
						break; // jump to next detected device
				}
		} // for

		if( initializing ) {
			for(std::vector<KeyboardDriver*>::iterator drivers_it = this->drivers_.begin();
				drivers_it != this->drivers_.end(); ++drivers_it) {
				if( (*d_it).driver_ID == (*drivers_it)->getDriverID() ) {

					(*drivers_it)->init(); // initialization

					this->initialized_devices_.push_back( (*d_it) );
					break;
				}
			} // for
		}
	} // for
	this->detected_devices_.clear();
}

void DevicesManager::searchSupportedDevices(void) {

	struct udev_enumerate *enumerate;

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

		struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
		if( devices == NULL )
			throw GLogiKExcept("devices empty list or failure");

		struct udev_list_entry *dev_list_entry;
		udev_list_entry_foreach(dev_list_entry, devices) {
			// Get the filename of the /sys entry for the device
			// and create a udev_device object (dev) representing it
			const char* path = udev_list_entry_get_name(dev_list_entry);
			if( path == NULL )
				throw GLogiKExcept("entry_get_name failure");

			struct udev_device *dev = udev_device_new_from_syspath(this->udev, path);
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

				try {
					for(std::vector<KeyboardDriver*>::iterator drivers_it = this->drivers_.begin();
						drivers_it != this->drivers_.end(); ++drivers_it) {
						for(std::vector<device>::iterator it = (*drivers_it)->getSupportedDevicesFirst();
							it != (*drivers_it)->getSupportedDevicesEnd(); ++it) {
							if( std::strcmp( (*it).vendor_id, vendor_id ) == 0 )
								if( std::strcmp( (*it).product_id, product_id ) == 0 ) {

									bool already_detected = false;
									for(std::vector<DetectedDevice>::iterator d_it = this->detected_devices_.begin();
										d_it != this->detected_devices_.end(); ++d_it) {
										if( (std::strcmp( (*d_it).vendor_id, vendor_id ) == 0) and
											(std::strcmp( (*d_it).product_id, product_id ) == 0) and
											(std::strcmp( (*d_it).hidraw_dev_node, devnode ) == 0) ) {
												LOG(DEBUG3) << "Device "  << vendor_id << ":" << product_id
															<< " - " << devnode << " already detected";
												already_detected = true;
												break;
										}
									}

									if( ! already_detected ) {
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

										DetectedDevice found;
										found.name				= (*it).name;
										found.vendor_id			= vendor_id;
										found.product_id		= product_id;
										found.hidraw_dev_node	= devnode;
										found.manufacturer		= manufacturer;
										found.product			= product;
										found.serial			= serial;
										found.driver_ID			= (*drivers_it)->getDriverID();

										this->detected_devices_.push_back(found);
										throw DeviceFound(product);
									}
								}
						} // for
					} // for
				}
				catch ( const DeviceFound & e ) {
					LOG(DEBUG3) << "Device found : " << e.what();
				}
			} // if subsystem == hidraw

			udev_device_unref(dev);

		} // udev_list_entry_foreach

	}
	catch ( const GLogiKExcept & e ) {
		// Free the enumerator object
		udev_enumerate_unref(enumerate);
		throw;
	}

	LOG(DEBUG3) << "Found " << this->detected_devices_.size() << " device(s)";

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

	this->drivers_.push_back( new LogitechG15() );

	this->searchSupportedDevices();
	this->initializeDrivers();

	while( GLogiKDaemon::is_daemon_enabled() ) {
		int ret = poll(this->fds, 1, 6000);
		// receive data ?
		if( ret > 0 ) {
			struct udev_device *dev = udev_monitor_receive_device(this->monitor);
			if( dev == NULL )
				throw GLogiKExcept("no device from receive_device(), something is wrong");

			const char* action = udev_device_get_action(dev);
			if( action == NULL )
				throw GLogiKExcept("device_get_action() failure");

			if( std::strcmp(action, "add") == 0 ) {
				this->searchSupportedDevices();
				this->initializeDrivers();
			}

			udev_device_unref(dev);
		}
	}

}

} // namespace GLogiKd

