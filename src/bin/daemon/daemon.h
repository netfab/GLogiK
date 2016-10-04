
#include <cstdio>
#include <sys/types.h>

#ifndef __GKLOGIK_DAEMON_H__
#define __GKLOGIK_DAEMON_H__


namespace GLogiK
{

class GLogiKDaemon
{
	public:
		GLogiKDaemon(void);
		~GLogiKDaemon(void);

		int run( const int& argc, char *argv[] );

	protected:
	private:
		pid_t pid;
		FILE* log_fd;

		void daemonize(void);
};

} // namespace GLogiK

#endif
