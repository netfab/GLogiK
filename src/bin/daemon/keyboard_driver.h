
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

		virtual void initializeDevice(const KeyboardDevice &device, const unsigned int bus, const unsigned int num) = 0;
		virtual void closeDevice(const KeyboardDevice &device, const unsigned int bus, const unsigned int num) = 0;

	protected:
		std::ostringstream buffer_;
		std::vector<KeyboardDevice> supported_devices_;

		void initializeLibusb(void);

	private:
		static bool libusb_status_;			/* is libusb initialized ? */
		static unsigned int drivers_cnt_;	/* initialized drivers counter */
		libusb_context *context_;
		libusb_device **list_;

		void closeLibusb(void);
		int handleLibusbError(int error_code, const char* except_msg);
};

} // namespace GLogiKd

#endif
