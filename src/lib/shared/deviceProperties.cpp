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

#include "include/enums.hpp"

#include "lib/utils/utils.hpp"

#include "deviceProperties.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

BacklightCapability::BacklightCapability(void)
	:	_red(0xFF),
		_green(0xFF),
		_blue(0xFF)
{
}

BacklightCapability::~BacklightCapability(void)
{
}

void BacklightCapability::setRGBBytes(const uint8_t r, const uint8_t g, const uint8_t b) {
	_red	= r & 0xFF;
	_green	= g & 0xFF;
	_blue	= b & 0xFF;
}

void BacklightCapability::getRGBBytes(uint8_t & r, uint8_t & g, uint8_t & b) const
{
	r = _red;
	g = _green;
	b = _blue;
}

/* -- -- -- */

const LCDPPArray_type LCDScreenCapability::_LCDPluginsPropertiesEmptyArray = {};

LCDScreenCapability::LCDScreenCapability(void)
	:	_LCDPluginsMask1(0)
{
}

LCDScreenCapability::~LCDScreenCapability(void)
{
}

const uint64_t LCDScreenCapability::getLCDPluginsMask1(void) const
{
	return _LCDPluginsMask1;
}

void LCDScreenCapability::setLCDPluginsMask(
	const uint8_t maskID,
	const uint64_t mask)
{
	if(maskID > static_cast<unsigned int>(LCDPluginsMask::GK_LCD_PLUGINS_MASK_1))
		throw GLogiKExcept("wrong maskID value");

	//const LCDPluginsMask id = static_cast<LCDPluginsMask>(maskID);
	_LCDPluginsMask1 = mask;
}

const LCDPPArray_type & LCDScreenCapability::getLCDPluginsProperties(void) const
{
	return _LCDPluginsProperties;
}

void LCDScreenCapability::setLCDPluginsProperties(const LCDPPArray_type & props)
{
	_LCDPluginsProperties = props;
}

/* -- -- -- */

clientDevice::clientDevice()
	:	DeviceID("unknown", "unknown", "unknown", "unknown", "none"),
		_capabilities(0)
{
}

clientDevice::~clientDevice() {
}

/* -- -- -- */

void clientDevice::setProperties(
	const std::string & vendor,
	const std::string & product,
	const std::string & name,
	const uint64_t capabilities)
{
	this->setVendor(vendor);
	this->setProduct(product);
	this->setName(name);
	_capabilities = capabilities;
}

/* -- -- -- */

const uint64_t clientDevice::getCapabilities(void) const {
	return _capabilities;
}

/* -- -- -- */

DeviceProperties::DeviceProperties(void)
	:	_watchedDescriptor(-1)
{
}

DeviceProperties::~DeviceProperties(void) {
}

const int DeviceProperties::getWatchDescriptor(void) const {
	return _watchedDescriptor;
}

void DeviceProperties::setWatchDescriptor(int wd) {
	_watchedDescriptor = wd;
}

void DeviceProperties::setProperties(const DeviceProperties & dev)
{
	dev.getRGBBytes(_red, _green, _blue);

	_GKeysBanks			= dev._GKeysBanks;
	_LCDPluginsMask1	= dev.getLCDPluginsMask1();

	if( _LCDPluginsMask1 == 0 ) {
		/* default enabled plugins */
		_LCDPluginsMask1 |= toEnumType(LCDScreenPlugin::GK_LCD_SPLASHSCREEN);
		_LCDPluginsMask1 |= toEnumType(LCDScreenPlugin::GK_LCD_SYSTEM_MONITOR);
	}
}

/* -- -- -- */
} // namespace GLogiK

