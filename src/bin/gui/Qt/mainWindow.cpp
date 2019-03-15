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

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include <new>
#include <stdexcept>
#include <vector>

#include <QString>
#include <QStringList>
#include <QTimer>
#include <QtWidgets>
#include <QFrame>
#include <QVBoxLayout>
#include <QComboBox>
#include <QTabWidget>

#include <boost/filesystem.hpp>

#include "lib/shared/glogik.hpp"
#include "lib/shared/deviceFile.hpp"
#include "lib/utils/utils.hpp"

#include "lib/dbus/arguments/GKDBusArgString.hpp"

#include "Tab.hpp"
#include "DaemonAndServiceTab.hpp"
#include "BacklightColorTab.hpp"
#include "DeviceControlTab.hpp"
#include "mainWindow.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

MainWindow::MainWindow(QWidget *parent)
	:	QMainWindow(parent),
		_pDBus(nullptr),
		_pid(0),
		_LOGfd(nullptr),
		_GUIResetThrow(true),
		_devicesComboBox(nullptr)
{
	LOG_TO_FILE_AND_CONSOLE::ConsoleReportingLevel() = INFO;
	if( LOG_TO_FILE_AND_CONSOLE::ConsoleReportingLevel() != NONE ) {
		LOG_TO_FILE_AND_CONSOLE::ConsoleStream() = stderr;
	}

#if DEBUGGING_ON
	LOG_TO_FILE_AND_CONSOLE::FileReportingLevel() = DEBUG3;

	if( LOG_TO_FILE_AND_CONSOLE::FileReportingLevel() != NONE ) {
		const std::string pid( std::to_string( getpid() ) );

		fs::path debugFile(DEBUG_DIR);
		debugFile /= "GKcQt5-debug-";
		debugFile += pid;
		debugFile += ".log";

		errno = 0;
		_LOGfd = std::fopen(debugFile.string().c_str(), "w");

		if(_LOGfd == nullptr) {
			LOG(ERROR) << "failed to open debug file";
			if(errno != 0) {
				LOG(ERROR) << strerror(errno);
			}
		}
		else fs::permissions(debugFile, fs::owner_read|fs::owner_write|fs::group_read);

		LOG_TO_FILE_AND_CONSOLE::FileStream() = _LOGfd;
	}
#endif

	if( _LOGfd == nullptr ) {
		LOG(INFO) << "debug file not opened";
	}
}

MainWindow::~MainWindow() {
	delete _pDBus; _pDBus = nullptr;

#if DEBUGGING_ON
	LOG(DEBUG2) << "exiting MainWindow process";
#endif

	LOG(INFO) << "GKcQt5 : bye !";
	if( _LOGfd != nullptr )
		std::fclose(_LOGfd);
}

/*
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 *
 * === public === public === public === public === public ===
 *
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 */

void MainWindow::init(void)
{
	LOG(INFO) << "Starting GKcQt5 vers. " << VERSION;

	try {
		_pDBus = new NSGKDBus::GKDBus(GLOGIK_DESKTOP_QT5_DBUS_ROOT_NODE, GLOGIK_DESKTOP_QT5_DBUS_ROOT_NODE_PATH);
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		throw GLogiKBadAlloc("GKDBus bad allocation");
	}

	_pDBus->connectToSessionBus(GLOGIK_DESKTOP_QT5_DBUS_BUS_CONNECTION_NAME);

	_pDBus->NSGKDBus::EventGKDBusCallback<VoidToVoid>::exposeSignal(
		NSGKDBus::BusConnection::GKDBUS_SESSION,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		"DevicesUpdated",
		{},
		std::bind(&MainWindow::resetInterface, this)
	);

	/* initializing timer */
	QTimer* timer = nullptr;
	try {
		timer = new QTimer(this);
	}
	catch (const std::bad_alloc& e) {
		throw GLogiKBadAlloc("QTimer bad alloc :(");
	}

	timer->setObjectName("CheckTimer");
	connect(timer, &QTimer::timeout, this, &MainWindow::checkDBusMessages);

	this->statusBar();
}

