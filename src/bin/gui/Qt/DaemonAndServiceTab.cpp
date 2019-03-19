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

#include <QSizePolicy>
#include <QString>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QHBoxLayout>

#include "lib/shared/glogik.hpp"
#include "lib/utils/utils.hpp"

#include "DaemonAndServiceTab.hpp"


namespace GLogiK
{

using namespace NSGKUtils;

DaemonAndServiceTab::DaemonAndServiceTab(
	NSGKDBus::GKDBus* pDBus,
	const QString & name
)	:	Tab(pDBus),
		_serviceStarted(false),
		_serviceRegistered(false)
{
	this->setObjectName(name);

	QVBoxLayout* vBox = nullptr;
	//QHBoxLayout* hBox = nullptr;

	try {
		vBox = new QVBoxLayout(this);
#if DEBUGGING_ON
		LOG(DEBUG1) << "allocated QVBoxLayout";
#endif
		this->setLayout(vBox);

		/* -- -- -- */
		vBox->addWidget( this->getHLine() );


		/* -- -- -- */
		{
			QGroupBox* daemonBox = new QGroupBox();
			vBox->addWidget(daemonBox);

			daemonBox->setTitle("Daemon");

			QHBoxLayout* layout = new QHBoxLayout();
			daemonBox->setLayout(layout);

			_daemonVersionLabel = new QLabel("Version");
			layout->addWidget(_daemonVersionLabel);
		}

		/* -- -- -- */

		{
			QGroupBox* serviceBox = new QGroupBox();
			vBox->addWidget(serviceBox);

			serviceBox->setTitle("Service");

			QHBoxLayout* layout = new QHBoxLayout();
			serviceBox->setLayout(layout);

			_serviceVersionLabel = new QLabel("Version");
			layout->addWidget(_serviceVersionLabel);

			_serviceStatusLabel = new QLabel("Status");
			layout->addWidget(_serviceStatusLabel);

			_pStart = new QPushButton("Start service");
#if DEBUGGING_ON
			LOG(DEBUG1) << "allocated Start button";
#endif
			_pStart->setObjectName("StartServiceButton");
			layout->addWidget(_pStart);

			/* keeping space used when hiding button */
			QSizePolicy retain = _pStart->sizePolicy();
			retain.setRetainSizeWhenHidden(true);
			_pStart->setSizePolicy(retain);

			_pStart->setEnabled(false);

			connect(_pStart, &QPushButton::clicked, this, &DaemonAndServiceTab::startSignal);
		}

		/* -- -- -- */
	}
	catch (const std::bad_alloc& e) {
		LOG(ERROR) << e.what();
		throw;
	}
}

DaemonAndServiceTab::~DaemonAndServiceTab()
{
}

const bool DaemonAndServiceTab::isServiceStarted(void) const
{
	return _serviceStarted;
}

const bool DaemonAndServiceTab::isServiceRegistered(void) const
{
	return _serviceRegistered;
}

void DaemonAndServiceTab::updateTab(void)
{
	{
		const QString vers("Version : unknown");
		_daemonVersionLabel->setText(vers);
		_serviceVersionLabel->setText(vers);

		/* assuming service stopped */
		_serviceStatusLabel->setText("Status : stopped");
		_serviceRegistered = false;
		_serviceStarted = false;
		_pStart->setVisible(false);
		_pStart->setEnabled(false);
	}

	const std::string remoteMethod("GetInformations");
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

			QString labelText("Version : ");
			labelText += _pDBus->getNextStringArgument().c_str();
			_daemonVersionLabel->setText(labelText);

			labelText = "Version : ";
			labelText += _pDBus->getNextStringArgument().c_str();
			_serviceVersionLabel->setText(labelText);

			labelText = "Status : started and ";
			QString status(_pDBus->getNextStringArgument().c_str());
			labelText += status;
			_serviceStatusLabel->setText(labelText);

			_serviceRegistered = (status == "registered");
			_serviceStarted = true;

			_pStart->setVisible(false);
			_pStart->setEnabled(false);
		}
		catch (const GLogiKExcept & e) {
			/*  make service start button visible and usable only when
			 *  service is not started */
			std::string reason(e.what());
			/* got DBus error as reply : The name com.glogik.Client
			 * was not provided by any .service files */
			if(reason.find("not provided") != std::string::npos) {
				_pStart->setVisible(true);
				_pStart->setEnabled(true);
			}

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

void DaemonAndServiceTab::startSignal(void)
{
	_pStart->setEnabled(false);

	std::string status("desktop service request");
	try {
		/* asking the launcher for the desktop service start */
		_pDBus->initializeBroadcastSignal(
			NSGKDBus::BusConnection::GKDBUS_SESSION,
			GLOGIK_DESKTOP_QT5_SESSION_DBUS_OBJECT_PATH,
			GLOGIK_DESKTOP_QT5_SESSION_DBUS_INTERFACE,
			"RestartRequest"
		);
		_pDBus->sendBroadcastSignal();

		status += " sent to launcher";
		LOG(INFO) << status;
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_pDBus->abandonBroadcastSignal();
		status += " to launcher failed";
		LOG(ERROR) << status << " - " << e.what();
		throw GLogiKExcept("service RestartRequest failed");
	}
}

} // namespace GLogiK

