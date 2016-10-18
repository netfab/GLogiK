
#ifndef __GLOGIKD_KEYBOARD_DRIVER_H__
#define __GLOGIKD_KEYBOARD_DRIVER_H__

#include <vector>
#include <string>

namespace GLogiKd
{

struct device {
	const char* name;
	const char* vendor_id;
	const char* product_id;
	device(const char* n, const char* v, const char* p) {
		name = n;
		vendor_id = v;
		product_id = p;
	}
};

class KeyboardDriver
{
	public:
		KeyboardDriver();
		virtual ~KeyboardDriver();

		virtual const char* getDriverName() const = 0;

		std::vector<device>::iterator getSupportedDevicesFirst(void);
		std::vector<device>::iterator getSupportedDevicesEnd(void);

	protected:
		std::vector<device>::iterator device_iterator_;
		std::vector<device> supported_devices_;

	private:


};

} // namespace GLogiKd

#endif
