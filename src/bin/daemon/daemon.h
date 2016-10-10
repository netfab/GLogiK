
#include <cstdio>
#include <sys/types.h>

#include <fstream>
#include <sstream>
#include <string>

#ifndef __GKLOGIK_DAEMON_H__
#define __GKLOGIK_DAEMON_H__


namespace GLogiK
{

class GLogiKDaemon
{
	public:
		GLogiKDaemon(void);
		~GLogiKDaemon(void);

		int run(const int& argc, char *argv[]);

	protected:
	private:
		pid_t pid;
		FILE* log_fd;

		std::string pid_file_name;
		std::ofstream pid_file;

		std::ostringstream buffer;

		static bool daemon;

		void daemonize(void);
		void parse_command_line(const int& argc, char *argv[]);
		static void handle_signal(int sig);
};

} // namespace GLogiK

#endif
