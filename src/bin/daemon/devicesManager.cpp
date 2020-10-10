/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2020  Fabrice Delliaux <netbox253@gmail.com>
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

#include <new>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <poll.h>
#include <libudev.h>

#include "lib/utils/utils.hpp"
#include "lib/shared/glogik.hpp"

#include "devicesManager.hpp"

#include "daemonControl.hpp"
#include "logitechG510.hpp"
#include "libUSB.hpp"

#include "include/enums.hpp"


namespace GLogiK
{

using namespace NSGKUtils;

#if GKDBUS
DevicesManager::DevicesManager()
	:	_pDBus(nullptr),
		_numClients(0),
		_unknown("unknown")
#else
DevicesManager::DevicesManager()
	:	_unknown("unknown")
#endif
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "initializing devices manager";
#endif
}

DevicesManager::~DevicesManager() {
#if DEBUGGING_ON
	LOG(DEBUG2) << "destroying devices manager";
#endif

	this->stopInitializedDevices();
	_stoppedDevices.clear();

#if DEBUGGING_ON
	LOG(DEBUG2) << "stopping drivers";
#endif
	for(const auto & driver : _drivers) {
		delete driver;
	}
	_drivers.clear();

#if DEBUGGING_ON
	LOG(DEBUG2) << "exiting devices manager";
#endif
}

#if GKDBUS
void DevicesManager::setDBus(NSGKDBus::GKDBus* pDBus)
{
	_pDBus = pDBus;
}

void DevicesManager::setNumClients(uint8_t num) {
	_numClients = num;
}

void DevicesManager::checkDBusMessages(void) noexcept {
	_pDBus->checkForMessages();
}

#endif

/* exceptions are catched within the function body */
void DevicesManager::initializeDevices(void) noexcept {
#if DEBUGGING_ON
	LOG(DEBUG2) << "initializing detected devices";
#endif
	std::vector<std::string> startedDevices;

	for(const auto & devicePair : _detectedDevices) {
		const auto & devID = devicePair.first;
		const auto & device = devicePair.second;

		if(_startedDevices.count(devID) == 1) {
#if DEBUGGING_ON
			LOG(DEBUG3) << "device "
						<< device.getVendorID() << ":"
						<< device.getProductID() << " - "
						<< device.getDevNode() << " already initialized";
#endif
			continue; // jump to next detected device
		}

		// TODO option ?
		if(_stoppedDevices.count(devID) == 1) {
#if DEBUGGING_ON
			LOG(DEBUG3) << "device "
						<< device.getVendorID() << ":"
						<< device.getProductID() << " - "
						<< device.getDevNode() << " has been stopped";
			LOG(DEBUG3) << "automatic initialization is disabled for stopped devices";
#endif
			continue; // jump to next detected device
		}

		try {
			for(const auto & driver : _drivers) {
				if( device.getDriverID() == driver->getDriverID() ) {
					// initialization
					driver->openDevice( device );
					_startedDevices[devID] = device;
					startedDevices.push_back(devID);

					std::ostringstream buffer(std::ios_base::app);
					buffer	<< device.getName() << " " << device.getVendorID()
							<< ":" << device.getProductID()
							<< " on bus " << toUInt(device.getBus()) << " initialized";
					GKSysLog(LOG_INFO, INFO, buffer.str());
					break;
				}
			} // for
		}
		catch ( const GLogiKExcept & e ) {
			std::ostringstream buffer(std::ios_base::app);
			buffer << "device initialization failure : " << e.what();
			GKSysLog(LOG_ERR, ERROR, buffer.str());
		}
	} // for

#if GKDBUS
	if( startedDevices.size() > 0 ) {
		/* inform clients */
		this->sendStatusSignalArrayToClients(_numClients, _pDBus, "DevicesStarted", startedDevices);
	}
#endif

	_detectedDevices.clear();
#if DEBUGGING_ON
	LOG(INFO) << "device(s) initialized : " << _startedDevices.size();
#endif
}

