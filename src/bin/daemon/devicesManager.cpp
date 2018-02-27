/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2018  Fabrice Delliaux <netbox253@gmail.com>
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

#include "lib/utils/utils.h"
#include "lib/shared/glogik.h"

#include "daemonControl.h"
#include "logitechG510.h"
#include "devicesManager.h"


namespace GLogiK
{

using namespace NSGKUtils;

DevicesManager::DevicesManager() : num_clients_(0), buffer_("", std::ios_base::app) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "initializing devices manager";
#endif

	this->fds[0].fd = -1;
	this->fds[0].events = POLLIN;
}

DevicesManager::~DevicesManager() {
#if DEBUGGING_ON
	LOG(DEBUG2) << "destroying devices manager";
#endif

	this->stopInitializedDevices();
	this->plugged_but_stopped_devices_.clear();

#if DEBUGGING_ON
	LOG(DEBUG2) << "stopping drivers";
#endif
	for(const auto& driver : this->drivers_) {
		delete driver;
	}
	this->drivers_.clear();

	if(this->monitor) {
#if DEBUGGING_ON
		LOG(DEBUG3) << "unref monitor object";
#endif
		udev_monitor_unref(this->monitor);
	}
	if(this->udev) {
#if DEBUGGING_ON
		LOG(DEBUG3) << "unref udev context";
#endif
		udev_unref(this->udev);
	}

#if DEBUGGING_ON
	LOG(DEBUG2) << "exiting devices manager";
#endif
}

void DevicesManager::setNumClients(uint8_t num) {
	this->num_clients_ = num;
}

void DevicesManager::initializeDevices(void) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "initializing detected devices";
#endif
	std::vector<std::string> startedDevices;

	for(const auto& det_dev : this->detected_devices_) {
		const auto & devID = det_dev.first;
		const auto & device = det_dev.second;

		if(this->initialized_devices_.count(devID) == 1) {
#if DEBUGGING_ON
			LOG(DEBUG3) << "device "
						<< device.device.vendor_id << ":"
						<< device.device.product_id << " - "
						<< device.input_dev_node << " already initialized";
#endif
			continue; // jump to next detected device
		}

		// TODO option ?
		if(this->plugged_but_stopped_devices_.count(devID) == 1) {
#if DEBUGGING_ON
			LOG(DEBUG3) << "device "
						<< device.device.vendor_id << ":"
						<< device.device.product_id << " - "
						<< device.input_dev_node << " has been stopped";
			LOG(DEBUG3) << "automatic initialization is disabled for stopped devices";
#endif
			continue; // jump to next detected device
		}

		try {
			for(const auto& driver : this->drivers_) {
				if( device.driver_ID == driver->getDriverID() ) {
					// initialization
					driver->initializeDevice( device.device, device.device_bus, device.device_num );
					this->initialized_devices_[devID] = device;

					this->buffer_.str( device.device.name );
					this->buffer_ << "(" << device.device.vendor_id << ":" << device.device.product_id
								  << ") on bus " << to_uint(device.device_bus) << " initialized";
					GKSysLog(LOG_INFO, INFO, this->buffer_.str());
					startedDevices.push_back(devID);
					break;
				}
			} // for
		}
		catch ( const GLogiKExcept & e ) {
			this->buffer_.str("device initialization failure : ");
			this->buffer_ << e.what();
			GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
		}
	} // for

	if( startedDevices.size() > 0 ) {
		/* inform clients */
		this->sendStatusSignalArrayToClients(this->num_clients_, this->pDBus_, "DevicesStarted", startedDevices);
	}

	this->detected_devices_.clear();
#if DEBUGGING_ON
	LOG(INFO) << "device(s) initialized : " << this->initialized_devices_.size();
#endif
}

const bool DevicesManager::startDevice(const std::string & devID) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "trying to start device " << devID;
#endif
	try {
		const auto & device = this->plugged_but_stopped_devices_.at(devID);
		for(const auto& driver : this->drivers_) {
			if( device.driver_ID == driver->getDriverID() ) {
				if( ! driver->isDeviceInitialized(devID) ) { /* sanity check */
					// initialization
					driver->initializeDevice( device.device, device.device_bus, device.device_num );
					this->initialized_devices_[devID] = device;

					this->buffer_.str( device.device.name );
					this->buffer_	<< "(" << device.device.vendor_id << ":" << device.device.product_id
									<< ") on bus " << to_uint(device.device_bus) << " initialized";
					GKSysLog(LOG_INFO, INFO, this->buffer_.str());

#if DEBUGGING_ON
					LOG(DEBUG3) << "removing " << devID << " from plugged-but-stopped devices";
#endif
					this->plugged_but_stopped_devices_.erase(devID);

					return true;
				}
			}
		}
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("device starting failure : device not found in plugged-but-stopped devices : ");
		this->buffer_ << oor.what();
		GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
		return false;
	}

	this->buffer_.str("device starting failure : driver not found !?");
	GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
	return false;
}

