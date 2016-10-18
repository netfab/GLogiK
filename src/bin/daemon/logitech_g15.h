
#ifndef __GLOGIKD_LOGITECH_G15_DRIVER_H__
#define __GLOGIKD_LOGITECH_G15_DRIVER_H__

#include "keyboard_driver.h"

namespace GLogiKd
{

#define VENDOR_LOGITECH "046d"

class LogitechG15 : public KeyboardDriver
{
	public:
		LogitechG15();
		~LogitechG15();

		const char* getDriverName() const { return "Logitech G15"; };

	protected:
	private:
};

} // namespace GLogiKd

#endif
