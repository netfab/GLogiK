/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2019  Fabrice Delliaux <netbox253@gmail.com>
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <cstdlib>

#include <QString>
#include <QApplication>
#include <QIcon>

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

		window.setFixedSize(800, 400);

		{
			QString title("GKcQt5 "); title += VERSION;
			window.setWindowTitle(title);

			QString icon(DATA_DIR);
			icon += "/icons/hicolor/48x48/apps/GLogiK.png";
			window.setWindowIcon(QIcon(icon));
		}

		window.show();
	}
	catch ( const NSGKUtils::GLogiKExcept & e ) {
		LOG(ERROR) << e.what();
		return EXIT_FAILURE;
	}

	return app.exec();
}