const bool DevicesManager::stopDevice(const std::string & devID) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "trying to stop device " << devID;
#endif

	try {
		const auto & device = this->initialized_devices_.at(devID);
		for(const auto& driver : this->drivers_) {
			if( device.driver_ID == driver->getDriverID() ) {
				if( driver->isDeviceInitialized(devID) ) {
					driver->closeDevice( device.device, device.device_bus, device.device_num );

					this->buffer_.str( device.device.name );
					this->buffer_	<< "(" << device.device.vendor_id << ":" << device.device.product_id
									<< ") on bus " << to_uint(device.device_bus) << " stopped";
					GKSysLog(LOG_INFO, INFO, this->buffer_.str());

					this->plugged_but_stopped_devices_[devID] = device;
					this->initialized_devices_.erase(devID);

					return true;
				}
			}
		}
	}
	catch (const std::out_of_range& oor) {
		this->buffer_.str("device stopping failure : device not found in initialized devices : ");
		this->buffer_ << oor.what();
		GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
		return false;
	}

	this->buffer_.str("device stopping failure : driver not found !?");
	GKSysLog(LOG_ERR, ERROR, this->buffer_.str());
	return false;
}

void DevicesManager::stopInitializedDevices(void) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "stopping initialized devices";
#endif

	std::vector<std::string> to_stop;
	for(const auto& device_pair : this->initialized_devices_) {
		to_stop.push_back(device_pair.first);
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

	std::vector<std::string> unpluggedDevices;

	/* checking for unplugged unstopped devices */
	std::vector<std::string> to_clean;
	for(const auto & device_pair : this->initialized_devices_) {
		if( this->detected_devices_.count(device_pair.first) == 0 ) {
			to_clean.push_back(device_pair.first);
		}
	}

	for(const auto & devID : to_clean) {
		try {
			this->buffer_.str("erasing unplugged initialized driver : ");

			const auto & device = this->initialized_devices_.at(devID);

			this->buffer_ << device.device.vendor_id << ":" << device.device.product_id
						  << ":" << device.input_dev_node << ":" << device.usec;

			GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
			GKSysLog(LOG_WARNING, WARNING, "Did you unplug your device before properly stopping it ?");
			GKSysLog(LOG_WARNING, WARNING, "You will get libusb warnings/errors if you do this.");

			if( this->stopDevice(devID) ) {
				this->plugged_but_stopped_devices_.erase(devID);
				this->unplugged_devices_.insert(devID);
				unpluggedDevices.push_back(devID);
			}
		}
		catch (const std::out_of_range& oor) {
			this->buffer_ << "!?! device not found !?! " << devID;
			GKSysLog(LOG_WARNING, WARNING, this->buffer_.str());
		}
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
		this->unplugged_devices_.insert(devID);
		unpluggedDevices.push_back(devID);
	}
	to_clean.clear();

	this->detected_devices_.clear();

#if DEBUGGING_ON
	LOG(DEBUG3) << "number of devices still initialized : " << this->initialized_devices_.size();
	LOG(DEBUG3) << "number of unplugged devices : " << unpluggedDevices.size();
#endif

	if(unpluggedDevices.size() > 0) {
		/* inform clients */
		this->sendStatusSignalArrayToClients(this->num_clients_, this->pDBus_, "DevicesUnplugged", unpluggedDevices);
	}
}

#if DEBUGGING_ON
void udevDeviceProperties(struct udev_device *dev, const std::string &subsystem) {
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
#if DEBUGGING_ON
	LOG(DEBUG2) << "searching for supported devices";
#endif

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
#if DEBUGGING_ON
				LOG(DEBUG3) << "new_from_syspath failure with path : " << path;
#endif
				continue;
			}

#if DEBUGGING_ON
			std::string devss = to_string( udev_device_get_subsystem(dev) );
			if( devss == "" ) {
				udev_device_unref(dev);
				throw GLogiKExcept("get_subsystem failure");
			}

			//udevDeviceProperties(dev, devss);
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
							udevDeviceProperties(dev, devss);
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

#if DEBUGGING_ON
							LOG(DEBUG3) << "found device - Vid:Pid:DevNode:usec | bus:num : " << vendor_id
										<< ":" << product_id << ":" << devnode << ":" << usec
										<< " | " << to_uint(bus) << ":" << to_uint(num);
#endif
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

#if DEBUGGING_ON
	LOG(DEBUG2) << "found " << this->detected_devices_.size() << " device(s)";
#endif

	// Free the enumerator object
	udev_enumerate_unref(enumerate);
}

const std::vector<std::string> DevicesManager::getStartedDevices(void) const {
	std::vector<std::string> ret;

	// dev code
	//std::vector<std::string> ret = {"aaa1", "bbb2", "ccc3"};

	for(const auto& device_pair : this->initialized_devices_) {
		ret.push_back(device_pair.first);
	}

	return ret;
}

