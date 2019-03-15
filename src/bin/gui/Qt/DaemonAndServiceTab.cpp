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

#include <string>

#include <QString>
#include <QVBoxLayout>
//#include <QHBoxLayout>

#include "lib/shared/glogik.hpp"
#include "lib/utils/utils.hpp"

#include "DaemonAndServiceTab.hpp"


namespace GLogiK
{

using namespace NSGKUtils;

DaemonAndServiceTab::DaemonAndServiceTab(
	NSGKDBus::GKDBus* pDBus,
	const QString & name
)	:	Tab(pDBus)
{
	this->setObjectName(name);

	QVBoxLayout *vBox = nullptr;
	//QHBoxLayout* hBox = nullptr;

	try {
		vBox = new QVBoxLayout(this);
#if DEBUGGING_ON
		LOG(DEBUG1) << "allocated QVBoxLayout";
#endif
		this->setLayout(vBox);

		/* -- -- -- */
		{
			QFrame* line = new QFrame();
#if DEBUGGING_ON
			LOG(DEBUG2) << "allocated QFrame";
#endif
			line->setFrameShape(QFrame::HLine);
			line->setFrameShadow(QFrame::Sunken);
			vBox->addWidget(line);
		}

		/* -- -- -- */

		_daemonVersion = new QLabel("Daemon version");
		vBox->addWidget(_daemonVersion);

		/* -- -- -- */
		_serviceVersion = new QLabel("Service version");
		vBox->addWidget(_serviceVersion);

		/* -- -- -- */

		vBox->addSpacing(300);
	}
	catch (const std::bad_alloc& e) {
		LOG(ERROR) << e.what();
		throw;
	}
}

DaemonAndServiceTab::~DaemonAndServiceTab()
{
}

void DaemonAndServiceTab::updateTab(void)
{
	const std::string remoteMethod("GetVersions");
	try {
		_pDBus->initializeRemoteMethodCall(
			NSGKDBus::BusConnection::GKDBUS_SESSION,
			GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		_pDBus->appendStringToRemoteMethodCall("reserved");

		_pDBus->sendRemoteMethodCall();

		try {
			_pDBus->waitForRemoteMethodCallReply();

			QString daemonVersion("Daemon version : ");
			daemonVersion += _pDBus->getNextStringArgument().c_str();

			QString serviceVersion("Service version : ");
			serviceVersion += _pDBus->getNextStringArgument().c_str();

			_daemonVersion->setText(daemonVersion);
			_serviceVersion->setText(serviceVersion);
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
			throw GLogiKExcept("failure to get request reply");
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_pDBus->abandonRemoteMethodCall();
		LogRemoteCallFailure
		throw GLogiKExcept("failure to build request");
	}
}

} // namespace GLogiK

