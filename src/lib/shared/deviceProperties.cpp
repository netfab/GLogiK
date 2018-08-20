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
	vendor_("unknown"),
	model_("unknown"),
	caps_(0),
	config_file_name_("none"),
	watch_descriptor_(-1),
	backlight_color_R_(0xFF),
	backlight_color_G_(0xFF),
	backlight_color_B_(0xFF)
{
}

DeviceProperties::~DeviceProperties() {
}

/* -- -- -- */

void DeviceProperties::setConfigFileName(const std::string & filename) {
	this->config_file_name_ = filename;
}

void DeviceProperties::setWatchDescriptor(int wd) {
	this->watch_descriptor_ = wd;
}

void DeviceProperties::setProperties(
	const std::string & vendor,
	const std::string & model,
	const uint64_t capabilites
)
{
	this->vendor_	= vendor;
	this->model_	= model;
	this->caps_		= capabilites;
}

void DeviceProperties::setProperties(const DeviceProperties & dev)
{
	dev.getRGBBytes(
		this->backlight_color_R_,
		this->backlight_color_G_,
		this->backlight_color_B_
	);

	_macrosBanks	= dev.getMacrosProfiles();
	this->media_commands_	= dev.getMediaCommands();
}

void DeviceProperties::setRGBBytes(const uint8_t r, const uint8_t g, const uint8_t b) {
	this->backlight_color_R_ = r & 0xFF;
	this->backlight_color_G_ = g & 0xFF;
	this->backlight_color_B_ = b & 0xFF;
}

/* -- -- -- */

const std::string & DeviceProperties::getVendor(void) const {
	return this->vendor_;
}

const std::string & DeviceProperties::getModel(void) const {
	return this->model_;
}

const uint64_t DeviceProperties::getCapabilities(void) const {
	return this->caps_;
}

const std::string & DeviceProperties::getConfigFileName(void) const {
	return this->config_file_name_;
}

const int DeviceProperties::getWatchDescriptor(void) const {
	return this->watch_descriptor_;
}

const std::string DeviceProperties::getMediaCommand(const std::string & mediaEvent) const
{
	std::string ret("");
	try {
		ret = this->media_commands_.at(mediaEvent);
	}
	catch (const std::out_of_range& oor) {
		LOG(WARNING) << "unknown media event : " << mediaEvent;
	}
	return ret;
}

const std::map<const std::string, std::string> & DeviceProperties::getMediaCommands(void) const {
	return this->media_commands_;
};

void DeviceProperties::getRGBBytes(uint8_t & r, uint8_t & g, uint8_t & b) const
{
	r = this->backlight_color_R_;
	g = this->backlight_color_G_;
	b = this->backlight_color_B_;
}

} // namespace GLogiK

