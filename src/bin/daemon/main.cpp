

//#include <iostream>
//#include <cstdlib>

#include "daemon.h"
#include "exception.h"

using namespace GLogiK;

int main(int argc, char *argv[]) {
	//try {
		GLogiKDaemon daemon;
		return daemon.run(argc, argv);
	/*
	}
	catch ( const GLogiKExcept & e ) {
		std::cerr << e.what() << "\n";
	}
	return EXIT_FAILURE;
	*/
}

