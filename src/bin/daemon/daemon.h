
#ifndef __GLOGIKD_DAEMON_H__
#define __GLOGIKD_DAEMON_H__

#include <cstdio>
#include <sys/types.h>

#include <fstream>
#include <sstream>
#include <string>

#include "daemon_control.h"


namespace GLogiKd
{

class GLogiKDaemon : public DaemonControl
{
	public:
		GLogiKDaemon(void);
		~GLogiKDaemon(void);

		int run(const int& argc, char *argv[]);

	protected:
	private:
		pid_t pid_ = 0;
		FILE* log_fd_ = nullptr;
		std::string pid_file_name_ = "";
		std::ofstream pid_file_;
		std::ostringstream buffer_;

		void daemonize(void);
		void parseCommandLine(const int& argc, char *argv[]);
		static void handle_signal(int sig);
};

} // namespace GLogiKd

#endif
