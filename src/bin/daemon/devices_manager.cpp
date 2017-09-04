/*
 *
 *	This file is part of GLogiK project.
 *	GLogiKd, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2017  Fabrice Delliaux <netbox253@gmail.com>
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <iostream>
#include <stdexcept>
#include <string>

#include <libudev.h>
#include <syslog.h>

#include "lib/utils/exception.h"
#include "include/log.h"
#include "daemon_control.h"

#include "devices_manager.h"

#include "logitech_G510.h"

namespace GLogiK
{

DevicesManager::DevicesManager() : buffer_("", std::ios_base::app) {
	this->fds[0].fd = -1;
	this->fds[0].events = POLLIN;
}

DevicesManager::~DevicesManager() {
	LOG(DEBUG2) << "exiting devices manager";
	if(this->monitor) {
		LOG(DEBUG3) << "unref monitor object";
		udev_monitor_unref(this->monitor);
	}
	if(this->udev) {
		LOG(DEBUG3) << "unref udev context";
		udev_unref(this->udev);
	}

	this->closeInitializedDevices();

	LOG(DEBUG2) << "closing drivers";
	for(const auto& driver : this->drivers_) {
		delete driver;
	}
	this->drivers_.clear();
}

void DevicesManager::initializeDevices(void) {
	LOG(DEBUG2) << "initializing detected devices";
	for(const auto& det_dev : this->detected_devices_) {

		bool initializing = true;

		for(const auto& init_dev : this->initialized_devices_) {
				if(	(det_dev.device.vendor_id == init_dev.device.vendor_id) and
					(det_dev.device.product_id == init_dev.device.product_id) and
					(det_dev.input_dev_node == init_dev.input_dev_node) and
					(det_dev.usec == init_dev.usec) ) {
						LOG(DEBUG3) << "device "
									<< det_dev.device.vendor_id << ":"
									<< det_dev.device.product_id << " - "
									<< det_dev.input_dev_node << " already initialized";
						initializing = false;
						break; // jump to next detected device
				}
		} // for

		try {
			if( initializing ) {
				for(const auto& driver : this->drivers_) {
					if( det_dev.driver_ID == driver->getDriverID() ) {
						// initialization
						driver->initializeDevice( det_dev.device, det_dev.device_bus, det_dev.device_num );
						this->initialized_devices_.push_back( det_dev );

						this->buffer_.str( det_dev.device.name );
						this->buffer_	<< "(" << det_dev.device.vendor_id << ":" << det_dev.device.product_id
										<< ") on bus " << to_uint(det_dev.device_bus) << " initialized";
						LOG(INFO) << this->buffer_.str();
						syslog(LOG_INFO, this->buffer_.str().c_str());
						break;
					}
				} // for
			}
		}
		catch ( const GLogiKExcept & e ) {
			this->buffer_.str("device initialization failure : ");
			this->buffer_ << e.what();
			LOG(ERROR) << this->buffer_.str();
			syslog(LOG_ERR, this->buffer_.str().c_str());
		}
	} // for
	this->detected_devices_.clear();
	LOG(INFO) << "device(s) initialized : " << this->initialized_devices_.size();
}


void DevicesManager::closeInitializedDevices(void) {
	LOG(DEBUG2) << "closing initialized devices";

	for(const auto& init_dev : this->initialized_devices_) {
		for(const auto& driver : this->drivers_) {
			try {
				if( init_dev.driver_ID == driver->getDriverID() ) {
					driver->closeDevice( init_dev.device, init_dev.device_bus, init_dev.device_num );

					this->buffer_.str( init_dev.device.name );
					this->buffer_	<< "(" << init_dev.device.vendor_id << ":" << init_dev.device.product_id
									<< ") on bus " << to_uint(init_dev.device_bus) << " closed";
					LOG(INFO) << this->buffer_.str();
					syslog(LOG_INFO, this->buffer_.str().c_str());
				}
			}
			catch ( const GLogiKExcept & e ) {
				syslog( LOG_ERR, e.what() );
				LOG(ERROR) << e.what();
			}
		}
	}
	this->initialized_devices_.clear();
}

void DevicesManager::cleanUnpluggedDevices(void) {
	LOG(DEBUG2) << "checking for unplugged initialized devices";
	for(auto it = this->initialized_devices_.begin(); it != this->initialized_devices_.end();) {
		bool stop_it = true;
		for(const auto& det_dev : this->detected_devices_) {
			if( ((*it).input_dev_node == det_dev.input_dev_node) and ((*it).usec == det_dev.usec )
				and ((*it).device.vendor_id == det_dev.device.vendor_id )
				and ((*it).device.product_id == det_dev.device.product_id ) ) {
				stop_it = false;
				++it;
				break;
			}
		}

		try {
			if(stop_it) {
#if DEBUGGING_ON
				LOG(WARNING)	<< "erasing unplugged initialized driver : "
								<< (*it).device.vendor_id << ":" << (*it).device.product_id
								<< ":" << (*it).input_dev_node << ":" << (*it).usec;
				LOG(WARNING)	<< "Did you unplug your device before properly closing it ?";
				LOG(WARNING)	<< "You will get libusb warnings/errors if you do this.";
#endif

				for(const auto& driver : this->drivers_) {
					if( (*it).driver_ID == driver->getDriverID() ) {
						this->buffer_.str("warning : device closing attempt ");
						this->buffer_ << (*it).device.vendor_id << ":" << (*it).device.product_id
									<< ":" << (*it).input_dev_node << ":" << (*it).usec;
						LOG(WARNING) << this->buffer_.str();
						syslog(LOG_WARNING, this->buffer_.str().c_str());
						driver->closeDevice( (*it).device, (*it).device_bus, (*it).device_num );
						break;
					}
				} // for

				it = this->initialized_devices_.erase(it);
			}
		}
		catch ( const GLogiKExcept & e ) {
			syslog( LOG_ERR, e.what() );
			LOG(ERROR) << e.what();
			it = this->initialized_devices_.erase(it);
		}
	}

	this->detected_devices_.clear();
	LOG(DEBUG3) << "number of devices still initialized : " << this->initialized_devices_.size();
	LOG(DEBUG3) << "---";
}

#if DEBUGGING_ON
void deviceProperties(struct udev_device *dev, const std::string &subsystem) {
	struct udev_list_entry *devs_props = nullptr;
	struct udev_list_entry *devs_attr = nullptr;
	struct udev_list_entry *devs_list_entry = nullptr;

	devs_props = udev_device_get_properties_list_entry( dev );
	devs_attr = udev_device_get_sysattr_list_entry( dev );

	LOG(DEBUG4) << "--";
	LOG(DEBUG4)	<< "Device properties";
	LOG(DEBUG4)	<< "--";

	std::string value;
	std::string attr;
	udev_list_entry_foreach( devs_list_entry, devs_props ) {
		attr = DevicesManager::toString( udev_list_entry_get_name( devs_list_entry ) );
		if( attr == "" )
			continue;

		value = DevicesManager::toString( udev_device_get_property_value(dev, attr.c_str()) );
		LOG(DEBUG4) << attr << " : " << value;
	}
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "/sys attributes";
	LOG(DEBUG4) << "--";
	udev_list_entry_foreach( devs_list_entry, devs_attr ) {
		attr = DevicesManager::toString( udev_list_entry_get_name( devs_list_entry ) );
		if( attr == "" )
			continue;

		value = DevicesManager::toString( udev_device_get_sysattr_value(dev, attr.c_str()) );
		LOG(DEBUG4) << attr << " : " << value;
	}
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "--";
}
#endif

void DevicesManager::searchSupportedDevices(void) {
	LOG(DEBUG2) << "searching for supported devices";

	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;

	enumerate = udev_enumerate_new(this->udev);
	if ( enumerate == nullptr )
		throw GLogiKExcept("usb enumerate object creation failure");

	try {
		// ---
		// ---
		// ---

		//LOG(DEBUG2) << "input enumerate";
		if( udev_enumerate_add_match_subsystem(enumerate, "usb") < 0 )
			throw GLogiKExcept("usb enumerate filtering init failure");

		if( udev_enumerate_scan_devices(enumerate) < 0 )
			throw GLogiKExcept("usb numerate_scan_devices failure");

		devices = udev_enumerate_get_list_entry(enumerate);
		if( devices == nullptr )
			throw GLogiKExcept("usb devices empty list or failure");

		udev_list_entry_foreach(dev_list_entry, devices) {
			// Get the filename of the /sys entry for the device
			// and create a udev_device object (dev) representing it
			std::string path = this->toString( udev_list_entry_get_name(dev_list_entry) );
			if( path == "" )
				throw GLogiKExcept("entry_get_name failure");

			struct udev_device *dev = udev_device_new_from_syspath(this->udev, path.c_str());
			if( dev == nullptr ) {
				LOG(DEBUG3) << "new_from_syspath failure with path : " << path;
				continue;
			}

#if DEBUGGING_ON
			std::string devss = this->toString( udev_device_get_subsystem(dev) );
			if( devss == "" ) {
				udev_device_unref(dev);
				throw GLogiKExcept("get_subsystem failure");
			}

			//deviceProperties(dev, devss);
#endif

			std::string vendor_id = this->toString( udev_device_get_property_value(dev, "ID_VENDOR_ID") );
			std::string product_id = this->toString( udev_device_get_property_value(dev, "ID_MODEL_ID") );
			if( (vendor_id == "") or (product_id == "") ) {
				udev_device_unref(dev);
				continue;
			}

			for(const auto& driver : this->drivers_) {
				for(const auto& device : driver->getSupportedDevices()) {
					if( device.vendor_id == vendor_id )
						if( device.product_id == product_id ) {

							// path to the event device node in /dev
							std::string devnode = this->toString( udev_device_get_devnode(dev) );
							if( devnode == "" ) {
								udev_device_unref(dev);
								continue;
							}

#if DEBUGGING_ON
							deviceProperties(dev, devss);
#endif

							std::string vendor = this->toString( udev_device_get_property_value(dev, "ID_VENDOR") );
							std::string model = this->toString( udev_device_get_property_value(dev, "ID_MODEL") );
							std::string serial = this->toString( udev_device_get_property_value(dev, "ID_SERIAL") );
							std::string usec = this->toString( udev_device_get_property_value(dev, "USEC_INITIALIZED") );

							uint8_t bus, num = 0;

							try {
								bus = std::stoi(this->toString( udev_device_get_sysattr_value(dev, "busnum")));
								num = std::stoi(this->toString( udev_device_get_sysattr_value(dev, "devnum")));
							}
							catch (const std::invalid_argument& ia) {
								udev_device_unref(dev);
								throw GLogiKExcept("stoi invalid argument");
							}
							catch (const std::out_of_range& oor) {
								udev_device_unref(dev);
								throw GLogiKExcept("stoi out of range");
							}

							DetectedDevice found;
							found.device			= device;
							found.input_dev_node	= devnode;
							found.vendor			= vendor;
							found.model				= model;
							found.serial			= serial;
							found.usec				= usec;
							found.driver_ID			= driver->getDriverID();
							found.device_bus		= bus;
							found.device_num		= num;

							this->detected_devices_.push_back(found);

							LOG(DEBUG3) << "found device - Vid:Pid:DevNode:usec | bus:num : " << vendor_id
										<< ":" << product_id << ":" << devnode << ":" << usec
										<< " | " << to_uint(bus) << ":" << to_uint(num);
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

	LOG(DEBUG2) << "found " << this->detected_devices_.size() << " device(s)";

	// Free the enumerator object
	udev_enumerate_unref(enumerate);
}

void DevicesManager::startMonitoring(DBus* GKDBus) {
	LOG(DEBUG2) << "initializing libudev";

	this->udev = udev_new();
	if ( this->udev == nullptr )
		throw GLogiKExcept("udev context init failure");

	this->monitor = udev_monitor_new_from_netlink(this->udev, "udev");
	if( this->monitor == nullptr )
		throw GLogiKExcept("allocating udev monitor failure");

	if( udev_monitor_filter_add_match_subsystem_devtype(this->monitor, "usb", nullptr) < 0 )
		throw GLogiKExcept("usb monitor filtering init failure");

	if( udev_monitor_enable_receiving(this->monitor) < 0 )
		throw GLogiKExcept("monitor enabling failure");

	this->fd_ = udev_monitor_get_fd(this->monitor);
	if( this->fd_ < 0 )
		throw GLogiKExcept("can't get the monitor file descriptor");

	this->fds[0].fd = this->fd_;
	this->fds[0].events = POLLIN;

	//GKDBus->addSignalMatch(BusConnection::GKDBUS_SESSION, "test.signal.Type");

	LOG(DEBUG2) << "loading known drivers";

	this->drivers_.push_back( new LogitechG510() );

	this->searchSupportedDevices();
	this->initializeDevices();

	while( DaemonControl::is_daemon_enabled() ) {
		int ret = poll(this->fds, 1, 1000);

		if( GKDBus->checkForNextMessage(BusConnection::GKDBUS_SESSION) ) {
			if( GKDBus->checkMessageForMethodCallOnInterface("com.glogik.Daemon.Device", "Stop") ) {
				LOG(DEBUG) << "Stop called !";

				try {
					LOG(INFO) << GKDBus->getNextStringArgument();
				}
				catch ( const EmptyContainer & e ) {
					LOG(DEBUG3) << e.what();
				}

				try {
					GKDBus->initializeMethodCallReply(BusConnection::GKDBUS_SESSION);
					GKDBus->appendToMethodCallReply(true);
				}
				catch (const std::bad_alloc& e) { /* handle new() failure */
					LOG(ERROR) << "GKDBus reply allocation failure : " << e.what();
				}
				catch ( const GLogiKExcept & e ) {
					LOG(ERROR) << "GKDBus reply failure : " << e.what();
				}

				/* delete reply object if allocated */
				GKDBus->sendMethodCallReply();
			}
/*
			if( GKDBus->checkMessageForSignalOnInterface("test.signal.Type", "Test") ) {
				try {
					LOG(INFO) << GKDBus->getNextStringArgument();
					LOG(INFO) << GKDBus->getNextStringArgument();
				}
				catch ( const EmptyContainer & e ) {
					LOG(DEBUG3) << e.what();
				}
			}
*/
		}

		// receive data ?
		if( ret > 0 ) {
			struct udev_device *dev = udev_monitor_receive_device(this->monitor);
			if( dev == nullptr )
				throw GLogiKExcept("no device from receive_device(), something is wrong");

			std::string action = this->toString( udev_device_get_action(dev) );
			if( action == "" )
				throw GLogiKExcept("device_get_action() failure");

			std::string devnode = this->toString( udev_device_get_devnode(dev) );
			if( devnode == "" ) // filtering empty events
				continue;

			LOG(DEBUG3) << "Action : " << action;
			this->searchSupportedDevices();

			if( action == "add" ) {
				this->initializeDevices();
			}
			else if( action == "remove" ) {
				this->cleanUnpluggedDevices();
			}

			udev_device_unref(dev);
		}
	}

}

} // namespace GLogiK

