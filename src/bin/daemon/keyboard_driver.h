
#ifndef __GLOGIKD_KEYBOARD_DRIVER_H__
#define __GLOGIKD_KEYBOARD_DRIVER_H__

#include <vector>
#include <string>
#include <atomic>
#include <thread>

#include <poll.h>

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

		virtual void init(const char* vendor_id, const char* product_id, const char* hidraw_dev_node) = 0;
		bool isConnected(void) const;
		void disconnectDevice(void);
		void connectDevice(const char* hidraw_dev_node);

	protected:
		std::vector<KeyboardDevice> supported_devices_;

	private:
		struct pollfd fds[1] = { {-1, POLLIN} };
		std::atomic<bool> connected_;
		std::thread monitorThread_;

		void openDevNode(const char* hidraw_dev_node);
		void closeDevNode(void);
		void monitorDevice(void);
};

} // namespace GLogiKd

#endif
