

#include <cstring>

#include <string>
#include <sstream>

#include <libudev.h>

#include "devices_manager.h"
#include "include/log.h"

#include "exception.h"
#include "daemon_control.h"

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

	for(const auto& driver : this->drivers_) {
		delete driver;
	}
	this->drivers_.clear();
}

void DevicesManager::initializeDrivers(void) {
	LOG(DEBUG2) << "starting DevicesManager::initializeDrivers()";
	for(const auto& det_dev : this->detected_devices_) {

		bool initializing = true;

		for(const auto& init_dev : this->initialized_devices_) {
				if(	(std::strcmp( det_dev.vendor_id, init_dev.vendor_id ) == 0) and
					(std::strcmp( det_dev.product_id, init_dev.product_id ) == 0) and
					(std::strcmp( det_dev.hidraw_dev_node, init_dev.hidraw_dev_node ) == 0) ) {
						LOG(DEBUG3) << "Device "  << det_dev.vendor_id << ":" << det_dev.product_id
									<< " - " << det_dev.hidraw_dev_node << " already initialized";
						initializing = false;
						break; // jump to next detected device
				}
		} // for

		if( initializing ) {
			for(const auto& driver : this->drivers_) {
				if( det_dev.driver_ID == driver->getDriverID() ) {
					//driver->init(det_dev.vendor_id, det_dev.product_id, det_dev.hidraw_dev_node); // initialization
					//driver->init(); // initialization
					this->initialized_devices_.push_back( det_dev );
					break;
				}
			} // for
		}
	} // for
	this->detected_devices_.clear();
}

void DevicesManager::searchSupportedDevices(void) {
	LOG(DEBUG2) << "starting DevicesManager::searchSupportedDevices()";

	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;

	enumerate = udev_enumerate_new(this->udev);
	if ( enumerate == NULL )
		throw GLogiKExcept("udev enumerate object creation failure");

	try {
		LOG(DEBUG2) << "hidraw enumerate";
		if( udev_enumerate_add_match_subsystem(enumerate, "hidraw") < 0 )
			throw GLogiKExcept("hidraw enumerate filtering init failure");

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

			struct udev_device *dev = udev_device_new_from_syspath(this->udev, path);
			if( dev == NULL )
				throw GLogiKExcept("new_from_syspath failure");

			const char* devss = udev_device_get_subsystem(dev);
			if( devss == NULL ) {
				udev_device_unref(dev);
				throw GLogiKExcept("get_subsystem failure");
			}

			// path to the HIDRAW device node in /dev
			const char* devnode = udev_device_get_devnode(dev);
			if( devnode == NULL ) {
				udev_device_unref(dev);
				throw GLogiKExcept("get_devnode failure");
			}

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
				for(const auto& driver : this->drivers_) {
					for(const auto& device : driver->getSupportedDevices()) {
						if( std::strcmp( device.vendor_id, vendor_id ) == 0 )
							if( std::strcmp( device.product_id, product_id ) == 0 ) {
/*
								struct udev_list_entry *devs, *devs_list_entry;
								devs = udev_device_get_sysattr_list_entry( dev );
								udev_list_entry_foreach( devs_list_entry, devs ) {
									const char* attr = udev_list_entry_get_name( devs_list_entry );
									if( attr != nullptr ) {
										LOG(DEBUG4) << attr << " " << udev_device_get_sysattr_value(dev, attr);
									}
								}
								LOG(DEBUG4) << "--";
*/
								bool already_detected = false;
								for(const auto& det_dev : this->detected_devices_) {
									if( (std::strcmp( det_dev.vendor_id, vendor_id ) == 0) and
										(std::strcmp( det_dev.product_id, product_id ) == 0) and
										(std::strcmp( det_dev.hidraw_dev_node, devnode ) == 0) ) {
											LOG(DEBUG3) << "Device "  << vendor_id << ":" << product_id
														<< " - " << devnode << " already detected";
											already_detected = true;
											break;
									}
								}

								if( ! already_detected ) {
									std::ostringstream s;
									s	<< "Device found !\n"
										<< "	Path		: " << path << "\n"
										<< "	Subsystem	: " << devss << "\n"
										<< "	Device Node	: " << devnode << "\n"
										<< "	Vendor ID	: " << vendor_id << "\n"
										<< "	Product ID	: " << product_id << "\n"
										<< "	Manufacturer	: " << manufacturer << "\n"
										<< "	Product		: " << product << "\n"
										<< "	Serial		: " << serial << "\n";

									DetectedDevice found;
									found.name				= device.name;
									found.vendor_id			= vendor_id;
									found.product_id		= product_id;
									found.hidraw_dev_node	= devnode;
									found.input_dev_node	= nullptr;
									found.manufacturer		= manufacturer;
									found.product			= product;
									found.serial			= serial;
									found.driver_ID			= driver->getDriverID();

									this->detected_devices_.push_back(found);
									throw DeviceFound(s.str());
								}
							}
					} // for
				} // for
			}
			catch ( const DeviceFound & e ) {
				LOG(DEBUG3) << e.what();
			}
			udev_device_unref(dev);
		} // udev_list_entry_foreach

		// ---
		// ---
		// ---

		// Free the enumerator object
		udev_enumerate_unref(enumerate);

		enumerate = udev_enumerate_new(this->udev);
		if ( enumerate == NULL )
			throw GLogiKExcept("udev enumerate object creation failure");

		// ---
		// ---
		// ---

		LOG(DEBUG2) << "input enumerate";
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

			struct udev_device *dev = udev_device_new_from_syspath(this->udev, path);
			if( dev == NULL )
				throw GLogiKExcept("new_from_syspath failure");

			if( udev_device_get_property_value(dev, "ID_INPUT_KEYBOARD") == nullptr ) {
				udev_device_unref(dev);
				continue;
			}

