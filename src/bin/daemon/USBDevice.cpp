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

#include "USBDevice.h"

namespace GLogiK
{

USBDevice::USBDevice() {
}

USBDevice::~USBDevice() {
}


void USBDevice::setRGBBytes(const uint8_t r, const uint8_t g, const uint8_t b)
{
	this->rgb[0] = r;
	this->rgb[1] = g;
	this->rgb[2] = b;
}

void USBDevice::getRGBBytes(uint8_t & r, uint8_t & g, uint8_t & b) const
{
	r = this->rgb[0];
	g = this->rgb[1];
	b = this->rgb[2];
}

} // namespace GLogiK