const bool DevicesManager::startDevice(const std::string & devID) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "starting device " << devID;
#endif
	try {
		const auto & device = _stoppedDevices.at(devID);
		for(const auto & driver : _drivers) {
			if( device.getDriverID() == driver->getDriverID() ) {
				// initialization
				driver->openDevice( device );
				_startedDevices[devID] = device;

				std::ostringstream buffer(std::ios_base::app);
				buffer	<< device.getName() << " " << device.getVendorID()
						<< ":" << device.getProductID()
						<< " on bus " << toUInt(device.getBus()) << " initialized";
				GKSysLog(LOG_INFO, INFO, buffer.str());

#if DEBUGGING_ON
				LOG(DEBUG3) << "removing " << devID << " from stopped devices container";
#endif
				_stoppedDevices.erase(devID);

				return true;
			}
		}
	}
	catch (const std::out_of_range& oor) {
		std::ostringstream buffer(std::ios_base::app);
		buffer << "device starting failure : device not found in stopped devices container : " << oor.what();
		GKSysLog(LOG_ERR, ERROR, buffer.str());
		return false;
	}

	GKSysLog(LOG_ERR, ERROR, "device starting failure : driver not found !?");
	return false;
}

const bool DevicesManager::stopDevice(const std::string & devID) noexcept {
#if DEBUGGING_ON
	LOG(DEBUG2) << "stopping device " << devID;
#endif

	try {
		const auto & device = _startedDevices.at(devID);
		for(const auto & driver : _drivers) {
			if( device.getDriverID() == driver->getDriverID() ) {
				driver->closeDevice( device );

				std::ostringstream buffer(std::ios_base::app);
				buffer	<< device.getName() << " " << device.getVendorID()
						<< ":" << device.getProductID()
						<< " on bus " << toUInt(device.getBus()) << " stopped";
				GKSysLog(LOG_INFO, INFO, buffer.str());

				_stoppedDevices[devID] = device;
				_startedDevices.erase(devID);

				return true;
			}
		}
	}
	catch (const std::out_of_range& oor) {
		std::ostringstream buffer(std::ios_base::app);
		buffer << "device stopping failure : device not found in started devices container : " << oor.what();
		GKSysLog(LOG_ERR, ERROR, buffer.str());
		return false;
	}

	GKSysLog(LOG_ERR, ERROR, "device stopping failure : driver not found !?");
	return false;
}

void DevicesManager::stopInitializedDevices(void) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "stopping initialized devices";
#endif

	std::vector<std::string> toStop;
	for(const auto & devicePair : _startedDevices) {
		toStop.push_back(devicePair.first);
	}

	for(const auto & devID : toStop) {
		this->stopDevice(devID);
	}

	toStop.clear();
	_startedDevices.clear();
}

void DevicesManager::checkInitializedDevicesThreadsStatus(void) noexcept {
#if 0 && DEBUGGING_ON
	LOG(DEBUG2) << "checking initialized devices threads status";
#endif

	std::vector<std::string> toSend;

	std::vector<std::string> toCheck;
	for(const auto& devicePair : _startedDevices) {
		toCheck.push_back(devicePair.first);
	}

	for(const auto & devID : toCheck) {
		for(const auto & driver : _drivers) {
			if( _startedDevices[devID].getDriverID() == driver->getDriverID() ) {
				if( ! driver->getDeviceThreadsStatus(devID) ) {
					GKSysLog(LOG_WARNING, WARNING, "USB port software reset detected, not cool :(");
					GKSysLog(LOG_WARNING, WARNING, "We are forced to hard stop a device.");
					GKSysLog(LOG_WARNING, WARNING, "You will get libusb warnings/errors if you do this.");
					this->stopDevice(devID);
					toSend.push_back(devID);
				}
			}
		}
	}

#if GKDBUS
	if( toSend.size() > 0 ) {
		/* inform clients */
		this->sendStatusSignalArrayToClients(_numClients, _pDBus, "DevicesStopped", toSend);
	}
#endif
}

