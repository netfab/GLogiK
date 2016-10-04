
#include <cstdlib>

#include <iostream>	// std::cout, std::endl
#include <iomanip>	// std:setw
#include <string>

#include "exception.h"
#include "core.h"

namespace GLogiK
{

GLogiKServerCore::GLogiKServerCore() : context(NULL), list(NULL), ret_value(0), num_devices(0), rvalue(EXIT_FAILURE)
{
	this->ret_value = libusb_init(&this->context);
	this->check_libusb_value(this->ret_value, "libusb initialization failed");
	this->num_devices = libusb_get_device_list(this->context, &this->list);
	//if( this->num_devices < 0 )
	//	throw GLogiKExcept("LIBUSB get_device_list error");  // FIXME
	this->check_libusb_value(this->num_devices, "libusb get_device_list error");
}

GLogiKServerCore::~GLogiKServerCore()
{
	libusb_free_device_list(this->list, 1);
}

int GLogiKServerCore::run( const int& argc, char *argv[] ) {

	for (size_t idx = 0; idx < (size_t)this->num_devices; ++idx) {
		libusb_device *device = this->list[idx];
		libusb_device_descriptor desc = {0};

		this->ret_value = libusb_get_device_descriptor(device, &desc);
		this->check_libusb_value(this->ret_value, "libusb get_device_decriptor failed");

		//printf("Vendor:Device = %04x:%04x\n", desc.idVendor, desc.idProduct);
		//std::cout << std::setw( -20 ) << "Name" << setw( 20 ) << "Surname"  << setw( 5 ) << "Id" << endl;
		std::cout << "Vendor:Device = " << std::setfill ('0') << std::setw (4)
						<< std::hex << desc.idVendor << ":"
						<< std::setfill ('0') << std::setw (4)
						<< std::hex << desc.idProduct << "\n";
	}
	this->rvalue = EXIT_SUCCESS;

	return this->rvalue;
}

void GLogiKServerCore::check_libusb_value( const ssize_t &val, const char *st ) {
	if( val >= LIBUSB_SUCCESS )
		return;
	std::string msg = st;
	msg += " : ";
	msg += libusb_strerror( (libusb_error)val );

	throw GLogiKExcept(msg);
}

}

