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

#include <QVBoxLayout>
#include <QHBoxLayout>

#include "lib/shared/glogik.hpp"
#include "lib/utils/utils.hpp"

#include "DeviceControlTab.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

DeviceControlTab::DeviceControlTab(
	NSGKDBus::GKDBus* pDBus,
	const QString & name)
	:	Tab(pDBus),
		_deviceStatusLabel(nullptr),
		_pStartButton(nullptr),
		_pStopButton(nullptr),
		_pRestartButton(nullptr),
		_HLineFrame2(nullptr),
		_HLineFrame3(nullptr)
{
	this->setObjectName(name);
}

DeviceControlTab::~DeviceControlTab()
{
}

void DeviceControlTab::buildTab(void)
{
	GK_LOG_FUNC

	try {
		QVBoxLayout* vBox = new QVBoxLayout(this);
		GKLog(trace, "allocated QVBoxLayout")

		this->setLayout(vBox);

		/* -- -- -- */

		vBox->addWidget( this->getHLine() );

		/* -- -- -- */

		_deviceStatusLabel = new QLabel("Device status : unknown");
		vBox->addWidget(_deviceStatusLabel);

		/* -- -- -- */

		_HLineFrame2 = this->getHLine();
		vBox->addWidget( _HLineFrame2 );

		/* -- -- -- */
		{
			QHBoxLayout* hBox = new QHBoxLayout();
			GKLog(trace, "allocated QHBoxLayout")

			/* -- -- -- */

			vBox->addLayout(hBox);

			/* -- -- -- */

			_pStartButton = new QPushButton("Start");
			GKLog(trace, "allocated Start button")
			hBox->addWidget(_pStartButton);

			_pStopButton = new QPushButton("Stop");
			GKLog(trace, "allocated Stop button")
			hBox->addWidget(_pStopButton);

			_pRestartButton = new QPushButton("Restart");
			GKLog(trace, "allocated Restart button")
			hBox->addWidget(_pRestartButton);
		}
		/* -- -- -- */

		_HLineFrame3 = this->getHLine();
		vBox->addWidget( _HLineFrame3 );

		/* -- -- -- */

		vBox->addSpacing(300);
	}
	catch (const std::bad_alloc& e) {
		LOG(error) << "bad allocation : " << e.what();
		throw;
	}

	QObject::connect(_pStartButton   , &QPushButton::clicked, this, &DeviceControlTab::startSignal);
	QObject::connect(_pStopButton    , &QPushButton::clicked, this, &DeviceControlTab::stopSignal);
	QObject::connect(_pRestartButton , &QPushButton::clicked, this, &DeviceControlTab::restartSignal);

	this->disableAndHide();
}

void DeviceControlTab::disableButtons(void)
{
	_pStartButton->setEnabled(false);
	_pStopButton->setEnabled(false);
	_pRestartButton->setEnabled(false);
}

void DeviceControlTab::disableAndHide(void)
{
	this->disableButtons();

	this->setVisibility(false);

	_deviceStatusLabel->setText("Please select a device into the combo box above.");
}

void DeviceControlTab::setVisibility(const bool visibility)
{
	_pStartButton->setVisible(visibility);
	_pStopButton->setVisible(visibility);
	_pRestartButton->setVisible(visibility);
	_HLineFrame2->setVisible(visibility);
	_HLineFrame3->setVisible(visibility);
}

void DeviceControlTab::updateTab(
	const DeviceProperties & device,
	const std::string & devID)
{
	GK_LOG_FUNC

	GKLog2(trace, "updating DeviceControlTab, device ", devID)

	_devID = devID;

	if(device.getStatus() == "started") {
		_pStartButton->setEnabled(false);
		_pStopButton->setEnabled(true);
		_pRestartButton->setEnabled(true);

		this->setVisibility(true);

		_deviceStatusLabel->setText("Device status : started");
	}
	else {
		_pStartButton->setEnabled(true);
		_pStopButton->setEnabled(false);
		_pRestartButton->setEnabled(false);

		this->setVisibility(true);

		_deviceStatusLabel->setText("Device status : stopped");
	}
}

void DeviceControlTab::sendStatusSignal(const std::string & signal)
{
	GK_LOG_FUNC

	LOG(info) << "sending " << signal << " signal for device " << _devID;

	try {
		_pDBus->initializeBroadcastSignal(
			_sessionBus,
			GLOGIK_DESKTOP_QT5_SESSION_DBUS_OBJECT_PATH,
			GLOGIK_DESKTOP_QT5_SESSION_DBUS_INTERFACE,
			"DeviceStatusChangeRequest"
		);
		_pDBus->appendStringToBroadcastSignal(_devID);
		_pDBus->appendStringToBroadcastSignal(signal);
		_pDBus->sendBroadcastSignal();
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_pDBus->abandonBroadcastSignal();
		LOG(error) << e.what();
	}

	this->disableButtons();
}

void DeviceControlTab::startSignal(void)
{
	this->sendStatusSignal("StartDevice");
}

void DeviceControlTab::stopSignal(void)
{
	this->sendStatusSignal("StopDevice");
}

void DeviceControlTab::restartSignal(void)
{
	this->sendStatusSignal("RestartDevice");
}

} // namespace GLogiK

