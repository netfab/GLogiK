/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2019  Fabrice Delliaux <netbox253@gmail.com>
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

DeviceProperties::DeviceProperties()
	:	_capabilities(0),
		_watchedDescriptor(-1),
		_backlightRed(0xFF),
		_backlightGreen(0xFF),
		_backlightBlue(0xFF),
		_LCDPluginsMask1(0)
{
}

DeviceProperties::~DeviceProperties() {
}

/* -- -- -- */

void DeviceProperties::setWatchDescriptor(int wd) {
	_watchedDescriptor = wd;
}

void DeviceProperties::setProperties(
	const std::string & vendor,
	const std::string & model,
	const uint64_t capabilities)
{
	this->setVendor(vendor);
	this->setModel(model);
	_capabilities = capabilities;
}

void DeviceProperties::setProperties(const DeviceProperties & dev)
{
	dev.getRGBBytes(_backlightRed, _backlightGreen, _backlightBlue);

	_macrosBanks		= dev.getMacrosBanks();
	_LCDPluginsMask1	= dev.getLCDPluginsMask1();

	if( _LCDPluginsMask1 == 0 ) {
		/* default enabled plugins */
		_LCDPluginsMask1 |= toEnumType(LCDScreenPlugin::GK_LCD_SPLASHSCREEN);
		_LCDPluginsMask1 |= toEnumType(LCDScreenPlugin::GK_LCD_SYSTEM_MONITOR);
	}
}

void DeviceProperties::setRGBBytes(const uint8_t r, const uint8_t g, const uint8_t b) {
	_backlightRed	= r & 0xFF;
	_backlightGreen	= g & 0xFF;
	_backlightBlue	= b & 0xFF;
}

void DeviceProperties::setLCDPluginsMask(
	const uint8_t maskID,
	const uint64_t mask)
{
	if(maskID > static_cast<unsigned int>(LCDPluginsMask::GK_LCD_PLUGINS_MASK_1))
		throw GLogiKExcept("wrong maskID value");

	//const LCDPluginsMask id = static_cast<LCDPluginsMask>(maskID);
	_LCDPluginsMask1 = mask;
}

/* -- -- -- */

const uint64_t DeviceProperties::getCapabilities(void) const {
	return _capabilities;
}

const int DeviceProperties::getWatchDescriptor(void) const {
	return _watchedDescriptor;
}

const uint64_t DeviceProperties::getLCDPluginsMask1(void) const
{
	return _LCDPluginsMask1;
}

void DeviceProperties::getRGBBytes(uint8_t & r, uint8_t & g, uint8_t & b) const
{
	r = _backlightRed;
	g = _backlightGreen;
	b = _backlightBlue;
}

} // namespace GLogiK

