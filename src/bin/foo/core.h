
#ifndef GKLOGIKSERVERCORE_H
#define GKLOGIKSERVERCORE_H

#include <libusb-1.0/libusb.h>

namespace GLogiK
{

class GLogiKServerCore
{
	public:
		GLogiKServerCore(void);
		~GLogiKServerCore(void);

		int run( const int& argc, char *argv[] );

	protected:
	private:
		libusb_context *context;
		libusb_device **list;
		ssize_t ret_value;
		ssize_t num_devices;
		int rvalue;

		void check_libusb_value( const ssize_t &val, const char* st );
};

}

#endif
