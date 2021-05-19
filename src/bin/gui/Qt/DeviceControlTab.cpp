/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
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

	QVBoxLayout* vBox = nullptr;
	QHBoxLayout* hBox = nullptr;

	try {
		vBox = new QVBoxLayout(this);
#if DEBUGGING_ON
		LOG(DEBUG1) << "allocated QVBoxLayout";
#endif
		this->setLayout(vBox);

		/* -- -- -- */

		vBox->addWidget( this->getHLine() );

		/* -- -- -- */

		_deviceStatusLabel = new QLabel("aaaa");
		vBox->addWidget(_deviceStatusLabel);

		hBox = new QHBoxLayout();
#if DEBUGGING_ON
		LOG(DEBUG1) << "allocated QHBoxLayout";
#endif

		/* -- -- -- */

		_HLineFrame2 = this->getHLine();
		vBox->addWidget( _HLineFrame2 );

		/* -- -- -- */

		vBox->addLayout(hBox);

		/* -- -- -- */

		_HLineFrame3 = this->getHLine();
		vBox->addWidget( _HLineFrame3 );

		/* -- -- -- */

		vBox->addSpacing(300);

		/* -- -- -- */

		_pStartButton = new QPushButton("Start");
#if DEBUGGING_ON
		LOG(DEBUG1) << "allocated Start button";
#endif
		hBox->addWidget(_pStartButton);

		_pStopButton = new QPushButton("Stop");
#if DEBUGGING_ON
		LOG(DEBUG1) << "allocated Stop button";
#endif
		hBox->addWidget(_pStopButton);

		_pRestartButton = new QPushButton("Restart");
#if DEBUGGING_ON
		LOG(DEBUG1) << "allocated Restart button";
#endif
		hBox->addWidget(_pRestartButton);
	}
	catch (const std::bad_alloc& e) {
		LOG(ERROR) << e.what();
		throw;
	}

	connect(_pStartButton   , &QPushButton::clicked, this, &DeviceControlTab::startSignal);
	connect(_pStopButton    , &QPushButton::clicked, this, &DeviceControlTab::stopSignal);
	connect(_pRestartButton , &QPushButton::clicked, this, &DeviceControlTab::restartSignal);

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
	const std::string & devID,
	const bool status)
{
	GK_LOG_FUNC

#if DEBUGGING_ON
	LOG(DEBUG1) << "updating DeviceControlTab";
	LOG(DEBUG2) << "device " << devID;
#endif
	_devID = devID;

	if(status) { /* device status == "started" */
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

	LOG(INFO) << "sending " << signal << " signal for device " << _devID;

	try {
		_pDBus->initializeBroadcastSignal(
			NSGKDBus::BusConnection::GKDBUS_SESSION,
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
		LOG(ERROR) << e.what();
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

