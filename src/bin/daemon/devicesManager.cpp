/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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

#include <boost/process.hpp>
#include <boost/process/search_path.hpp>

#include "lib/utils/utils.hpp"
#include "lib/shared/glogik.hpp"

#include "devicesManager.hpp"

#include "daemonControl.hpp"
#include "logitechG510.hpp"

#include "devicesManager.hpp"

#if GKLIBUSB
#include "libusb.hpp"
#elif GKHIDAPI
#include "hidapi.hpp"
#endif

#include "include/enums.hpp"

namespace bp = boost::process;

namespace GLogiK
{

using namespace NSGKUtils;

#if GKDBUS
DevicesManager::DevicesManager()
	:	_unknown("unknown"),
		_pDBus(nullptr),
		_numClients(0)
#else
DevicesManager::DevicesManager()
	:	_unknown("unknown")
#endif
{
	GK_LOG_FUNC

	GKLog(trace, "initializing devices manager")
}

DevicesManager::~DevicesManager()
{
	GK_LOG_FUNC

	GKLog(trace, "destroying devices manager")

	this->stopInitializedDevices();
	_stoppedDevices.clear();

	GKLog(trace, "stopping drivers")
	for(const auto & driver : _drivers) {
		delete driver;
	}
	_drivers.clear();

	GKLog(trace, "exiting devices manager")
}

const std::string DevicesManager::getLibudevVersion(void)
{
	GK_LOG_FUNC

	std::vector<std::string> data;
	std::string ret;

	try {
		bp::ipstream is;
		std::string line;

		auto p = bp::search_path("udevadm");
		if( p.empty() ) {
			GKSysLogError("udevadm executable not found in PATH");
			return ret;
		}

		GKLog(trace, "running udevadm --version")
		bp::child c(p, "--version", bp::std_out > is);

		while(c.running() && std::getline(is, line) && !line.empty())
			data.push_back(line);

		c.wait();
	}
	catch (const bp::process_error & e) {
		GKSysLogError("exception catched while trying to run udevadm process");
		GKSysLogError( e.what() );
	}

	if( data.empty() ) {
		GKSysLogError("no output from udevadm --version");
		return ret;
	}
	else if( data.size() > 1 ) {
		GKSysLogWarning("multiple lines from udevadm --version");
	}

	ret = data[0];
	return ret;
}

#if GKDBUS
void DevicesManager::setDBus(NSGKDBus::GKDBus* pDBus)
{
	_pDBus = pDBus;
}

void DevicesManager::setNumClients(uint8_t num)
{
	_numClients = num;
}

void DevicesManager::checkDBusMessages(void) noexcept
{
	_pDBus->checkForMessages();
}

#endif

/* exceptions are catched within the function body */
void DevicesManager::initializeDevices(const bool openDevices) noexcept
{
	GK_LOG_FUNC

	GKLog(trace, "initializing detected devices")

#if GKDBUS
	std::vector<std::string> initializedDevices;
#endif

	for(const auto & devicePair : _detectedDevices)
	{
		const auto & devID = devicePair.first;
		const auto & device = devicePair.second;

		if(_startedDevices.count(devID) == 1) {
#if DEBUGGING_ON
			if(GKLogging::GKDebug) {
				LOG(trace)	<< "device already started : "
							<< device.getVendorID() << ":"
							<< device.getProductID() << " - "
							<< device.getDevnode();
			}
#endif
			continue; // jump to next detected device
		}

		// TODO option ?
		if(_stoppedDevices.count(devID) == 1) {
#if DEBUGGING_ON
			if(GKLogging::GKDebug) {
				LOG(trace)	<< "device already initialized, but is in stopped state : "
							<< device.getVendorID() << ":"
							<< device.getProductID() << " - "
							<< device.getDevnode();
				LOG(trace)	<< "automatic initialization is disabled for stopped devices";
			}
#endif
			continue; // jump to next detected device
		}

		for(const auto & driver : _drivers) {
			if( device.getDriverID() == driver->getDriverID() ) {
				std::ostringstream buffer(std::ios_base::app);
				buffer	<< device.getFullName() << " "
						<< device.getVendorID() << ":" << device.getProductID()
						<< " on bus " << toUInt(device.getBus());

				try {
					// initialization
					driver->initializeDevice( device );

					if(openDevices) {
						driver->openDevice( device ); /* throws GLogiKExcept on any failure */
						_startedDevices[devID] = device;
						buffer << " initialized (started)";
					}
					else {
						_stoppedDevices[devID] = device;
						buffer << " initialized (stopped)";
					}
#if GKDBUS
					initializedDevices.push_back(devID);
#endif
				}
				catch ( const GLogiKExcept & e ) {
					buffer << " NOT initialized (failed)";
					GKSysLogError("device initialization failure : ", e.what());
				}

				GKSysLogInfo(buffer.str());
				break;
			}
		} // for : _drivers
	} // for : _detectedDevices

#if GKDBUS
	if( initializedDevices.size() > 0 ) {
		std::string signal("DevicesStopped");
		if(openDevices)
			signal = "DevicesStarted";

		/* inform clients */
		this->sendStatusSignalArrayToClients(_numClients, _pDBus, signal, initializedDevices);
	}
#endif

	_detectedDevices.clear();

	GKLog2(info, "device(s) initialized: ", initializedDevices.size())
}

const bool DevicesManager::startDevice(const std::string & devID)
{
	GK_LOG_FUNC

	GKLog2(trace, devID, " starting device")

	try {
		const auto & device = _stoppedDevices.at(devID);
		for(const auto & driver : _drivers) {
			if( device.getDriverID() == driver->getDriverID() ) {
				driver->initializeDevice( device );
				driver->openDevice( device ); /* throws GLogiKExcept on any failure */

				std::ostringstream buffer(std::ios_base::app);
				buffer	<< device.getFullName() << " "
						<< device.getVendorID() << ":" << device.getProductID()
						<< " on bus " << toUInt(device.getBus()) << " initialized";
				GKSysLogInfo(buffer.str());

				_startedDevices[devID] = device;
				_stoppedDevices.erase(devID);

				return true;
			}
		}
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError("device starting failure : device not found in stopped devices container : ", oor.what());
		return false;
	}
	catch ( const GLogiKExcept & e ) {
		GKSysLogError("device failure : ", e.what());
		return false;
	}

	GKSysLogError("device starting failure : driver not found !?");
	return false;
}

const bool DevicesManager::stopDevice(
	const std::string & devID,
	const bool skipUSBRequests) noexcept
{
	GK_LOG_FUNC

	GKLog2(trace, devID, " stopping device")

	try {
		const auto & device = _startedDevices.at(devID);
		for(const auto & driver : _drivers) {
			if( device.getDriverID() == driver->getDriverID() ) {
				driver->closeDevice( device, skipUSBRequests );

				std::ostringstream buffer(std::ios_base::app);
				buffer	<< device.getFullName() << " "
						<< device.getVendorID() << ":" << device.getProductID()
						<< " on bus " << toUInt(device.getBus()) << " stopped";
				GKSysLogInfo(buffer.str());

				_stoppedDevices[devID] = device;
				_startedDevices.erase(devID);

				return true;
			}
		}
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError("device stopping failure : device not found in started devices container : ", oor.what());
		return false;
	}

	GKSysLogError("device stopping failure : driver not found !?");
	return false;
}

void DevicesManager::stopInitializedDevices(void)
{
	GK_LOG_FUNC

	GKLog(trace, "stopping initialized devices")

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

void DevicesManager::checkInitializedDevicesThreadsStatus(void) noexcept
{
	GK_LOG_FUNC

	//GKLog(trace, "checking initialized devices threads status")

#if GKDBUS
	std::vector<std::string> toSend;
#endif

	std::vector<std::string> toCheck;
	for(const auto& devicePair : _startedDevices) {
		toCheck.push_back(devicePair.first);
	}

	for(const auto & devID : toCheck) {
		for(const auto & driver : _drivers) {
			auto & device = _startedDevices.at(devID);

			if( device.getDriverID() == driver->getDriverID() ) {
				if( ! driver->getDeviceThreadsStatus(devID) ) {
					GKSysLogWarning("USB port software reset detected, not cool :(");
					GKSysLogWarning("We are forced to hard stop a device.");
					GKSysLogWarning("You will get libusb warnings/errors if you do this.");

					this->stopDevice(devID, true);
#if GKDBUS
					toSend.push_back(devID);
#endif
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

void DevicesManager::checkForUnpluggedDevices(void) noexcept
{
	GK_LOG_FUNC

	GKLog(trace, "checking for unplugged devices")

#if GKDBUS
	std::vector<std::string> toSend;
#endif

	/* checking for unplugged unstopped devices */
	std::vector<std::string> toClean;
	for(const auto & devicePair : _startedDevices) {
		if( _detectedDevices.count(devicePair.first) == 0 ) {
			toClean.push_back(devicePair.first);
		}
	}

	for(const auto & devID : toClean) {
		try {
			{
				auto & device = _startedDevices.at(devID);
				std::ostringstream buffer(std::ios_base::app);
				buffer	<< devID << " erasing unplugged initialized driver : "
						<< device.getVendorID() << ":" << device.getProductID()
						<< ":" << device.getDevnode() << ":" << device.getUSec();

				GKSysLogWarning(buffer.str());
				GKSysLogWarning("Did you unplug your device before properly stopping it ?");
				GKSysLogWarning("You will get libusb warnings/errors if you do this.");
			}

			if( this->stopDevice(devID, true) ) {
				const auto & device = _stoppedDevices.at(devID);
				_unpluggedDevices[devID] = device;
				_stoppedDevices.erase(devID);
#if GKDBUS
				toSend.push_back(devID);
#endif
			}
		}
		catch (const std::out_of_range& oor) {
			std::ostringstream buffer(std::ios_base::app);
			buffer << devID << " device not found";
			GKSysLogWarning(buffer.str());
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
		try {
			GKLog2(trace, devID, " removing device from stopped-devices container")
			const auto & device = _stoppedDevices.at(devID);
			_unpluggedDevices[devID] = device;
			_stoppedDevices.erase(devID);
#if GKDBUS
			toSend.push_back(devID);
#endif
		}
		catch (const std::out_of_range& oor) {
			std::ostringstream buffer(std::ios_base::app);
			buffer << "!?! device not found !?! " << devID;
			GKSysLogWarning(buffer.str());
		}
	}
	toClean.clear();

	_detectedDevices.clear();

#if GKDBUS
	GKLog2(trace, "number of unplugged devices : ", toSend.size())

	if(toSend.size() > 0) {
		/* inform clients */
		this->sendStatusSignalArrayToClients(_numClients, _pDBus, "DevicesUnplugged", toSend);
	}
#endif

	GKLog2(trace, "number of devices still initialized : ", _startedDevices.size())
}

#if DEBUGGING_ON
void udevDeviceProperties(struct udev_device * pDevice, const std::string & subSystem)
{
	GK_LOG_FUNC

	struct udev_list_entry *devs_props = nullptr;
	struct udev_list_entry *devs_attr = nullptr;
	struct udev_list_entry *devs_list_entry = nullptr;

	devs_props = udev_device_get_properties_list_entry( pDevice );
	devs_attr = udev_device_get_sysattr_list_entry( pDevice );

	GKLog(trace, "====== Device Properties ======")

	std::string value;
	std::string attr;
	udev_list_entry_foreach( devs_list_entry, devs_props ) {
		attr = toString( udev_list_entry_get_name( devs_list_entry ) );
		if( attr.empty() )
			continue;

		value = toString( udev_device_get_property_value(pDevice, attr.c_str()) );

		GKLog3(trace, attr, " : ", value)
	}

	GKLog(trace, "====== /sys attributes ======")

	udev_list_entry_foreach( devs_list_entry, devs_attr ) {
		attr = toString( udev_list_entry_get_name( devs_list_entry ) );
		if( attr.empty() )
			continue;

		value = toString( udev_device_get_sysattr_value(pDevice, attr.c_str()) );

		GKLog3(trace, attr, " : ", value)
	}

	GKLog(trace, "=============================")
}
#endif

void DevicesManager::searchSupportedDevices(struct udev * pUdev)
{
	GK_LOG_FUNC

	GKLog(trace, "searching for supported devices")

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
			const std::string path( toString( udev_list_entry_get_name(dev_list_entry) ) );
			if( path.empty() )
				throw GLogiKExcept("entry_get_name failure");

			struct udev_device *dev = udev_device_new_from_syspath(pUdev, path.c_str());
			if( dev == nullptr ) {
				GKLog2(trace, "new_from_syspath failure with path : ", path)
				continue;
			}

#if DEBUGGING_ON
			const std::string devss( toString( udev_device_get_subsystem(dev) ) );
			if( devss.empty() ) {
				udev_device_unref(dev);
				throw GLogiKExcept("get_subsystem failure");
			}

			//udevDeviceProperties(dev, devss);
#endif

			const std::string vendorID( toString( udev_device_get_property_value(dev, "ID_VENDOR_ID") ) );
			const std::string productID( toString( udev_device_get_property_value(dev, "ID_MODEL_ID") ) );
			if( vendorID.empty() or productID.empty() ) {
				udev_device_unref(dev);
				continue;
			}

			for(const auto & driver : _drivers) {
				for(const auto& device : driver->getSupportedDevices()) {
					if( device.getVendorID() == vendorID ) {
						if( device.getProductID() == productID ) {

							// path to the event device node in /dev
							const std::string devnode( toString( udev_device_get_devnode(dev) ) );
							if( devnode.empty() ) {
								udev_device_unref(dev);
								continue;
							}

#if DEBUGGING_ON
							udevDeviceProperties(dev, devss);
#endif

							//const std::string vendor( toString( udev_device_get_property_value(dev, "ID_VENDOR") ) );
							//const std::string model( toString( udev_device_get_property_value(dev, "ID_MODEL") ) );
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

							const std::string devpath( toString( udev_device_get_sysattr_value(dev, "devpath") ) );
							if( devpath.empty() ) {
								udev_device_unref(dev);
								continue;
							}

							try {
								const USBDeviceID & d = _detectedDevices.at(devID);
								std::ostringstream buffer(std::ios_base::app);
								buffer << devID << " found already detected device : " << d.getDevnode();
								GKSysLogWarning(buffer.str());
							}
							catch (const std::out_of_range& oor) {

								USBDeviceID found(
									device,
									devnode,
									devpath,
									serial,
									usec,
									driver->getDriverID(),
									bus, num
								);

								_detectedDevices[devID] = found;

#if DEBUGGING_ON
								if(GKLogging::GKDebug) {
									LOG(trace)	<< "found device - Vid:Pid:node:usec | bus:num - "
												<< vendorID << ":" << productID << ":"
												<< devnode << ":" << usec
												<< " | " << toUInt(bus) << ":" << toUInt(num);
								}
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

	// Free the enumerator object
	udev_enumerate_unref(enumerate);

	GKLog2(trace, "number of found device(s) : ", _detectedDevices.size())
}

const std::vector<std::string> DevicesManager::getStartedDevices(void) const
{
	std::vector<std::string> ret;

	// dev code
	//std::vector<std::string> ret = {"aaa1", "bbb2", "ccc3"};

	for(const auto& devicePair : _startedDevices) {
		ret.push_back(devicePair.first);
	}

	return ret;
}

const std::vector<std::string> DevicesManager::getStoppedDevices(void) const
{
	std::vector<std::string> ret;

	for(const auto & devicePair : _stoppedDevices) {
		ret.push_back(devicePair.first);
	}

	return ret;
}

const std::string & DevicesManager::getDeviceVendor(const std::string & devID) const
{
	GK_LOG_FUNC

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
			GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
		}
	}

	return _unknown;
}

const uint64_t DevicesManager::getDeviceCapabilities(const std::string & devID) const
{
	GK_LOG_FUNC

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
			GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
		}
	}

	return 0;
}

const std::string & DevicesManager::getDeviceProduct(const std::string & devID) const
{
	GK_LOG_FUNC

	try {
		const auto & device = _startedDevices.at(devID);
		return device.getProduct();
	}
	catch (const std::out_of_range& oor) {
		try {
			const auto & device = _stoppedDevices.at(devID);
			return device.getProduct();
		}
		catch (const std::out_of_range& oor) {
			GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
		}
	}

	return _unknown;
}

const std::string & DevicesManager::getDeviceName(const std::string & devID) const
{
	GK_LOG_FUNC

	try {
		const auto & device = _startedDevices.at(devID);
		return device.getName();
	}
	catch (const std::out_of_range& oor) {
		try {
			const auto & device = _stoppedDevices.at(devID);
			return device.getName();
		}
		catch (const std::out_of_range& oor) {
			GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
		}
	}

	return _unknown;
}

const LCDPPArray_type &
	DevicesManager::getDeviceLCDPluginsProperties(const std::string & devID) const
{
	GK_LOG_FUNC

	try {
		const auto & device = _startedDevices.at(devID);
		GKLog2(trace, devID, " device is started")

		for(const auto & driver : _drivers) {
			if( device.getDriverID() == driver->getDriverID() ) {
				return driver->getDeviceLCDPluginsProperties(devID);
			}
		}
	}
	catch (const std::out_of_range& oor) {
		try {
			const auto & device = _stoppedDevices.at(devID);
			GKLog2(trace, devID, " device is stopped")

			for(const auto & driver : _drivers) {
				if( device.getDriverID() == driver->getDriverID() ) {
					return driver->getDeviceLCDPluginsProperties(devID);
				}
			}
		}
		catch (const std::out_of_range& oor) {
			GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
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
	const uint8_t r,
	const uint8_t g,
	const uint8_t b,
	const uint64_t LCDPluginsMask1)
{
	GK_LOG_FUNC

	try {
		const auto & device = _startedDevices.at(devID);
		GKLog2(trace, devID, " device is started")

		for(const auto & driver : _drivers) {
			if( device.getDriverID() == driver->getDriverID() ) {
				driver->setDeviceActiveConfiguration(devID, r, g, b, LCDPluginsMask1);
				return;
			}
		}
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
	}
}

const MKeysIDArray_type DevicesManager::getDeviceMKeysIDArray(const std::string & devID) const
{
	GK_LOG_FUNC

	try {
		const auto & device = _startedDevices.at(devID);
		GKLog2(trace, devID, " device is started")

		if( KeyboardDriver::checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
			for(const auto & driver : _drivers) {
				if( device.getDriverID() == driver->getDriverID() ) {
					return driver->getMKeysIDArray();
				}
			}
		}
	}
	catch (const std::out_of_range& oor) {
		try {
			const auto & device = _stoppedDevices.at(devID);
			GKLog2(trace, devID, " device is stopped")

			if( KeyboardDriver::checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
				for(const auto & driver : _drivers) {
					if( device.getDriverID() == driver->getDriverID() ) {
						return driver->getMKeysIDArray();
					}
				}
			}
		}
		catch (const std::out_of_range& oor) {
			GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
		}
	}

	MKeysIDArray_type ret;
	return ret;
}

const GKeysIDArray_type DevicesManager::getDeviceGKeysIDArray(const std::string & devID) const
{
	GK_LOG_FUNC

	try {
		const auto & device = _startedDevices.at(devID);
		GKLog2(trace, devID, " device is started")

		if( KeyboardDriver::checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
			for(const auto & driver : _drivers) {
				if( device.getDriverID() == driver->getDriverID() ) {
					return driver->getGKeysIDArray();
				}
			}
		}
	}
	catch (const std::out_of_range& oor) {
		try {
			const auto & device = _stoppedDevices.at(devID);
			GKLog2(trace, devID, " device is stopped")

			if( KeyboardDriver::checkDeviceCapability(device, Caps::GK_MACROS_KEYS) ) {
				for(const auto & driver : _drivers) {
					if( device.getDriverID() == driver->getDriverID() ) {
						return driver->getGKeysIDArray();
					}
				}
			}
		}
		catch (const std::out_of_range& oor) {
			GKSysLogError(CONST_STRING_UNKNOWN_DEVICE, devID);
		}
	}

	GKeysIDArray_type ret;
	return ret;
}

void DevicesManager::resetDevicesStates(void)
{
	GK_LOG_FUNC

	GKLog(trace, "resetting initialized devices states")

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
 *	USB library failures on devices start/stop are catched internally.
 */
void DevicesManager::startMonitoring(void) {
	GK_LOG_FUNC

	GKLog(trace, "initializing libudev")

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

			GKLog(trace, "loading drivers")

			try {
				KeyboardDriver* driver = nullptr;
#if GKLIBUSB
				driver = new LogitechG510<libusb>();
#elif GKHIDAPI
				driver = new LogitechG510<hidapi>();
#endif

#if GKDBUS
				driver->setDBus(_pDBus);
#endif

				_drivers.push_back( driver );
			}
			catch (const std::bad_alloc& e) { /* handle new() failure */
				throw GLogiKBadAlloc("catch driver wrong allocation");
			}

			this->searchSupportedDevices(pUdev);	/* throws GLogiKExcept on failure */
			this->initializeDevices(true);

#if GKDBUS
			/* send signal, even if no client registered, clients could have started before daemon */
			this->sendSignalToClients(_numClients, _pDBus, "DaemonIsStarting", true);
#endif

			uint16_t c = 0;

			while( DaemonControl::isDaemonRunning() )
			{
				int ret = poll(fds, 1, 100);

				// receive data ?
				if( ret > 0 ) {
					struct udev_device *dev = udev_monitor_receive_device(monitor);
					if( dev == nullptr )
						throw GLogiKExcept("no device from receive_device(), something is wrong");

					try { /* dev unref on catch */
						const std::string action( toString( udev_device_get_action(dev) ) );
						if( action.empty() ) {
							throw GLogiKExcept("device_get_action() failure");
						}

						const std::string devnode( toString( udev_device_get_devnode(dev) ) );

						// filtering empty events
						if( devnode.empty() ) {
							continue;
						}

						GKLog2(trace, "Action : ", action)

						this->searchSupportedDevices(pUdev);	/* throws GLogiKExcept on failure */

						if( action == "add" ) {
							this->initializeDevices(false);
						}
						else if( action == "remove" ) {
							this->checkForUnpluggedDevices();
						}
						else {
							/* clear detected devices container */
							_detectedDevices.clear();
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

