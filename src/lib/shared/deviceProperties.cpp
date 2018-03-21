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

#include "deviceProperties.h"

namespace GLogiK
{

DeviceProperties::DeviceProperties() :
	vendor_("unknown"),
	model_("unknown"),
	config_file_name_("none"),
	watch_descriptor_(-1),
	backlight_color_R_(0xFF),
	backlight_color_G_(0xFF),
	backlight_color_B_(0xFF)
{
}

DeviceProperties::~DeviceProperties() {
}

void DeviceProperties::setVendor(const std::string & vendor) {
	this->vendor_ = vendor;
}

void DeviceProperties::setModel(const std::string & model) {
	this->model_ = model;
}

void DeviceProperties::setConfigFileName(const std::string & filename) {
	this->config_file_name_ = filename;
}

void DeviceProperties::setWatchDescriptor(int wd) {
	this->watch_descriptor_ = wd;
}

const std::string & DeviceProperties::getVendor(void) const {
	return this->vendor_;
}

const std::string & DeviceProperties::getModel(void) const {
	return this->model_;
}

const std::string & DeviceProperties::getConfigFileName(void) const {
	return this->config_file_name_;
}

const int DeviceProperties::getWatchDescriptor(void) const {
	return this->watch_descriptor_;
}

} // namespace GLogiK

