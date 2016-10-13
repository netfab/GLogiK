
#ifndef __GLOGIKD_DEVICES_MANAGER_H__
#define __GLOGIKD_DEVICES_MANAGER_H__

#include <poll.h>


namespace GLogiKd
{

class DevicesManager
{
	public:
		DevicesManager(void);
		~DevicesManager(void);

		void startMonitoring(void);
	protected:

	private:
		struct udev *udev;
		struct udev_monitor *monitor;
		struct pollfd fds[1];
		int fd_;

};

} // namespace GLogiKd

#endif
