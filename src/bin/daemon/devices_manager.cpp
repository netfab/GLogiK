

#include <string>

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
	LOG(DEBUG2) << "DevicesManager::initializeDrivers()";
	for(const auto& det_dev : this->detected_devices_) {

		bool initializing = true;

		for(const auto& init_dev : this->initialized_devices_) {
				if(	(det_dev.vendor_id == init_dev.vendor_id) and
					(det_dev.product_id == init_dev.product_id) and
					(det_dev.input_dev_node == init_dev.input_dev_node) and
					(det_dev.usec == init_dev.usec) ) {
						LOG(DEBUG3) << "Device "  << det_dev.vendor_id << ":" << det_dev.product_id
									<< " - " << det_dev.input_dev_node << " already initialized";
						initializing = false;
						break; // jump to next detected device
				}
		} // for

		if( initializing ) {
			for(const auto& driver : this->drivers_) {
				if( det_dev.driver_ID == driver->getDriverID() ) {
					LOG(DEBUG3) << "initializing "
								<< det_dev.vendor_id << ":" << det_dev.product_id
								<< ":" << det_dev.input_dev_node << ":" << det_dev.usec;
					driver->init(det_dev.vendor_id.c_str(), det_dev.product_id.c_str()); // initialization
					this->initialized_devices_.push_back( det_dev );
					break;
				}
			} // for
		}
	} // for
	this->detected_devices_.clear();
	LOG(DEBUG2) << "Initialized " << this->initialized_devices_.size() << " device(s)";
	LOG(DEBUG2) << "---";
}

void DevicesManager::cleanDrivers(void) {
	LOG(DEBUG2) << "DevicesManager::cleanDrivers()";

	for(auto it = this->initialized_devices_.begin(); it != this->initialized_devices_.end();) {
		bool stop_it = true;
		for(const auto& det_dev : this->detected_devices_) {
			if( ((*it).input_dev_node == det_dev.input_dev_node) and ((*it).usec == det_dev.usec )
				and ((*it).vendor_id == det_dev.vendor_id ) and ((*it).product_id == det_dev.product_id ) ) {
				stop_it = false;
				++it;
				break;
			}
		}

		if(stop_it) {
			LOG(DEBUG3) << "erasing initialized driver : "
						<< (*it).vendor_id << ":" << (*it).product_id
						<< ":" << (*it).input_dev_node << ":" << (*it).usec;
						it = this->initialized_devices_.erase(it);
		}
	}

	this->detected_devices_.clear();
	LOG(DEBUG2) << "Initialized " << this->initialized_devices_.size() << " device(s)";
	LOG(DEBUG2) << "---";
}

#if GLOGIKD_GLOBAL_DEBUG
void deviceProperties(struct udev_device *dev, const std::string &subsystem) {
	struct udev_list_entry *devs, *devs_list_entry;

	if( subsystem == "input" )
		devs = udev_device_get_properties_list_entry( dev );
	else if( subsystem == "hidraw" )
		devs = udev_device_get_sysattr_list_entry( dev );

	const char* value = nullptr;
	std::string attr;
	udev_list_entry_foreach( devs_list_entry, devs ) {
		attr = DevicesManager::getString( udev_list_entry_get_name( devs_list_entry ) );
		if( attr == "" )
			continue;
		if( subsystem == "input" )
			value = udev_device_get_property_value(dev, attr.c_str());
		else if( subsystem == "hidraw" )
			value = udev_device_get_sysattr_value(dev, attr.c_str());

		LOG(DEBUG4) << attr << " : " << value;
	}
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "--";
}
#endif

