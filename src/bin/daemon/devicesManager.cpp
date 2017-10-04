/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
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
#include <functional>
#include <thread>
#include <chrono>

#include <libudev.h>
#include <syslog.h>

#include "lib/utils/utils.h"

#include "daemonControl.h"
#include "logitechG510.h"
#include "devicesManager.h"


namespace GLogiK
{

DevicesManager::DevicesManager() : buffer_("", std::ios_base::app) {
	LOG(DEBUG2) << "initializing devices manager";
	this->fds[0].fd = -1;
	this->fds[0].events = POLLIN;
}

DevicesManager::~DevicesManager() {
	LOG(DEBUG2) << "destroying devices manager";

	this->stopInitializedDevices();
	this->plugged_but_stopped_devices_.clear();

	LOG(DEBUG2) << "stopping drivers";
	for(const auto& driver : this->drivers_) {
		delete driver;
	}
	this->drivers_.clear();

	if(this->monitor) {
		LOG(DEBUG3) << "unref monitor object";
		udev_monitor_unref(this->monitor);
	}
	if(this->udev) {
		LOG(DEBUG3) << "unref udev context";
		udev_unref(this->udev);
	}

	LOG(DEBUG2) << "exiting devices manager";
}

void DevicesManager::initializeDevices(void) {
	LOG(DEBUG2) << "initializing detected devices";
	for(const auto& det_dev : this->detected_devices_) {
		const auto & devID = det_dev.first;
		const auto & device = det_dev.second;

		if(this->initialized_devices_.count(devID) == 1) {
			LOG(DEBUG3) << "device "
						<< device.device.vendor_id << ":"
						<< device.device.product_id << " - "
						<< device.input_dev_node << " already initialized";
			continue; // jump to next detected device
		}

		// TODO option ?
		if(this->plugged_but_stopped_devices_.count(devID) == 1) {
			LOG(DEBUG3) << "device "
						<< device.device.vendor_id << ":"
						<< device.device.product_id << " - "
						<< device.input_dev_node << " has been stopped";
			LOG(DEBUG3) << "automatic initialization is disabled for stopped devices";
			continue; // jump to next detected device
		}

		try {
			for(const auto& driver : this->drivers_) {
				if( device.driver_ID == driver->getDriverID() ) {
					// initialization
					driver->initializeDevice( device.device, device.device_bus, device.device_num );
					this->initialized_devices_[devID] = device;

					this->buffer_.str( device.device.name );
					this->buffer_	<< "(" << device.device.vendor_id << ":" << device.device.product_id
									<< ") on bus " << to_uint(device.device_bus) << " initialized";
					LOG(INFO) << this->buffer_.str();
					syslog(LOG_INFO, this->buffer_.str().c_str());
					break;
				}
			} // for
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

void DevicesManager::sendSignalToClients(const std::string & signal) {
	try {
		this->DBus->initializeBroadcastSignal(BusConnection::GKDBUS_SYSTEM,
			this->DBus_CSMH_object_path_, this->DBus_CSMH_interface_,
			signal.c_str());
		this->DBus->sendBroadcastSignal();
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << "DBus signal failure : " << e.what();
	}
}

const bool DevicesManager::startDevice(const std::string & devID) {
	LOG(DEBUG2) << "trying to start device " << devID;
	try {
		auto & device = this->plugged_but_stopped_devices_.at(devID);
		for(const auto& driver : this->drivers_) {
			if( device.driver_ID == driver->getDriverID() ) {
				if( ! driver->isDeviceInitialized(devID) ) { /* sanity check */
					// initialization
					driver->initializeDevice( device.device, device.device_bus, device.device_num );
					this->initialized_devices_[devID] = device;

					this->buffer_.str( device.device.name );
					this->buffer_	<< "(" << device.device.vendor_id << ":" << device.device.product_id
									<< ") on bus " << to_uint(device.device_bus) << " initialized";
					LOG(INFO) << this->buffer_.str();
					syslog(LOG_INFO, this->buffer_.str().c_str());

					LOG(DEBUG3) << "removing " << devID << " from plugged-but-stopped devices";
					this->plugged_but_stopped_devices_.erase(devID);

					/* inform clients */
					this->sendSignalToClients("SomethingChanged");

					return true;
				}
			}
		}
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("device starting failure : device not found in plugged-but-stopped devices : ");
		this->buffer_ << oor.what();
		LOG(ERROR) << this->buffer_.str();
		syslog(LOG_ERR, this->buffer_.str().c_str());
		return false;
	}

	this->buffer_.str("device starting failure : driver not found !?");
	LOG(ERROR) << this->buffer_.str();
	syslog(LOG_ERR, this->buffer_.str().c_str());
	return false;
}

const bool DevicesManager::stopDevice(const std::string & devID) {
	LOG(DEBUG2) << "trying to stop device " << devID;

	try {
		const auto & device = this->initialized_devices_.at(devID);
		for(const auto& driver : this->drivers_) {
			if( device.driver_ID == driver->getDriverID() ) {
				if( driver->isDeviceInitialized(devID) ) {
					driver->closeDevice( device.device, device.device_bus, device.device_num );

					this->buffer_.str( device.device.name );
					this->buffer_	<< "(" << device.device.vendor_id << ":" << device.device.product_id
									<< ") on bus " << to_uint(device.device_bus) << " stopped";
					LOG(INFO) << this->buffer_.str();
					syslog(LOG_INFO, this->buffer_.str().c_str());

					this->plugged_but_stopped_devices_[devID] = device;
					this->initialized_devices_.erase(devID);

					/* inform clients */
					this->sendSignalToClients("SomethingChanged");

					return true;
				}
			}
		}
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("device stopping failure : device not found in initialized devices : ");
		this->buffer_ << oor.what();
		LOG(ERROR) << this->buffer_.str();
		syslog(LOG_ERR, this->buffer_.str().c_str());
		return false;
	}

	this->buffer_.str("device stopping failure : driver not found !?");
	LOG(ERROR) << this->buffer_.str();
	syslog(LOG_ERR, this->buffer_.str().c_str());
	return false;
}

const bool DevicesManager::restartDevice(const std::string & devID) {
	LOG(DEBUG1) << "trying to restart device " << devID;
	if(this->stopDevice(devID)) {
#if DEBUGGING_ON
		LOG(DEBUG) << "device restart : sleeping for 200 ms";
#endif
		std::this_thread::sleep_for(std::chrono::milliseconds(200));

		if(this->startDevice(devID)) {
			return true;
		}

		this->buffer_.str("device restarting failure : start failed");
		LOG(ERROR) << this->buffer_.str();
		syslog(LOG_ERR, this->buffer_.str().c_str());
		return false;
	}

	this->buffer_.str("device restarting failure : stop failed");
	LOG(ERROR) << this->buffer_.str();
	syslog(LOG_ERR, this->buffer_.str().c_str());
	return false;
}

void DevicesManager::stopInitializedDevices(void) {
	LOG(DEBUG2) << "stopping initialized devices";

	std::vector<std::string> to_stop;
	for(const auto& init_dev : this->initialized_devices_) {
		to_stop.push_back(init_dev.first);
	}

	for(const auto & devID : to_stop) {
		this->stopDevice(devID);
	}

	to_stop.clear();
	this->initialized_devices_.clear();
}

void DevicesManager::checkForUnpluggedDevices(void) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "checking for unplugged devices";
#endif

	/* checking for unplugged unstopped devices */
	std::vector<std::string> to_clean;
	for(const auto & init_dev : this->initialized_devices_) {
		if( this->detected_devices_.count(init_dev.first) == 0 ) {
			to_clean.push_back(init_dev.first);
		}
	}

	for(const auto & devID : to_clean) {
#if DEBUGGING_ON
		const auto & device = this->initialized_devices_.at(devID);
		LOG(WARNING)	<< "erasing unplugged initialized driver : "
						<< device.device.vendor_id << ":" << device.device.product_id
						<< ":" << device.input_dev_node << ":" << device.usec;
		LOG(WARNING)	<< "Did you unplug your device before properly stopping it ?";
		LOG(WARNING)	<< "You will get libusb warnings/errors if you do this.";
#endif
		this->stopDevice(devID);
	}

	to_clean.clear();

	/* checking for unplugged but stopped devices */
	for(const auto & stopped_dev : this->plugged_but_stopped_devices_) {
		if(this->detected_devices_.count(stopped_dev.first) == 0 ) {
			to_clean.push_back(stopped_dev.first);
		}
	}
	for(const auto & devID : to_clean) {
#if DEBUGGING_ON
		LOG(DEBUG3) << "removing " << devID << " from plugged-but-stopped devices";
#endif
		this->plugged_but_stopped_devices_.erase(devID);
	}
	to_clean.clear();

	this->detected_devices_.clear();

#if DEBUGGING_ON
	LOG(DEBUG3) << "number of devices still initialized : " << this->initialized_devices_.size();
#endif
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
		attr = to_string( udev_list_entry_get_name( devs_list_entry ) );
		if( attr == "" )
			continue;

		value = to_string( udev_device_get_property_value(dev, attr.c_str()) );
		LOG(DEBUG4) << attr << " : " << value;
	}
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "/sys attributes";
	LOG(DEBUG4) << "--";
	udev_list_entry_foreach( devs_list_entry, devs_attr ) {
		attr = to_string( udev_list_entry_get_name( devs_list_entry ) );
		if( attr == "" )
			continue;

		value = to_string( udev_device_get_sysattr_value(dev, attr.c_str()) );
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
			std::string path = to_string( udev_list_entry_get_name(dev_list_entry) );
			if( path == "" )
				throw GLogiKExcept("entry_get_name failure");

			struct udev_device *dev = udev_device_new_from_syspath(this->udev, path.c_str());
			if( dev == nullptr ) {
				LOG(DEBUG3) << "new_from_syspath failure with path : " << path;
				continue;
			}

#if DEBUGGING_ON
			std::string devss = to_string( udev_device_get_subsystem(dev) );
			if( devss == "" ) {
				udev_device_unref(dev);
				throw GLogiKExcept("get_subsystem failure");
			}

			//deviceProperties(dev, devss);
#endif

			std::string vendor_id = to_string( udev_device_get_property_value(dev, "ID_VENDOR_ID") );
			std::string product_id = to_string( udev_device_get_property_value(dev, "ID_MODEL_ID") );
			if( (vendor_id == "") or (product_id == "") ) {
				udev_device_unref(dev);
				continue;
			}

			for(const auto& driver : this->drivers_) {
				for(const auto& device : driver->getSupportedDevices()) {
					if( device.vendor_id == vendor_id )
						if( device.product_id == product_id ) {

							// path to the event device node in /dev
							std::string devnode = to_string( udev_device_get_devnode(dev) );
							if( devnode == "" ) {
								udev_device_unref(dev);
								continue;
							}

#if DEBUGGING_ON
							deviceProperties(dev, devss);
#endif

							std::string vendor = to_string( udev_device_get_property_value(dev, "ID_VENDOR") );
							std::string model = to_string( udev_device_get_property_value(dev, "ID_MODEL") );
							std::string serial = to_string( udev_device_get_property_value(dev, "ID_SERIAL") );
							std::string usec = to_string( udev_device_get_property_value(dev, "USEC_INITIALIZED") );

							uint8_t bus, num = 0;

							try {
								bus = std::stoi(to_string( udev_device_get_sysattr_value(dev, "busnum")));
								num = std::stoi(to_string( udev_device_get_sysattr_value(dev, "devnum")));
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

							const std::string devID = KeyboardDriver::getDeviceID(bus, num);
							if( this->detected_devices_.count(devID) > 0 ) { /* sanity check */
								udev_device_unref(dev);
								throw GLogiKExcept("detected devices container error");
							}
							this->detected_devices_[devID] = found;

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

const std::vector<std::string> DevicesManager::getStartedDevices(void) {
	std::vector<std::string> ret;

	for(const auto& init_dev : this->initialized_devices_) {
		ret.push_back(init_dev.first);
	}

	return ret;
}

void DevicesManager::checkDBusMessages(void) {
	if( this->DBus->checkForNextMessage(BusConnection::GKDBUS_SYSTEM) ) {
		try {
			this->DBus->checkForMethodCall(BusConnection::GKDBUS_SYSTEM);
			this->DBus->freeMessage();
		}
		catch ( const GLogiKExcept & e ) {
			this->DBus->freeMessage();
			throw;
		}
	}
}

void DevicesManager::startMonitoring(GKDBus* pDBus) {
	LOG(DEBUG2) << "initializing libudev";
	this->DBus = pDBus;

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

	{
		this->DBus->addEvent_StringToBool_Callback( this->DBus_object_, this->DBus_interface_, "Stop",
			{	{"s", "device_id", "in", "device ID coming from ..."}, // FIXME
				{"b", "did_stop_succeeded", "out", "did the Stop method succeeded ?"} },
			std::bind(&DevicesManager::stopDevice, this, std::placeholders::_1) );

		this->DBus->addEvent_StringToBool_Callback( this->DBus_object_, this->DBus_interface_, "Start",
			{	{"s", "device_id", "in", "device ID coming from ..."}, // FIXME
				{"b", "did_start_succeeded", "out", "did the Start method succeeded ?"} },
			std::bind(&DevicesManager::startDevice, this, std::placeholders::_1) );

		this->DBus->addEvent_StringToBool_Callback( this->DBus_object_, this->DBus_interface_, "Restart",
			{	{"s", "device_id", "in", "device ID coming from ..."}, // FIXME
				{"b", "did_restart_succeeded", "out", "did the Restart method succeeded ?"} },
			std::bind(&DevicesManager::restartDevice, this, std::placeholders::_1) );
	}

	LOG(DEBUG2) << "loading known drivers";

	this->drivers_.push_back( new LogitechG510() );

	this->searchSupportedDevices();
	this->initializeDevices();

	while( DaemonControl::is_daemon_enabled() ) {
		int ret = poll(this->fds, 1, 1000);

		// receive data ?
		if( ret > 0 ) {
			struct udev_device *dev = udev_monitor_receive_device(this->monitor);
			if( dev == nullptr )
				throw GLogiKExcept("no device from receive_device(), something is wrong");

			try {
				std::string action = to_string( udev_device_get_action(dev) );
				if( action == "" ) {
					throw GLogiKExcept("device_get_action() failure");
				}

				std::string devnode = to_string( udev_device_get_devnode(dev) );
				// filtering empty events
				if( devnode != "" ) {
					LOG(DEBUG3) << "Action : " << action;
					this->searchSupportedDevices();

					if( action == "add" ) {
						this->initializeDevices();
					}
					else if( action == "remove" ) {
						this->checkForUnpluggedDevices();
					}
				}
			}
			catch ( const GLogiKExcept & e ) {
				udev_device_unref(dev);
				throw;
			}

			udev_device_unref(dev);
		}

		this->checkDBusMessages();
	}

}

} // namespace GLogiK