void DevicesManager::checkForUnpluggedDevices(void) noexcept {
#if DEBUGGING_ON
	LOG(DEBUG2) << "checking for unplugged devices";
#endif

	std::vector<std::string> toSend;

	/* checking for unplugged unstopped devices */
	std::vector<std::string> toClean;
	for(const auto & devicePair : _startedDevices) {
		if( _detectedDevices.count(devicePair.first) == 0 ) {
			toClean.push_back(devicePair.first);
		}
	}

	for(const auto & devID : toClean) {
		try {

			const auto & device = _startedDevices.at(devID);
			std::ostringstream buffer(std::ios_base::app);
			buffer	<< "erasing unplugged initialized driver : "
					<< device.getVendorID() << ":" << device.getProductID()
					<< ":" << device.getDevNode() << ":" << device.getUSec();

			GKSysLog(LOG_WARNING, WARNING, buffer.str());
			GKSysLog(LOG_WARNING, WARNING, "Did you unplug your device before properly stopping it ?");
			GKSysLog(LOG_WARNING, WARNING, "You will get libusb warnings/errors if you do this.");

			if( this->stopDevice(devID) ) {
				_stoppedDevices.erase(devID);
				_unpluggedDevices.insert(devID);
				toSend.push_back(devID);
			}
		}
		catch (const std::out_of_range& oor) {
			std::ostringstream buffer(std::ios_base::app);
			buffer << "!?! device not found !?! " << devID;
			GKSysLog(LOG_WARNING, WARNING, buffer.str());
		}
	}

	toClean.clear();

	/* checking for unplugged but stopped devices */
	for(const auto & devicePair : _stoppedDevices) {
		if(_detectedDevices.count(devicePair.first) == 0 ) {
			toClean.push_back(devicePair.first);
		}
	}
	for(const auto & devID : toClean) {
#if DEBUGGING_ON
		LOG(DEBUG3) << "removing " << devID << " from stopped devices container";
#endif
		_stoppedDevices.erase(devID);
		_unpluggedDevices.insert(devID);
		toSend.push_back(devID);
	}
	toClean.clear();

	_detectedDevices.clear();

#if DEBUGGING_ON
	LOG(DEBUG3) << "number of devices still initialized : " << _startedDevices.size();
	LOG(DEBUG3) << "number of unplugged devices : " << toSend.size();
#endif

#if GKDBUS
	if(toSend.size() > 0) {
		/* inform clients */
		this->sendStatusSignalArrayToClients(_numClients, _pDBus, "DevicesUnplugged", toSend);
	}
#endif
}

#if DEBUGGING_ON
void udevDeviceProperties(struct udev_device * pDevice, const std::string & subSystem) {
	struct udev_list_entry *devs_props = nullptr;
	struct udev_list_entry *devs_attr = nullptr;
	struct udev_list_entry *devs_list_entry = nullptr;

	devs_props = udev_device_get_properties_list_entry( pDevice );
	devs_attr = udev_device_get_sysattr_list_entry( pDevice );

	LOG(DEBUG4) << "--";
	LOG(DEBUG4)	<< "Device properties";
	LOG(DEBUG4)	<< "--";

	std::string value;
	std::string attr;
	udev_list_entry_foreach( devs_list_entry, devs_props ) {
		attr = toString( udev_list_entry_get_name( devs_list_entry ) );
		if( attr == "" )
			continue;

		value = toString( udev_device_get_property_value(pDevice, attr.c_str()) );
		LOG(DEBUG4) << attr << " : " << value;
	}
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "/sys attributes";
	LOG(DEBUG4) << "--";
	udev_list_entry_foreach( devs_list_entry, devs_attr ) {
		attr = toString( udev_list_entry_get_name( devs_list_entry ) );
		if( attr == "" )
			continue;

		value = toString( udev_device_get_sysattr_value(pDevice, attr.c_str()) );
		LOG(DEBUG4) << attr << " : " << value;
	}
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "--";
	LOG(DEBUG4) << "--";
}
#endif

void DevicesManager::searchSupportedDevices(struct udev * pUdev) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "searching for supported devices";
#endif

	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry = nullptr;

	enumerate = udev_enumerate_new(pUdev);
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
			std::string path = toString( udev_list_entry_get_name(dev_list_entry) );
			if( path == "" )
				throw GLogiKExcept("entry_get_name failure");

			struct udev_device *dev = udev_device_new_from_syspath(pUdev, path.c_str());
			if( dev == nullptr ) {
#if DEBUGGING_ON
				LOG(DEBUG3) << "new_from_syspath failure with path : " << path;
#endif
				continue;
			}

#if DEBUGGING_ON
			std::string devss = toString( udev_device_get_subsystem(dev) );
			if( devss == "" ) {
				udev_device_unref(dev);
				throw GLogiKExcept("get_subsystem failure");
			}

			//udevDeviceProperties(dev, devss);
