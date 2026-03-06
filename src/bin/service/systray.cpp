/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2026  Fabrice Delliaux <netbox253@gmail.com>
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

#include <new>

#include <QString>
#include <QIcon>

#include "lib/utils/utils.hpp"

#include "systray.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

DesktopServiceSystray::DesktopServiceSystray(void)
	:	_deviceID(""),
		_deviceEvent(SystrayDeviceEvent::DEVICE_STOP),
		_trayIcon(nullptr),
		_trayIconMenu(nullptr),
		_deviceEventTriggered(false),
		_wantToRestart(false),
		_wantToStop(false)
{
	try
	{
		_trayIcon = new QSystemTrayIcon(this);
		{
			QString icon(DATA_DIR);
			icon += "/icons/hicolor/48x48/apps/GLogiK.png";
			_trayIcon->setIcon(QIcon(icon));
		}

		_trayIconMenu = new QMenu(this);
	}
	catch (const std::bad_alloc& e)
	{ /* handle new() failure */
		throw GLogiKBadAlloc("catch wrong allocation");
	}
}

DesktopServiceSystray::~DesktopServiceSystray(void)
{
}

void DesktopServiceSystray::resetDeviceEvent(void)
{
	_deviceEventTriggered = false;
}

void DesktopServiceSystray::updateContextMenu(const DevicesMap_type & devices)
{
	_trayIcon->hide();

	try
	{
		/* clear the entire menu. All actions owned by the menu are deleted, and
		 * all signals to and from these actions are automatically disconnected
		 * see QObject destructor : https://doc.qt.io/qt-6/qobject.html#dtor.QObject
		 */
		_trayIconMenu->clear();

		{ /* rebuilding the menu */
			{
				QMenu* devicesSubmenu;

				{
					QString devicesTitle("no device connected");
					const DevicesMap_type::size_type n = devices.size();
					if(n == 1)
						devicesTitle = "1 device connected";
					else if(n > 1)
					{
						devicesTitle.setNum(n);
						devicesTitle += " devices connected";
					}

					devicesSubmenu = _trayIconMenu->addMenu(devicesTitle);
					if(n == 0)
						devicesSubmenu->setDisabled(true);
				}

				for(const auto & devicePair : devices)
				{ // <<< start/restart/stop events for each device
					auto & devID = devicePair.first;
					auto & deviceID = devicePair.second;

					const QString QdevID = QString::fromStdString(devID);
					QMenu* deviceEventMenu = devicesSubmenu->addMenu(QdevID);

					{ // <<< device identification
						QString dev;
						dev += deviceID.getVendor();
						dev += " ";
						dev += deviceID.getProduct();
						dev += " ";
						dev += deviceID.getName();
						QAction* fullname = new QAction(dev, this);
						fullname->setEnabled(false);

						deviceEventMenu->addAction(fullname);
					} // >>>

					deviceEventMenu->addSeparator();

					{ // <<< start action
						QAction* startDevice = new QAction("start", this);
						if(deviceID.getStatus() == "started")
							startDevice->setEnabled(false);
						else
						{
							startDevice->setData(QdevID);
							QObject::connect(startDevice, &QAction::triggered,
								this, &DesktopServiceSystray::startDevice);
						}

						deviceEventMenu->addAction(startDevice);
					} // >>>

					{ // <<< restart action
						QAction* restartDevice = new QAction("restart", this);
						if(deviceID.getStatus() == "stopped")
							restartDevice->setEnabled(false);
						else
						{
							restartDevice->setData(QdevID);
							QObject::connect(restartDevice, &QAction::triggered,
								this, &DesktopServiceSystray::restartDevice);
						}

						deviceEventMenu->addAction(restartDevice);
					} // >>>

					deviceEventMenu->addSeparator();

					{ // <<< stop action
						QAction* stopDevice = new QAction("stop", this);
						if(deviceID.getStatus() == "stopped")
							stopDevice->setEnabled(false);
						else
						{
							stopDevice->setData(QdevID);
							QObject::connect(stopDevice, &QAction::triggered,
								this, &DesktopServiceSystray::stopDevice);
						}

						deviceEventMenu->addAction(stopDevice);
					} // >>>
				} // >>>
			} // end Devices submenu

			_trayIconMenu->addSeparator();

			QMenu* serviceSubmenu = _trayIconMenu->addMenu("Service");

			{
				QAction* restart = new QAction("restart", this);
				QObject::connect(restart, &QAction::triggered,
					this, &DesktopServiceSystray::restartService);

				QAction* stop = new QAction("stop", this);
				stop->setIcon( QIcon::fromTheme(QIcon::ThemeIcon::ApplicationExit) );
				QObject::connect(stop, &QAction::triggered,
					this, &DesktopServiceSystray::stopService);

				serviceSubmenu->addAction(restart);
				serviceSubmenu->addSeparator();
				serviceSubmenu->addAction(stop);
			} // end Service submenu
		}

		_trayIcon->setContextMenu(_trayIconMenu);
		_trayIcon->show();
	}
	catch (const std::bad_alloc& e)
	{ /* handle new() failure */
		throw GLogiKBadAlloc("catch wrong allocation");
	}
}

void DesktopServiceSystray::restartService(void)
{
	_wantToRestart = true;
}

void DesktopServiceSystray::stopService(void)
{
	_wantToStop = true;
}

void DesktopServiceSystray::startDevice(void)
{
	GK_LOG_FUNC

	QAction* pAction = qobject_cast<QAction*>(QObject::sender());
	if(pAction == nullptr)
	{
		LOG(warning) << "cannot get sender pointer";
		return;
	}

	QVariant v = pAction->data();
	QString QdevID = v.value<QString>();
	if(QdevID.isEmpty())
	{
		LOG(warning) << "QVariant conversion failure";
		return;
	}

	_deviceID = QdevID.toStdString();
	_deviceEvent = SystrayDeviceEvent::DEVICE_START;
	_deviceEventTriggered = true;

	GKLog2(trace, "systray starting device action: ", _deviceID)
}

void DesktopServiceSystray::stopDevice(void)
{
	GK_LOG_FUNC

	QAction* pAction = qobject_cast<QAction*>(QObject::sender());
	if(pAction == nullptr)
	{
		LOG(warning) << "cannot get sender pointer";
		return;
	}

	QVariant v = pAction->data();
	QString QdevID = v.value<QString>();
	if(QdevID.isEmpty())
	{
		LOG(warning) << "QVariant conversion failure";
		return;
	}

	_deviceID = QdevID.toStdString();
	_deviceEvent = SystrayDeviceEvent::DEVICE_STOP;
	_deviceEventTriggered = true;

	GKLog2(trace, "systray stopping device action: ", _deviceID)
}

void DesktopServiceSystray::restartDevice(void)
{
	GK_LOG_FUNC

	QAction* pAction = qobject_cast<QAction*>(QObject::sender());
	if(pAction == nullptr)
	{
		LOG(warning) << "cannot get sender pointer";
		return;
	}

	QVariant v = pAction->data();
	QString QdevID = v.value<QString>();
	if(QdevID.isEmpty())
	{
		LOG(warning) << "QVariant conversion failure";
		return;
	}

	_deviceID = QdevID.toStdString();
	_deviceEvent = SystrayDeviceEvent::DEVICE_RESTART;
	_deviceEventTriggered = true;

	GKLog2(trace, "systray restarting device action: ", _deviceID)
}
} // namespace GLogiK
