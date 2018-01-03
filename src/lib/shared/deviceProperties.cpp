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

DeviceProperties::DeviceProperties() : vendor_("unknown"), model_("unknown"), conf_file_("none"),
	backlight_color_R_(0xFF), backlight_color_G_(0xFF), backlight_color_B_(0xFF),
	check_(true), state_(DeviceState::STATE_UNKNOWN)
{
}

DeviceProperties::~DeviceProperties() {
}

void DeviceProperties::check(void) {
	this->check_ = true;
}

void DeviceProperties::uncheck(void) {
	this->check_ = false;
}

const bool DeviceProperties::checked(void) const {
	return this->check_;
}

void DeviceProperties::setVendor(const std::string & vendor) {
	this->vendor_ = vendor;
}

void DeviceProperties::setModel(const std::string & model) {
	this->model_ = model;
}

void DeviceProperties::setConfFile(const std::string & conf_file) {
	this->conf_file_ = conf_file;
}

const bool DeviceProperties::started(void) const {
	return (this->state_ == DeviceState::STATE_STARTED);
}

const bool DeviceProperties::stopped(void) const {
	return (this->state_ == DeviceState::STATE_STOPPED);
}

void DeviceProperties::start(void) {
	this->state_ = DeviceState::STATE_STARTED;
}

void DeviceProperties::stop(void) {
	this->state_ = DeviceState::STATE_STOPPED;
}

} // namespace GLogiK

