
#ifndef __GLOGIKD_DEVICES_MANAGER_H__
#define __GLOGIKD_DEVICES_MANAGER_H__

#include <poll.h>

#include <vector>

#include "keyboard_driver.h"

#include "globals.h"

namespace GLogiKd
{

struct DetectedDevice {
	const char* name;
	const char* vendor_id;
	const char* product_id;
	const char* hidraw_dev_node;
	const char* input_dev_node;
	const char* manufacturer;
	const char* product;
	const char* serial;
	unsigned int driver_ID;
};

#if GLOGIKD_GLOBAL_DEBUG
void deviceProperties(struct udev_device *dev, const char* subsystem);
#endif

class DevicesManager
{
	public:
		DevicesManager(void);
		~DevicesManager(void);

		void startMonitoring(void);

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