void MainWindow::build(void)
{
	QVBoxLayout* vBox = nullptr;

	try {
		QFrame *frame = new QFrame(this);
		this->setCentralWidget(frame);

		vBox = new QVBoxLayout(frame);

#if DEBUGGING_ON
		LOG(DEBUG1) << "allocated QVBoxLayout";
#endif

		/* -- -- -- */

		_devicesComboBox = new QComboBox();
#if DEBUGGING_ON
		LOG(DEBUG2) << "allocated QComboBox";
#endif
		_devicesComboBox->setObjectName("devicesList");

		vBox->addWidget(_devicesComboBox);

		/* -- -- -- */

		QFrame* line = new QFrame();
#if DEBUGGING_ON
		LOG(DEBUG2) << "allocated QFrame";
#endif

		line->setFrameShape(QFrame::HLine);
		line->setFrameShadow(QFrame::Sunken);

		vBox->addWidget(line);
		/* -- -- -- */

		QTabWidget* tabWidget = new QTabWidget();
#if DEBUGGING_ON
		LOG(DEBUG2) << "allocated QTabWidget";
#endif
		tabWidget->setObjectName("Tabs");
		vBox->addWidget(tabWidget);

		tabWidget->addTab(new DaemonAndServiceTab(_pDBus, "DaemonAndService"), tr("Daemon and Service"));
		tabWidget->addTab(new DeviceControlTab(_pDBus, "DeviceControl"), tr("Device Control"));
		tabWidget->addTab(new BacklightColorTab("BacklightColor"), tr("Backlight Color"));
		//tabWidget->addTab(new QWidget(), tr("Multimedia Keys"));
		//tabWidget->addTab(new QWidget(), tr("LCD Screen Plugins"));
		//tabWidget->addTab(new QWidget(), tr("Macros"));
#if DEBUGGING_ON
		LOG(DEBUG3) << "allocated 5 tabs";
#endif

		this->setTabEnabled("DaemonAndService", true);
		this->setTabEnabled("DeviceControl", true);
		this->setTabEnabled("BacklightColor", false);

		/* -- -- -- */

		/* launching timer */
		QTimer *timer = this->findChild<QTimer *>("CheckTimer", Qt::FindDirectChildrenOnly);
		timer->start(200);
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		throw GLogiKBadAlloc("Qt bad alloc :(");
	}

	this->resetInterface();
	_GUIResetThrow = false; /* after here, don't throw if reset interface fails */
	this->initializeQtSignalsSlots();
}

/*
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 *
 * === private === private === private === private === private ===
 *
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 */

void MainWindow::initializeQtSignalsSlots(void)
{
	connect(_devicesComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updateInterface);
};

void MainWindow::updateInterface(int index)
{
	/*  don't update interface on combo clear() */
	if(index == -1) {
		return;
	}
#if DEBUGGING_ON
	LOG(DEBUG2) << "updating interface with index : " << index;
#endif

	try {
		QTabWidget* tabWidget = nullptr;
		QWidget* tab = nullptr;

		this->setTabWidgetPointers("DeviceControl", tabWidget, tab);
		DeviceControlTab* deviceControlTab = dynamic_cast<DeviceControlTab*>(tab);

		if(index == 0) {
			deviceControlTab->disableAndHide();
			this->setTabEnabled("BacklightColor", false);
			this->statusBar()->showMessage("Selected device : none", 2000);
		}
		else {
			const std::string devID( _devicesComboBox->currentText().split(" ").at(0).toStdString() );
#if DEBUGGING_ON
			LOG(DEBUG3) << "devID: " << devID;
#endif

			try {
				const DeviceFile & device = _devices.at(devID);
				const bool status = (device.getStatus() == "started");

				deviceControlTab->updateTab(devID, status);
				this->setTabEnabled("BacklightColor", status);
			}
			catch (const std::out_of_range& oor) {
				std::string error("device not found in container : ");
				error += devID;
				throw GLogiKExcept(error);
			}

			QString msg("Selected device : ");
			msg += _devicesComboBox->currentText();
			this->statusBar()->showMessage(msg, 2000);
		}
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << "error updating interface : " << e.what();
	}
}

