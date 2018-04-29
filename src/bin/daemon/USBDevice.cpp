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

#include <iterator>
#include <algorithm>
#include <new>

#include "lib/utils/utils.h"

#include "USBDevice.h"

namespace GLogiK
{

using namespace NSGKUtils;

USBDevice::USBDevice(
	const std::string & n,
	const std::string & v,
	const std::string & p,
	const uint64_t c,
	uint8_t b,
	uint8_t nu,
	const std::string & id)
		:	BusNumDeviceID(n, v, p, c, b, nu),
			fatal_errors(0),
			macros_man(nullptr),
			pressed_keys(0),
			current_leds_mask(0),
			exit_macro_record_mode(false),
			strID(id),
			last_interrupt_transfer_length(0),
			keys_endpoint(0),
			usb_device(nullptr),
			usb_handle(nullptr),
			listen_status(true)
{
	std::fill_n(this->keys_buffer, KEYS_BUFFER_LENGTH, 0);
	std::fill_n(this->previous_keys_buffer, KEYS_BUFFER_LENGTH, 0);
	this->rgb[0] = 0xFF;
	this->rgb[1] = 0xFF;
	this->rgb[2] = 0xFF;
}

void USBDevice::operator=(const USBDevice& dev)
{
	this->name = dev.name;
	this->vendor_id = dev.vendor_id;
	this->product_id = dev.product_id;
	this->capabilities = dev.capabilities;
	this->bus = dev.bus;
	this->num = dev.num;
	/* end friendship members */

	/* public */
	this->fatal_errors				= dev.fatal_errors;
	this->listen_thread_id			= dev.listen_thread_id;
	this->lcd_thread_id				= dev.lcd_thread_id;
	this->macros_man				= dev.macros_man;
	this->pressed_keys				= dev.pressed_keys;
	this->current_leds_mask			= static_cast<uint8_t>(dev.current_leds_mask);
	this->exit_macro_record_mode	= static_cast<bool>(dev.exit_macro_record_mode);
	std::copy(
		std::begin(dev.keys_buffer),
		std::end(dev.keys_buffer),
		std::begin(this->keys_buffer)
	);
	std::copy(
		std::begin(dev.previous_keys_buffer),
		std::end(dev.previous_keys_buffer),
		std::begin(this->previous_keys_buffer)
	);
	this->chosen_macro_key = dev.chosen_macro_key;
	this->media_key = dev.media_key;
	this->standard_keys_events = dev.standard_keys_events;
	this->last_call = dev.last_call;

	/* private */
	this->strID = dev.strID;
	std::copy(
		std::begin(dev.rgb),
		std::end(dev.rgb),
		std::begin(this->rgb)
	);
	this->last_interrupt_transfer_length = dev.last_interrupt_transfer_length;
	this->keys_endpoint = dev.keys_endpoint;
	this->usb_device = dev.usb_device;
	this->usb_handle = dev.usb_handle;
	this->endpoints = dev.endpoints;
	this->to_release = dev.to_release;
	this->to_attach = dev.to_attach;

	this->listen_status = static_cast<bool>(dev.listen_status);
}

void USBDevice::initializeMacrosManager(
	const char* vk_name,
	const std::vector<std::string> & keys_names)
{
	try {
		this->macros_man = new MacrosManager( vk_name, keys_names );
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		throw GLogiKBadAlloc("macros manager allocation failure");
	}
}

void USBDevice::destroyMacrosManager(void) {
	if( this->macros_man ) {
		delete this->macros_man;
		this->macros_man = nullptr;
	}
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

