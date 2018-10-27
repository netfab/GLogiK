
#include <QApplication>

#include "mainWindow.hpp"

#include "lib/utils/utils.hpp"

using namespace GLogiK;
using namespace NSGKUtils;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	MainWindow window;

	try {

		window.init();
		window.build();

		//window.resize(250, 150);
		window.setFixedSize(800, 400);
		window.setWindowTitle("Simple example");
		window.show();
	}
	catch ( const NSGKUtils::GLogiKExcept & e ) {
		LOG(ERROR) << e.what();
	}

	return app.exec();
}

