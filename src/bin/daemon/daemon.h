
#include <cstdio>
#include <sys/types.h>

#include <fstream>
#include <sstream>
#include <string>

#include <boost/atomic.hpp>

#ifndef __GLOGIKD_DAEMON_H__
#define __GLOGIKD_DAEMON_H__


namespace GLogiKd
{

class GLogiKDaemon
{
	public:
		GLogiKDaemon(void);
		~GLogiKDaemon(void);

		int run(const int& argc, char *argv[]);

		static bool isItEnabled(void);
		static void disableDaemon(void);

	protected:
	private:
		pid_t pid;
		FILE* log_fd;

		std::string pid_file_name;
		std::ofstream pid_file;

		std::ostringstream buffer;

		static boost::atomic<bool> daemon;

		void daemonize(void);
		void parse_command_line(const int& argc, char *argv[]);
		static void handle_signal(int sig);
};

} // namespace GLogiKd

#endif
