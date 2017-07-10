
#ifndef __GLOGIKD_LOGITECH_G15_DRIVER_H__
#define __GLOGIKD_LOGITECH_G15_DRIVER_H__

#include <sstream>

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

		void initializeDevice(const char* vendor_id, const char* product_id);
		void closeDevice(void);

	protected:
	private:
		bool initialized;
		std::ostringstream buffer_;
};

} // namespace GLogiKd

#endif
