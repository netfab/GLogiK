
#ifndef __GLOGIKD_DEVICES_MANAGER_H__
#define __GLOGIKD_DEVICES_MANAGER_H__


namespace GLogiKd
{

class DevicesManager
{
	public:
		DevicesManager(void);
		~DevicesManager(void);

		void monitor(void);
	protected:

	private:
		struct udev *udev;
		struct udev_monitor *mon;

};

} // namespace GLogiKd

#endif
