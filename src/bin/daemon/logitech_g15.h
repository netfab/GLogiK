
#ifndef __GLOGIKD_LOGITECH_G15_DRIVER_H__
#define __GLOGIKD_LOGITECH_G15_DRIVER_H__

#include "keyboard_driver.h"
#include "globals.h"

namespace GLogiKd
{

#define VENDOR_LOGITECH "046d"

class LogitechG15 : public KeyboardDriver
{
	public:
		LogitechG15();
		~LogitechG15();

		const char* getDriverName() const { return "Logitech G15"; };
		unsigned int getDriverID() const { return GLOGIKD_DRIVER_ID_G15; };

		void init(const char* vendor_id, const char* product_id, const char* hidraw_dev_node);

	protected:
	private:
};

} // namespace GLogiKd

#endif
