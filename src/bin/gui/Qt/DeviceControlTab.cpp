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

#include <vector>

#include <QVBoxLayout>
#include <QHBoxLayout>

#include "DeviceControlTab.hpp"

#include "lib/shared/glogik.hpp"
#include "lib/utils/utils.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

DeviceControlTab::DeviceControlTab(
	NSGKDBus::GKDBus* pDBus,
	const QString & name
)	:	_pDBus(pDBus),
		_deviceStatus(nullptr),
		_pStart(nullptr),
		_pStop(nullptr),
		_pRestart(nullptr),
		_line2(nullptr),
		_line3(nullptr)
{
	this->setObjectName(name);

	QVBoxLayout *vBox = nullptr;
	QHBoxLayout* hBox = nullptr;

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

		_deviceStatus = new QLabel("aaaa");
		vBox->addWidget(_deviceStatus);

		hBox = new QHBoxLayout();
#if DEBUGGING_ON
		LOG(DEBUG1) << "allocated QHBoxLayout";
#endif

		/* -- -- -- */

		{
			_line2 = new QFrame();
#if DEBUGGING_ON
			LOG(DEBUG2) << "allocated QFrame";
#endif
			_line2->setFrameShape(QFrame::HLine);
			_line2->setFrameShadow(QFrame::Sunken);
			vBox->addWidget(_line2);
		}
		/* -- -- -- */

		vBox->addLayout(hBox);

		/* -- -- -- */

		{
			_line3 = new QFrame();
#if DEBUGGING_ON
			LOG(DEBUG2) << "allocated QFrame";
#endif
			_line3->setFrameShape(QFrame::HLine);
			_line3->setFrameShadow(QFrame::Sunken);
			vBox->addWidget(_line3);
		}

		/* -- -- -- */

		vBox->addSpacing(300);

		/* -- -- -- */

		_pStart = new QPushButton("Start");
#if DEBUGGING_ON
		LOG(DEBUG1) << "allocated Start button";
#endif
		_pStart->setObjectName("StartButton");
		hBox->addWidget(_pStart);

		_pStop = new QPushButton("Stop");
#if DEBUGGING_ON
		LOG(DEBUG1) << "allocated Stop button";
#endif
		_pStop->setObjectName("StopButton");
		hBox->addWidget(_pStop);

		_pRestart = new QPushButton("Restart");
#if DEBUGGING_ON
		LOG(DEBUG1) << "allocated Restart button";
#endif
		_pRestart->setObjectName("RestartButton");
		hBox->addWidget(_pRestart);
	}
	catch (const std::bad_alloc& e) {
		LOG(ERROR) << e.what();
		throw;
	}

	connect(_pStart   , &QPushButton::clicked, this, &DeviceControlTab::startSignal);
	connect(_pStop    , &QPushButton::clicked, this, &DeviceControlTab::stopSignal);
	connect(_pRestart , &QPushButton::clicked, this, &DeviceControlTab::restartSignal);

	this->disableAndHide();
}

DeviceControlTab::~DeviceControlTab()
{
}

void DeviceControlTab::disableButtons(void)
{
	_pStart->setEnabled(false);
	_pStop->setEnabled(false);
	_pRestart->setEnabled(false);
}

void DeviceControlTab::disableAndHide(void)
{
	this->disableButtons();

	this->setVisibility(false);

	_deviceStatus->setText("Please select a device into the combo box above.");
}

void DeviceControlTab::setVisibility(const bool visibility)
{
	_pStart->setVisible(visibility);
	_pStop->setVisible(visibility);
	_pRestart->setVisible(visibility);
	_line2->setVisible(visibility);
	_line3->setVisible(visibility);
}

void DeviceControlTab::updateTab(
	const std::string & devID,
	const bool status)
{
#if DEBUGGING_ON
	LOG(DEBUG1) << "updating tab for device " << devID;
#endif
	_devID = devID;

	if(status) { /* device status == "started" */
		_pStart->setEnabled(false);
		_pStop->setEnabled(true);
		_pRestart->setEnabled(true);

		this->setVisibility(true);

		_deviceStatus->setText("Device status : started");
	}
	else {
		_pStart->setEnabled(true);
		_pStop->setEnabled(false);
		_pRestart->setEnabled(false);

		this->setVisibility(true);

		_deviceStatus->setText("Device status : stopped");
	}
}

void DeviceControlTab::sendStatusSignal(const std::string & signal)
{
	LOG(INFO) << "sending " << signal << " signal for device " << _devID;

	// TODO : would need to implement StringToVoid GKDBus template to avoid vector
	const std::vector<std::string> devIDArray = {_devID};

	try {
		_pDBus->initializeBroadcastSignal(
			NSGKDBus::BusConnection::GKDBUS_SESSION,
			GLOGIK_DESKTOP_QT5_SESSION_DBUS_OBJECT_PATH,
			GLOGIK_DESKTOP_QT5_SESSION_DBUS_INTERFACE,
			signal.c_str()
		);
		_pDBus->appendStringVectorToBroadcastSignal(devIDArray);
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
	this->sendStatusSignal("DeviceStartRequest");
}

void DeviceControlTab::stopSignal(void)
{
	this->sendStatusSignal("DeviceStopRequest");
}

void DeviceControlTab::restartSignal(void)
{
	this->sendStatusSignal("DeviceRestartRequest");
}

} // namespace GLogiK

