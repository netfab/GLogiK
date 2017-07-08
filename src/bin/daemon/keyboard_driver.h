
#ifndef __GLOGIKD_KEYBOARD_DRIVER_H__
#define __GLOGIKD_KEYBOARD_DRIVER_H__

#include <vector>

namespace GLogiKd
{

struct KeyboardDevice {
	const char* name;
	const char* vendor_id;
	const char* product_id;
};

class KeyboardDriver
{
	public:
		KeyboardDriver();
		virtual ~KeyboardDriver();

		virtual const char* getDriverName() const = 0;
		virtual unsigned int getDriverID() const = 0;

		std::vector<KeyboardDevice> getSupportedDevices(void) const;

		virtual void init(const char* vendor_id, const char* product_id) = 0;
		virtual void closeDevice(void) = 0;

	protected:
		std::vector<KeyboardDevice> supported_devices_;

	private:
};

} // namespace GLogiKd

#endif
