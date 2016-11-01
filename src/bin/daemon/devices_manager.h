
#ifndef __GLOGIKD_DEVICES_MANAGER_H__
#define __GLOGIKD_DEVICES_MANAGER_H__

#include <poll.h>

#include <string>
#include <vector>

#include "keyboard_driver.h"

#include "globals.h"

namespace GLogiKd
{

struct DetectedDevice {
	std::string name;
	std::string vendor_id;
	std::string product_id;
	std::string hidraw_dev_node;
	std::string input_dev_node;
	std::string manufacturer;
	std::string product;
	std::string serial;
	unsigned int driver_ID;
};

#if GLOGIKD_GLOBAL_DEBUG
void deviceProperties(struct udev_device *dev, const std::string &subsystem);
#endif

class DevicesManager
{
	public:
		DevicesManager(void);
		~DevicesManager(void);

		void startMonitoring(void);
		static std::string getString(const char* s) { return s == nullptr ? "" : s; };

	protected:

	private:
		struct udev *udev = nullptr;
		struct udev_monitor *monitor = nullptr;
		struct pollfd fds[1];
		int fd_ = -1;

		std::vector<KeyboardDriver*> drivers_;
		std::vector<DetectedDevice> detected_devices_;
		std::vector<DetectedDevice> initialized_devices_;

		void searchSupportedDevices(void);
		void initializeDrivers(void);
};

} // namespace GLogiKd

#endif
