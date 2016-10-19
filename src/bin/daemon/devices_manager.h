
#ifndef __GLOGIKD_DEVICES_MANAGER_H__
#define __GLOGIKD_DEVICES_MANAGER_H__

#include <poll.h>

#include <vector>

#include "keyboard_driver.h"

#define GLOGIKD_DEVICES_MANAGER_DEBUG 0

namespace GLogiKd
{

class DevicesManager
{
	public:
		DevicesManager(void);
		~DevicesManager(void);

		void startMonitoring(void);
		void searchSupportedDevices();

	protected:

	private:
		struct udev *udev;
		struct udev_monitor *monitor;
		struct pollfd fds[1];
		int fd_;

		std::vector<KeyboardDriver*> drivers;

};

} // namespace GLogiKd

#endif
