
#ifndef GKLOGIK_DAEMON_H
#define GKLOGIK_DAEMON_H


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
};

}

#endif
