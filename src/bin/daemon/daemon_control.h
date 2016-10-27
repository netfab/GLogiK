
#ifndef __GLOGIKD_DAEMON_CONTROL_H__
#define __GLOGIKD_DAEMON_CONTROL_H__

#include <atomic>

namespace GLogiKd
{

class DaemonControl
{
	public:
		static bool is_daemon_enabled(void);

	protected:
		DaemonControl(void);
		~DaemonControl(void);

		static void disable_daemon(void);

		static std::atomic<bool> daemonized_;
	private:
};

} // namespace GLogiKd

#endif