void DevicesManager::searchSupportedDevices(void) {
	LOG(DEBUG2) << "DevicesManager::searchSupportedDevices()";

	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;

	enumerate = udev_enumerate_new(this->udev);
	if ( enumerate == nullptr )
		throw GLogiKExcept("udev enumerate object creation failure");

	try {
		// ---
		// ---
		// ---

		//LOG(DEBUG2) << "input enumerate";
		if( udev_enumerate_add_match_subsystem(enumerate, "input") < 0 )
			throw GLogiKExcept("input enumerate filtering init failure");

		if( udev_enumerate_scan_devices(enumerate) < 0 )
			throw GLogiKExcept("enumerate_scan_devices failure");

		devices = udev_enumerate_get_list_entry(enumerate);
		if( devices == nullptr )
			throw GLogiKExcept("devices empty list or failure");

		udev_list_entry_foreach(dev_list_entry, devices) {
			// Get the filename of the /sys entry for the device
			// and create a udev_device object (dev) representing it
			std::string path = this->getString( udev_list_entry_get_name(dev_list_entry) );
			if( path == "" )
				throw GLogiKExcept("entry_get_name failure");

			struct udev_device *dev = udev_device_new_from_syspath(this->udev, path.c_str());
			if( dev == nullptr )
				throw GLogiKExcept("new_from_syspath failure");

			std::string devss = this->getString( udev_device_get_subsystem(dev) );
			if( devss == "" ) {
				udev_device_unref(dev);
				throw GLogiKExcept("get_subsystem failure");
			}

			if( udev_device_get_property_value(dev, "ID_INPUT_KEYBOARD") == nullptr ) {
				udev_device_unref(dev);
				continue;
			}

			/* GLOGIKD_GLOBAL_DEBUG */
			//deviceProperties(dev, devss);

			std::string vendor_id = this->getString( udev_device_get_property_value(dev, "ID_VENDOR_ID") );
			std::string product_id = this->getString( udev_device_get_property_value(dev, "ID_MODEL_ID") );
			if( (vendor_id == "") or (product_id == "") ) {
				udev_device_unref(dev);
				continue;
			}

			for(const auto& driver : this->drivers_) {
				for(const auto& device : driver->getSupportedDevices()) {
					if( device.vendor_id == vendor_id )
						if( device.product_id == product_id ) {

							// path to the event device node in /dev/input/
							std::string devnode = this->getString( udev_device_get_devnode(dev) );
							if( devnode == "" ) {
								udev_device_unref(dev);
								continue;
							}

							/* GLOGIKD_GLOBAL_DEBUG */
							//deviceProperties(dev, devss);

							std::string vendor = this->getString( udev_device_get_property_value(dev, "ID_VENDOR") );
							std::string model = this->getString( udev_device_get_property_value(dev, "ID_MODEL") );
							std::string serial = this->getString( udev_device_get_property_value(dev, "ID_SERIAL") );
							std::string usec = this->getString( udev_device_get_property_value(dev, "USEC_INITIALIZED") );

							DetectedDevice found;
							found.name				= device.name;
							found.vendor_id			= vendor_id;
							found.product_id		= product_id;
							found.input_dev_node	= devnode;
							found.vendor			= vendor;
							found.model				= model;
							found.serial			= serial;
							found.usec				= usec;
							found.driver_ID			= driver->getDriverID();

							this->detected_devices_.push_back(found);

							LOG(DEBUG3) << "Vid:Pid:DevNode:usec : " << vendor_id
										<< ":" << product_id << ":" << devnode << ":" << usec;
						}
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
	LOG(DEBUG2) << "DevicesManager::startMonitoring()";

	this->udev = udev_new();
	if ( this->udev == nullptr )
		throw GLogiKExcept("udev context init failure");

	this->monitor = udev_monitor_new_from_netlink(this->udev, "udev");
	if( this->monitor == nullptr )
		throw GLogiKExcept("allocating udev monitor failure");
/*
	if( udev_monitor_filter_add_match_subsystem_devtype(this->monitor, "hidraw", nullptr) < 0 )
		throw GLogiKExcept("hidraw monitor filtering init failure");
*/
	if( udev_monitor_filter_add_match_subsystem_devtype(this->monitor, "input", nullptr) < 0 )
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
			if( dev == nullptr )
				throw GLogiKExcept("no device from receive_device(), something is wrong");

			std::string action = this->getString( udev_device_get_action(dev) );
			if( action == "" )
				throw GLogiKExcept("device_get_action() failure");

			std::string devnode = this->getString( udev_device_get_devnode(dev) );
			if( devnode == "" ) // filtering empty events
				continue;

			//if( devnode == "/dev/input/event21" )
			//	continue;

			if( action == "add" ) {
				this->searchSupportedDevices();
				this->initializeDrivers();
			}
			if( action == "remove" ) {
				this->searchSupportedDevices();
				this->cleanDrivers();
			}

			udev_device_unref(dev);
		}
	}

}

} // namespace GLogiKd