void MainWindow::updateDevicesList(void)
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "updating devices list";
#endif

	_devices.clear();

	const std::string remoteMethod("GetDevicesList");
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

			/* this block could be part of GKDBus, but we want to avoid to link
			 * the GKDBus library to GKShared (to get DeviceFile object) */
			try {
				using namespace NSGKDBus;
				do {
					DeviceFile device;

					const std::string devID( GKDBusArgumentString::getNextStringArgument() );

					device.setStatus( GKDBusArgumentString::getNextStringArgument() );
					device.setVendor( GKDBusArgumentString::getNextStringArgument() );
					device.setModel( GKDBusArgumentString::getNextStringArgument() );
					device.setConfigFilePath( GKDBusArgumentString::getNextStringArgument() );

					_devices[devID] = device;
				}
				while( ! GKDBusArgumentString::isContainerEmpty() );
			}
			catch ( const EmptyContainer & e ) {
				LOG(WARNING) << "missing argument : " << e.what();
				throw GLogiKExcept("rebuilding device map failed");
			}

			QString msg("Detected ");
			msg += QString::number(_devices.size());
			msg += " device(s)";

			LOG(INFO) << msg.toStdString();
			this->statusBar()->showMessage(msg, 2000);
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

void MainWindow::resetInterface(void)
{
#if DEBUGGING_ON
	LOG(DEBUG1) << "resetting interface";
#endif

	try {
		this->updateDevicesList();

		/* clear() set current index to -1 */
		_devicesComboBox->clear();
		{
			QStringList items = {""}; // index 0
			for(const auto & devicePair : _devices) {
				std::string item( devicePair.first );
				item += " ";
				item += devicePair.second.getVendor();
				item += " ";
				item += devicePair.second.getModel();
				std::replace( item.begin(), item.end(), '_', ' ');
				items << item.c_str();
			}
			/* additems() set current index to 0 */
			_devicesComboBox->addItems(items);
		}

		QTabWidget* tabWidget = nullptr;
		QWidget* tab = nullptr;

		this->setTabWidgetPointers("DaemonAndService", tabWidget, tab);
		DaemonAndServiceTab* daemonAndServiceTab = dynamic_cast<DaemonAndServiceTab*>(tab);
		daemonAndServiceTab->updateTab();

		this->setTabWidgetPointers("DeviceControl", tabWidget, tab);
		DeviceControlTab * deviceControlTab = dynamic_cast<DeviceControlTab*>(tab);
		deviceControlTab->disableButtons();
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << "error resetting interface : " << e.what();
		if(_GUIResetThrow) {
			throw GLogiKExcept("interface reset failure");
		}
	}
}

void MainWindow::setTabWidgetPointers(
	const std::string & name,
	QTabWidget*& tabWidget,
	QWidget*& tab)
{
	tabWidget = this->findChild<QTabWidget*>("Tabs");
	if(tabWidget == 0) {
		LOG(ERROR) << "tabWidget not found : " << name;
		throw GLogiKExcept("tabWidget not found");
	}

	QString n(name.c_str());
	tab = tabWidget->findChild<QWidget *>(n);
	if(tab == 0) {
		LOG(ERROR) << "tab not found : " << name;
		throw GLogiKExcept("tab not found in tabWidget");
	}
}

void MainWindow::setTabEnabled(const std::string & name, const bool status)
{
	try {
		QTabWidget* tabWidget = nullptr;
		QWidget* tab = nullptr;
		this->setTabWidgetPointers(name, tabWidget, tab);

		int index = tabWidget->indexOf(tab);
		if(index == -1) {
			LOG(ERROR) << "index not found : " << name;
			throw GLogiKExcept("tab index not found");
		}

		tabWidget->setTabEnabled(index, status);
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << "error setting TabEnabled property : " << e.what();
		throw;
	}
}

void MainWindow::checkDBusMessages(void)
{
	_pDBus->checkForNextMessage(NSGKDBus::BusConnection::GKDBUS_SESSION);
}

} // namespace GLogiK

