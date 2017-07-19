
#ifndef __GLOGIKD_KEYBOARD_DRIVER_H__
#define __GLOGIKD_KEYBOARD_DRIVER_H__

#include <string>
#include <vector>
#include <sstream>


#include <libusb-1.0/libusb.h>

namespace GLogiKd
{

struct KeyboardDevice {
	std::string name;
	std::string vendor_id;
	std::string product_id;
};

class KeyboardDriver
{
	public:
		KeyboardDriver();
		virtual ~KeyboardDriver();

		virtual const char* getDriverName() const = 0;
		virtual unsigned int getDriverID() const = 0;

		std::vector<KeyboardDevice> getSupportedDevices(void) const;

		virtual void initializeDevice(const KeyboardDevice &device) = 0;
		virtual void closeDevice(void) = 0;

	protected:
		std::ostringstream buffer_;
		std::vector<KeyboardDevice> supported_devices_;

		void initializeLibusb(void);
		void closeLibusb(void);
		int handleLibusbError(int error_code);

	private:
		libusb_context *context_;
		libusb_device **list_;
};

} // namespace GLogiKd

#endif
