

#include <iostream>
#include <cstdlib>

#include "core.h"
#include "exception.h"

using namespace GLogiK;

int main(int argc, char *argv[]) {
	try {
		GLogiKServerCore server;
		return server.run(argc, argv);
	}
	catch ( const GLogiKExcept & e ) {
		std::cerr << e.what() << "\n";
	}
	return EXIT_FAILURE;
}