#endif

			const std::string vendorID( toString( udev_device_get_property_value(dev, "ID_VENDOR_ID") ) );
			const std::string productID( toString( udev_device_get_property_value(dev, "ID_MODEL_ID") ) );
			if( (vendorID == "") or (productID == "") ) {
				udev_device_unref(dev);
				continue;
			}

			for(const auto & driver : _drivers) {
				for(const auto& device : driver->getSupportedDevices()) {
					if( device.getVendorID() == vendorID ) {
						if( device.getProductID() == productID ) {

							// path to the event device node in /dev
							std::string devnode = toString( udev_device_get_devnode(dev) );
							if( devnode == "" ) {
								udev_device_unref(dev);
								continue;
							}

#if DEBUGGING_ON
							udevDeviceProperties(dev, devss);
#endif

							const std::string vendor( toString( udev_device_get_property_value(dev, "ID_VENDOR") ) );
							const std::string model( toString( udev_device_get_property_value(dev, "ID_MODEL") ) );
							const std::string serial( toString( udev_device_get_property_value(dev, "ID_SERIAL") ) );
							const std::string usec( toString( udev_device_get_property_value(dev, "USEC_INITIALIZED") ) );

							uint8_t bus, num = 0;

							try {
								bus = std::stoi( toString( udev_device_get_sysattr_value(dev, "busnum")) );
								num = std::stoi( toString( udev_device_get_sysattr_value(dev, "devnum")) );
							}
							catch (const std::invalid_argument& ia) {
								udev_device_unref(dev);
								throw GLogiKExcept("stoi invalid argument");
							}
							catch (const std::out_of_range& oor) {
								udev_device_unref(dev);
								throw GLogiKExcept("stoi out of range");
							}

							const std::string devID( USBDeviceID::getDeviceID(bus, num) );

							try {
								const DetectedDevice & d = _detectedDevices.at(devID);
								LOG(WARNING) << "found already detected device : " << devID << " " << d.getDevNode();
							}
							catch (const std::out_of_range& oor) {

								DetectedDevice found(
									device.getName(),
									device.getVendorID(),
									device.getProductID(),
									device.getCapabilities(),
									bus, num,
									device.getBConfigurationValue(),
									device.getBInterfaceNumber(),
									device.getBAlternateSetting(),
									device.getBNumEndpoints(),
									device.getKeysInterruptBufferMaxLength(),
									device.getMacrosKeysLength(),
									device.getMediaKeysLength(),
									device.getLCDKeysLength(),
									devnode, vendor, model,
									serial, usec, driver->getDriverID()
								);

								_detectedDevices[devID] = found;

#if DEBUGGING_ON
								LOG(DEBUG3) << "found device - Vid:Pid:DevNode:usec | bus:num : " << vendorID
											<< ":" << productID << ":" << devnode << ":" << usec
											<< " | " << toUInt(bus) << ":" << toUInt(num);
#endif
							}
						}
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
	LOG(DEBUG2) << "found " << _detectedDevices.size() << " device(s)";
#endif

	// Free the enumerator object
	udev_enumerate_unref(enumerate);
}

const std::vector<std::string> DevicesManager::getStartedDevices(void) const {
	std::vector<std::string> ret;

	// dev code
	//std::vector<std::string> ret = {"aaa1", "bbb2", "ccc3"};

	for(const auto& devicePair : _startedDevices) {
		ret.push_back(devicePair.first);
	}

	return ret;
}

const std::vector<std::string> DevicesManager::getStoppedDevices(void) const {
	std::vector<std::string> ret;

	for(const auto & devicePair : _stoppedDevices) {
		ret.push_back(devicePair.first);
	}

	return ret;
}

const std::string & DevicesManager::getDeviceVendor(const std::string & devID) const
{
	try {
		const auto & device = _startedDevices.at(devID);
		return device.getVendor();
	}
	catch (const std::out_of_range& oor) {
		try {
			const auto & device = _stoppedDevices.at(devID);
			return device.getVendor();
		}
		catch (const std::out_of_range& oor) {
			GKSysLog_UnknownDevice
		}
	}

	return _unknown;
}

const uint64_t DevicesManager::getDeviceCapabilities(const std::string & devID) const
{
	try {
		const auto & device = _startedDevices.at(devID);
		return device.getCapabilities();
	}
	catch (const std::out_of_range& oor) {
		try {
			const auto & device = _stoppedDevices.at(devID);
			return device.getCapabilities();
		}
		catch (const std::out_of_range& oor) {
			GKSysLog_UnknownDevice
		}
	}

	return 0;
}

const std::string & DevicesManager::getDeviceModel(const std::string & devID) const
{
	try {
		const auto & device = _startedDevices.at(devID);
		return device.getModel();
	}
	catch (const std::out_of_range& oor) {
		try {
			const auto & device = _stoppedDevices.at(devID);
			return device.getModel();
		}
		catch (const std::out_of_range& oor) {
			GKSysLog_UnknownDevice
		}
	}

	return _unknown;
}

const LCDPluginsPropertiesArray_type & DevicesManager::getDeviceLCDPluginsProperties(
	const std::string & devID
) const
{
	try {
		const auto & device = _startedDevices.at(devID);
#if DEBUGGING_ON
		LOG(DEBUG2) << "found " << device.getModel() << " in started devices";
#endif
		for(const auto & driver : _drivers) {
			if( device.getDriverID() == driver->getDriverID() ) {
				return driver->getDeviceLCDPluginsProperties(devID);
			}
		}
	}
	catch (const std::out_of_range& oor) {
		try {
			const auto & device = _stoppedDevices.at(devID);
#if DEBUGGING_ON
			LOG(DEBUG2) << "found " << device.getModel() << " in stopped devices";
#endif
			for(const auto & driver : _drivers) {
				if( device.getDriverID() == driver->getDriverID() ) {
					return driver->getDeviceLCDPluginsProperties(devID);
				}
			}
		}
		catch (const std::out_of_range& oor) {
			GKSysLog_UnknownDevice
		}
	}

	return LCDScreenPluginsManager::_LCDPluginsPropertiesEmptyArray;
}

const std::string DevicesManager::getDeviceStatus(const std::string & devID) const
{
	std::string ret(_unknown);
	if(_startedDevices.count(devID) == 1)
		ret = "started";
	else if(_stoppedDevices.count(devID) == 1)
		ret = "stopped";
	else if(_unpluggedDevices.count(devID) == 1)
		ret = "unplugged";
	return ret;
}

void DevicesManager::setDeviceActiveConfiguration(
	const std::string & devID,
	const banksMap_type & macrosBanks,
	const uint8_t r,
	const uint8_t g,
	const uint8_t b,
	const uint64_t LCDPluginsMask1
) {
	try {
		const auto & device = _startedDevices.at(devID);
#if DEBUGGING_ON
		LOG(DEBUG2) << "found " << device.getModel() << " in started devices";
#endif
		for(const auto & driver : _drivers) {
			if( device.getDriverID() == driver->getDriverID() ) {
				driver->setDeviceActiveConfiguration(devID, macrosBanks, r, g, b, LCDPluginsMask1);
				return;
			}
		}
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
}

const banksMap_type & DevicesManager::getDeviceMacrosBanks(const std::string & devID) const
{
	try {
		const auto & device = _startedDevices.at(devID);
#if DEBUGGING_ON
		LOG(DEBUG2) << "found " << device.getModel() << " in started devices";
#endif
		if( KeyboardDriver::checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
			for(const auto & driver : _drivers) {
				if( device.getDriverID() == driver->getDriverID() ) {
					return driver->getDeviceMacrosBanks(devID);
				}
			}
		}
	}
	catch (const std::out_of_range& oor) {
		GKSysLog_UnknownDevice
	}
	return MacrosBanks::emptyMacrosBanks;
}

const std::vector<std::string> & DevicesManager::getDeviceMacroKeysNames(const std::string & devID) const
{
	try {
		const auto & device = _startedDevices.at(devID);
#if DEBUGGING_ON
		LOG(DEBUG2) << "found " << device.getModel() << " in started devices";
#endif
		if( KeyboardDriver::checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
			for(const auto & driver : _drivers) {
				if( device.getDriverID() == driver->getDriverID() ) {
					return driver->getMacroKeysNames();
				}
			}
		}
	}
	catch (const std::out_of_range& oor) {
		try {
			const auto & device = _stoppedDevices.at(devID);
#if DEBUGGING_ON
			LOG(DEBUG2) << "found " << device.getModel() << " in stopped devices";
#endif
			if( KeyboardDriver::checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
				for(const auto & driver : _drivers) {
					if( device.getDriverID() == driver->getDriverID() ) {
						return driver->getMacroKeysNames();
					}
				}
			}
		}
		catch (const std::out_of_range& oor) {
			GKSysLog_UnknownDevice
		}
	}

	return KeyboardDriver::getEmptyStringVector();
}

void DevicesManager::resetDevicesStates(void) {
#if DEBUGGING_ON
	LOG(DEBUG1) << "resetting initialized devices states";
#endif
	for(const auto& devicePair : _startedDevices) {
		const auto & device = devicePair.second;
		for(const auto & driver : _drivers) {
			if( device.getDriverID() == driver->getDriverID() ) {
				driver->resetDeviceState( device );
			}
		}
	}
}

/*
 *	Throws GLogiKExcept in many ways on udev related functions failures.
 *	LibUSB failures on devices start/stop are catched internally.
 */
void DevicesManager::startMonitoring(void) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "initializing libudev";
#endif

	struct udev * pUdev = udev_new();
	if(pUdev == nullptr )
		throw GLogiKExcept("udev context init failure");

	try { /* pUdev unref on catch */
		struct udev_monitor * monitor = udev_monitor_new_from_netlink(pUdev, "udev");
		if( monitor == nullptr )
			throw GLogiKExcept("allocating udev monitor failure");

		try { /* monitor unref on catch */
			if( udev_monitor_filter_add_match_subsystem_devtype(monitor, "usb", nullptr) < 0 )
				throw GLogiKExcept("usb monitor filtering init failure");

			if( udev_monitor_enable_receiving(monitor) < 0 )
				throw GLogiKExcept("monitor enabling failure");

			pollfd fds[1];
			{
				int fd = udev_monitor_get_fd(monitor);
				if( fd < 0 )
					throw GLogiKExcept("can't get the monitor file descriptor");

				fds[0].fd = fd;
				fds[0].events = POLLIN;
			}

#if DEBUGGING_ON
			LOG(DEBUG2) << "loading drivers";
#endif
			try {
				_drivers.push_back( new LogitechG510<LibUSB>() );
			}
			catch (const std::bad_alloc& e) { /* handle new() failure */
				throw GLogiKBadAlloc("catch driver wrong allocation");
			}

			this->searchSupportedDevices(pUdev);	/* throws GLogiKExcept on failure */
			this->initializeDevices();

#if GKDBUS
			/* send signal, even if no client registered, clients could have started before daemon */
			this->sendSignalToClients(_numClients, _pDBus, "DaemonIsStarting", true);
#endif

			unsigned short c = 0;

			while( DaemonControl::isDaemonRunning() )
			{
				int ret = poll(fds, 1, 100);

				// receive data ?
				if( ret > 0 ) {
					struct udev_device *dev = udev_monitor_receive_device(monitor);
					if( dev == nullptr )
						throw GLogiKExcept("no device from receive_device(), something is wrong");

					try { /* dev unref on catch */
						std::string action = toString( udev_device_get_action(dev) );
						if( action == "" ) {
							throw GLogiKExcept("device_get_action() failure");
						}

						std::string devnode = toString( udev_device_get_devnode(dev) );
						// filtering empty events
						if( devnode != "" ) {
#if DEBUGGING_ON
							LOG(DEBUG3) << "Action : " << action;
#endif
							this->searchSupportedDevices(pUdev);	/* throws GLogiKExcept on failure */

							if( action == "add" ) {
								this->initializeDevices();
							}
							else if( action == "remove" ) {
								this->checkForUnpluggedDevices();
							}
							else {
								/* clear detected devices container */
								_detectedDevices.clear();
							}
						}
					}
					catch ( const GLogiKExcept & e ) {
						udev_device_unref(dev);
						throw;
					}

					udev_device_unref(dev);
				}

#if GKDBUS
				this->checkDBusMessages();
#endif
				if(c++ >= 10) { /* bonus point */
					this->checkInitializedDevicesThreadsStatus();
					c = 0;
				}
			}

		} // try
		catch ( const GLogiKExcept & e ) {
			udev_monitor_unref(monitor);
			throw;
		}

		udev_monitor_unref(monitor);

	} // try
	catch ( const GLogiKExcept & e ) {
		udev_unref(pUdev);
		throw;
	}

	udev_unref(pUdev);
}

} // namespace GLogiK