const std::vector<std::string> DevicesManager::getStoppedDevices(void) const {
	std::vector<std::string> ret;

	for(const auto& stopped_dev : this->plugged_but_stopped_devices_) {
		ret.push_back(stopped_dev.first);
	}

	return ret;
}

const std::vector<std::string> DevicesManager::getDeviceProperties(const std::string & devID) {
	std::vector<std::string> ret;

	try {
		const auto & device = this->initialized_devices_.at(devID);
#if DEBUGGING_ON
		LOG(DEBUG2) << "found " << device.model << " in started devices";
#endif
		ret.push_back(device.vendor);
		ret.push_back(device.model);
	}
	catch (const std::out_of_range& oor) {
		try {
			const auto & device = this->plugged_but_stopped_devices_.at(devID);
#if DEBUGGING_ON
			LOG(DEBUG2) << "found " << device.model << " in stopped devices";
#endif
			ret.push_back(device.vendor);
			ret.push_back(device.model);
		}
		catch (const std::out_of_range& oor) {
			GKSysLog_UnknownDevice
		}
	}

	return ret;
}

const std::string DevicesManager::getDeviceStatus(const std::string & devID) {
	std::string ret = "unknown";
	if(this->initialized_devices_.count(devID) == 1)
		ret = "started";
	else if(this->plugged_but_stopped_devices_.count(devID) == 1)
		ret = "stopped";
	else if(this->unplugged_devices_.count(devID) == 1)
		ret = "unplugged";
	return ret;
}

void DevicesManager::setDeviceBacklightColor(
	const std::string & devID,
	const uint8_t r,
	const uint8_t g,
	const uint8_t b
) {
	try {
		const auto & device = this->initialized_devices_.at(devID);
#if DEBUGGING_ON
		LOG(DEBUG2) << "found " << device.model << " in started devices";
#endif
		for(const auto& driver : this->drivers_) {
			if( device.driver_ID == driver->getDriverID() ) {
				driver->setDeviceBacklightColor(devID, r, g, b);
				return;
			}
		}
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
}

void DevicesManager::setDeviceMacrosProfiles(
	const std::string & devID,
	const macros_map_t & macros_profiles
) {
	try {
		const auto & device = this->initialized_devices_.at(devID);
#if DEBUGGING_ON
		LOG(DEBUG2) << "found " << device.model << " in started devices";
#endif
		for(const auto& driver : this->drivers_) {
			if( device.driver_ID == driver->getDriverID() ) {
				driver->setDeviceMacrosProfiles(devID, macros_profiles);
				return;
			}
		}
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
}

const macros_map_t & DevicesManager::getDeviceMacrosProfiles(const std::string & devID) {
	try {
		const auto & device = this->initialized_devices_.at(devID);
#if DEBUGGING_ON
		LOG(DEBUG2) << "found " << device.model << " in started devices";
#endif
		for(const auto& driver : this->drivers_) {
			if( device.driver_ID == driver->getDriverID() ) {
				return driver->getDeviceMacrosProfiles(devID);
			}
		}
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
	return MacrosBanks::empty_macros_profiles_;
}

const std::vector<std::string> & DevicesManager::getDeviceMacroKeysNames(const std::string & devID) {
	try {
		const auto & device = this->initialized_devices_.at(devID);
#if DEBUGGING_ON
		LOG(DEBUG2) << "found " << device.model << " in started devices";
#endif
		for(const auto& driver : this->drivers_) {
			if( device.driver_ID == driver->getDriverID() ) {
				return driver->getMacroKeysNames();
			}
		}
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}

	return KeyboardDriver::getEmptyStringVector();
}

void DevicesManager::resetDevicesStates(void) {
#if DEBUGGING_ON
	LOG(DEBUG1) << "resetting initialized devices states";
#endif
	for(const auto& device_pair : this->initialized_devices_) {
		const auto & device = device_pair.second;
		for(const auto& driver : this->drivers_) {
			if( device.driver_ID == driver->getDriverID() ) {
				driver->resetDeviceState(device.device, device.device_bus, device.device_num);
			}
		}
	}
}

void DevicesManager::checkDBusMessages(void) {
	this->pDBus_->checkForNextMessage(NSGKDBus::BusConnection::GKDBUS_SYSTEM);
}

void DevicesManager::startMonitoring(NSGKDBus::GKDBus* pDBus) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "initializing libudev";
#endif
	this->pDBus_ = pDBus;

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

#if DEBUGGING_ON
	LOG(DEBUG2) << "loading drivers";
#endif

	this->drivers_.push_back( new LogitechG510() );

	this->searchSupportedDevices();
	this->initializeDevices();

	/* force num_client to 1 here else signal will never be sent */
	this->sendSignalToClients(this->num_clients_, this->pDBus_, "DaemonIsStarting", true);

	while( DaemonControl::isDaemonRunning() ) {
		int ret = poll(this->fds, 1, 100);

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
#if DEBUGGING_ON
					LOG(DEBUG3) << "Action : " << action;
#endif
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