/*
			struct udev_list_entry *devs, *devs_list_entry;
			devs = udev_device_get_properties_list_entry( dev );
			udev_list_entry_foreach( devs_list_entry, devs ) {
				const char* attr = udev_list_entry_get_name( devs_list_entry );
				if( attr != nullptr ) {
					LOG(DEBUG4) << attr << " : " << udev_device_get_property_value(dev, attr);
				}
			}
			LOG(DEBUG4) << "--";
			LOG(DEBUG4) << "--";
			LOG(DEBUG4) << "--";
*/
			const char* vendor_id = udev_device_get_property_value(dev, "ID_VENDOR_ID");
			const char* product_id = udev_device_get_property_value(dev, "ID_MODEL_ID");
			if( (vendor_id == nullptr) or (product_id == nullptr) ) {
				udev_device_unref(dev);
				continue;
			}

			for( auto& detected : this->detected_devices_ ) {
				if( detected.input_dev_node != nullptr )
					continue; // skipping already found event device

				if( std::strcmp( detected.vendor_id, vendor_id ) == 0 )
					if( std::strcmp( detected.product_id, product_id ) == 0 ) {
/*
						struct udev_list_entry *devs, *devs_list_entry;
						devs = udev_device_get_properties_list_entry( dev );
						udev_list_entry_foreach( devs_list_entry, devs ) {
							const char* attr = udev_list_entry_get_name( devs_list_entry );
							if( attr != nullptr ) {
								LOG(DEBUG4) << attr << " : " << udev_device_get_property_value(dev, attr);
							}
						}
						LOG(DEBUG4) << "--";
						LOG(DEBUG4) << "--";
						LOG(DEBUG4) << "--";
*/
						// path to the event device node in /dev/input/
						const char* devnode = udev_device_get_devnode(dev);
						if( devnode == nullptr ) {
							udev_device_unref(dev);
							continue;
						}

						detected.input_dev_node = devnode;
						LOG(DEBUG3) << "Vid:Pid : " << vendor_id << ":" << product_id << " found event input : " << devnode;
					}
			}

			udev_device_unref(dev);
		} // udev_list_entry_foreach

	} // try
	catch ( const GLogiKExcept & e ) {
		// Free the enumerator object
		udev_enumerate_unref(enumerate);
		throw;
	}

	LOG(DEBUG2) << "Found " << this->detected_devices_.size() << " device(s)";

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

	while( DaemonControl::is_daemon_enabled() ) {
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

