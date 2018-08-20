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

#include <stdexcept>

#include "lib/utils/utils.h"

#include "deviceProperties.h"

namespace GLogiK
{

using namespace NSGKUtils;

DeviceProperties::DeviceProperties() :
	_vendor("unknown"),
	_model("unknown"),
	_capabilities(0),
	_configFileName("none"),
	_watchedDescriptor(-1),
	_backlightRed(0xFF),
	_backlightGreen(0xFF),
	_backlightBlue(0xFF)
{
}

DeviceProperties::~DeviceProperties() {
}

/* -- -- -- */

void DeviceProperties::setConfigFileName(const std::string & fileName) {
	_configFileName = fileName;
}

void DeviceProperties::setWatchDescriptor(int wd) {
	_watchedDescriptor = wd;
}

void DeviceProperties::setProperties(
	const std::string & vendor,
	const std::string & model,
	const uint64_t capabilites
)
{
	_vendor	= vendor;
	_model	= model;
	_capabilities = capabilites;
}

void DeviceProperties::setProperties(const DeviceProperties & dev)
{
	dev.getRGBBytes(_backlightRed, _backlightGreen, _backlightBlue);

	_macrosBanks	= dev.getMacrosBanks();
	_mediaCommands	= dev.getMediaCommands();
}

void DeviceProperties::setRGBBytes(const uint8_t r, const uint8_t g, const uint8_t b) {
	_backlightRed	= r & 0xFF;
	_backlightGreen	= g & 0xFF;
	_backlightBlue	= b & 0xFF;
}

/* -- -- -- */

const std::string & DeviceProperties::getVendor(void) const {
	return _vendor;
}

const std::string & DeviceProperties::getModel(void) const {
	return _model;
}

const uint64_t DeviceProperties::getCapabilities(void) const {
	return _capabilities;
}

const std::string & DeviceProperties::getConfigFileName(void) const {
	return _configFileName;
}

const int DeviceProperties::getWatchDescriptor(void) const {
	return _watchedDescriptor;
}

const std::string DeviceProperties::getMediaCommand(const std::string & mediaEvent) const
{
	std::string ret("");
	try {
		ret = _mediaCommands.at(mediaEvent);
	}
	catch (const std::out_of_range& oor) {
		LOG(WARNING) << "unknown media event : " << mediaEvent;
	}
	return ret;
}

const std::map<const std::string, std::string> & DeviceProperties::getMediaCommands(void) const {
	return _mediaCommands;
};

void DeviceProperties::getRGBBytes(uint8_t & r, uint8_t & g, uint8_t & b) const
{
	r = _backlightRed;
	g = _backlightGreen;
	b = _backlightBlue;
}

} // namespace GLogiK

