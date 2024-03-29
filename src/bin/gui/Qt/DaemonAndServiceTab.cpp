/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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

#include <QPixmap>
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
		_daemonVersionLabel(nullptr),
		_serviceVersionLabel(nullptr),
		_serviceStatusLabel(nullptr),
		_serviceRegistered(false)
{
	this->setObjectName(name);
}

DaemonAndServiceTab::~DaemonAndServiceTab()
{
}

void DaemonAndServiceTab::buildTab(void)
{
	GK_LOG_FUNC

	try {
		QVBoxLayout* vBox = new QVBoxLayout(this);
		GKLog(trace, "allocated QVBoxLayout")

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

			QLabel* logoLabel = new QLabel();
			layout->addWidget(logoLabel);

			QString logo(DATA_DIR);
			logo += "/icons/hicolor/96x96/apps/GLogiK.png";

			QPixmap image(logo);
			logoLabel->setPixmap(image);
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

			_pStartButton = new QPushButton("Start service");
			GKLog(trace, "allocated Start button")

			layout->addWidget(_pStartButton);

			/* keeping space used when hiding button */
			QSizePolicy retain = _pStartButton->sizePolicy();
			retain.setRetainSizeWhenHidden(true);
			_pStartButton->setSizePolicy(retain);

			_pStartButton->setEnabled(false);

			QObject::connect(_pStartButton, &QPushButton::clicked,
				this, &DaemonAndServiceTab::sendServiceStartRequest);
		}

		/* -- -- -- */
	}
	catch (const std::bad_alloc& e) {
		LOG(error) << "bad allocation : " << e.what();
		throw;
	}
}

const bool DaemonAndServiceTab::isServiceRegistered(void) const
{
	return _serviceRegistered;
}

void DaemonAndServiceTab::updateTab(void)
{
	GK_LOG_FUNC

	GKLog(trace, "updating DaemonAndServiceTab")

	{
		const QString vers("Version : unknown");
		_daemonVersionLabel->setText(vers);
		_serviceVersionLabel->setText(vers);

		/* assuming service stopped */
		_serviceStatusLabel->setText("Status : stopped");
		_serviceRegistered = false;
		_pStartButton->setVisible(false);
		_pStartButton->setEnabled(false);
	}

	const std::string remoteMethod("GetInformations");
	try {
		_pDBus->initializeRemoteMethodCall(
			_sessionBus,
			GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		_pDBus->appendStringToRemoteMethodCall("reserved");

		_pDBus->sendRemoteMethodCall();

		try {
			_pDBus->waitForRemoteMethodCallReply();

			const std::vector<std::string> ret = _pDBus->getNextStringArray();
			if(ret.size() != 3)
				throw GLogiKExcept("wrong informations array size");

			QString labelText("Version : ");
			labelText += ret[0].c_str();
			_daemonVersionLabel->setText(labelText);

			labelText = "Version : ";
			labelText += ret[1].c_str();
			_serviceVersionLabel->setText(labelText);

			labelText = "Status : started and ";
			QString status(ret[2].c_str());
			labelText += status;
			_serviceStatusLabel->setText(labelText);

			_serviceRegistered = (status == "registered");

			_pStartButton->setVisible(false);
			_pStartButton->setEnabled(false);
		}
		catch (const GLogiKExcept & e) {
			/*  make service start button visible and usable only when
			 *  service is not started */
			std::string reason(e.what());
			/* got DBus error as reply : The name com.glogik.Client
			 * was not provided by any .service files */
			if(reason.find("not provided") != std::string::npos) {
				_pStartButton->setVisible(true);
				_pStartButton->setEnabled(true);
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

void DaemonAndServiceTab::sendServiceStartRequest(void)
{
	GK_LOG_FUNC

	_pStartButton->setEnabled(false);

	std::string status("desktop service seems not started, request");
	try {
		/* asking the launcher to spawn the service after sleeping 100 ms */
		_pDBus->initializeBroadcastSignal(
			_sessionBus,
			GLOGIK_DESKTOP_QT5_SESSION_DBUS_OBJECT_PATH,
			GLOGIK_DESKTOP_QT5_SESSION_DBUS_INTERFACE,
			"ServiceStartRequest"
		);
		_pDBus->appendUInt16ToBroadcastSignal(100);
		_pDBus->sendBroadcastSignal();

		status += " sent to launcher";
		LOG(info) << status;
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_pDBus->abandonBroadcastSignal();
		status += " to launcher failed";
		LOG(error) << status << " - " << e.what();
	}
}

} // namespace GLogiK

