
#ifndef __GLOGIKD_LOGITECH_G510_DRIVER_H__
#define __GLOGIKD_LOGITECH_G510_DRIVER_H__

#include "keyboard_driver.h"
#include "globals.h"

namespace GLogiKd
{

#define VENDOR_LOGITECH "046d"

class LogitechG510 : public KeyboardDriver
{
	public:
		LogitechG510();
		~LogitechG510();

		const char* getDriverName() const { return "Logitech G510/G510s"; };
		unsigned int getDriverID() const { return GLOGIKD_DRIVER_ID_G510; };

		void initializeDevice(const KeyboardDevice & device);
		void closeDevice(void);

	protected:
	private:
};

} // namespace GLogiKd

#endif