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

#include <csignal>

#include <new>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <functional>

#include <QString>
#include <QStringList>
#include <QTimer>
#include <QtWidgets>
#include <QFrame>
#include <QVBoxLayout>
#include <QComboBox>
#include <QTabWidget>
#include <QMenu>
#include <QMetaObject>
#include <QIcon>
#include <QMessageBox>

#include "lib/shared/glogik.hpp"
#include "lib/shared/deviceConfigurationFile.hpp"
#include "lib/utils/utils.hpp"

#include "include/enums.hpp"

#include "AboutDialog.hpp"
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
		_serviceStartRequest(false),
		_statusBarTimeout(3000),
		_devicesComboBox(nullptr),
		_tabbedWidgets(nullptr),
		_daemonAndServiceTab(nullptr),
		_deviceControlTab(nullptr),
		_backlightColorTab(nullptr),
		_LCDPluginsTab(nullptr)
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
		else {
			boost::system::error_code ec;
			fs::permissions(debugFile, fs::owner_read|fs::owner_write|fs::group_read, ec);
			if( ec.value() != 0 ) {
				LOG(ERROR) << "failure to set debug file permissions : " << ec.value();
			}
		}

		LOG_TO_FILE_AND_CONSOLE::FileStream() = _LOGfd;
	}
#endif

	if( _LOGfd == nullptr ) {
		LOG(INFO) << "debug file not opened";
	}

	std::signal(SIGINT, MainWindow::handleSignal);
	std::signal(SIGTERM, MainWindow::handleSignal);
	std::signal(SIGHUP, MainWindow::handleSignal);
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

void MainWindow::handleSignal(int sig)
{
	LOG(INFO) << "catched signal : " << sig;
	QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
}

void MainWindow::init(void)
{
	LOG(INFO) << "Starting GKcQt5 vers. " << VERSION;

	try {
		_pDBus = new NSGKDBus::GKDBus(GLOGIK_DESKTOP_QT5_DBUS_ROOT_NODE, GLOGIK_DESKTOP_QT5_DBUS_ROOT_NODE_PATH);
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		throw GLogiKBadAlloc("GKDBus bad allocation");
	}

	_pDBus->connectToSessionBus(GLOGIK_DESKTOP_QT5_DBUS_BUS_CONNECTION_NAME, NSGKDBus::ConnectionFlag::GKDBUS_SINGLE);

	this->statusBar();

	_configurationRootDirectory = XDGUserDirs::getConfigurationRootDirectory();
	_configurationRootDirectory /= PACKAGE_NAME;

	/* -- -- -- */

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

		_tabbedWidgets = new QTabWidget();
		vBox->addWidget(_tabbedWidgets);
#if DEBUGGING_ON
		LOG(DEBUG2) << "allocated QTabWidget";
#endif
		_tabbedWidgets->setObjectName("Tabs");

		_daemonAndServiceTab = new DaemonAndServiceTab(_pDBus, "DaemonAndService");
		_tabbedWidgets->addTab(_daemonAndServiceTab, tr("Daemon and Service"));
		_daemonAndServiceTab->buildTab();

		_deviceControlTab = new DeviceControlTab(_pDBus, "DeviceControl");
		_tabbedWidgets->addTab(_deviceControlTab, tr("Device"));
		_deviceControlTab->buildTab();

		_backlightColorTab = new BacklightColorTab(_pDBus, "BacklightColor");
		_tabbedWidgets->addTab(_backlightColorTab, tr("Backlight Color"));
		_backlightColorTab->buildTab();

		_LCDPluginsTab = new LCDPluginsTab(_pDBus, "LCDPlugins");
		_tabbedWidgets->addTab(_LCDPluginsTab, tr("LCD Screen Plugins"));
		_LCDPluginsTab->buildTab();

		//_tabbedWidgets->addTab(new QWidget(), tr("Macros"));
#if DEBUGGING_ON
		LOG(DEBUG3) << "allocated 4 tabs";
#endif

		this->setTabEnabled("DaemonAndService", true);
		this->setTabEnabled("DeviceControl", true);
		this->setTabEnabled("BacklightColor", false);
		this->setTabEnabled("LCDPlugins", false);

		/* -- -- -- */

		QMenu* fileMenu = this->menuBar()->addMenu("&File");
		QAction* quit = new QAction("&Quit", this);
		fileMenu->addAction(quit);

		QMenu* helpMenu = this->menuBar()->addMenu("&Help");
		QAction* about = new QAction("&About", this);
		helpMenu->addAction(about);

		connect(quit, &QAction::triggered, qApp, QApplication::quit);
		connect(about, &QAction::triggered, this, &MainWindow::aboutDialog);

#if DEBUGGING_ON
		LOG(DEBUG2) << "built Qt menu";
#endif

		/* -- -- -- */
		/* initializing timer */
		QTimer* timer = new QTimer(this);

		connect(timer, &QTimer::timeout, this, &MainWindow::checkDBusMessages);
		timer->start(100);

#if DEBUGGING_ON
		LOG(DEBUG2) << "Qt timer started";
#endif
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		throw GLogiKBadAlloc("Qt bad alloc :(");
	}

	/* -- -- -- */

	try {
		this->resetInterface(); /* try 1 */
	}
	catch (const GLogiKExcept & e) {
		if(! _serviceStartRequest) {
			/* should not happen since the desktop service is assumed stopped
			 * until sucessful daemonAndServiceTab::updateTab() call */
			throw;
		}

		std::string status("desktop service seems not started, request");
		try {
			/* asking the launcher for the desktop service restart */
			_pDBus->initializeBroadcastSignal(
				NSGKDBus::BusConnection::GKDBUS_SESSION,
				GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
				GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
				"RestartRequest"
			);
			_pDBus->sendBroadcastSignal();

			status += " sent to launcher";
			LOG(WARNING) << status;
		}
		catch (const GKDBusMessageWrongBuild & e) {
			_pDBus->abandonBroadcastSignal();
			status += " to launcher failed";
			LOG(ERROR) << status << " - " << e.what();
			throw GLogiKExcept("Service RestartRequest failed");
		}

		/* sleeping for 2 seconds before retrying */
		std::this_thread::sleep_for(std::chrono::seconds(2));

		this->resetInterface(); /* try 2 */
	}

	/* -- -- -- */

	/* after here, don't throw if ::resetInterface() fails */
	_GUIResetThrow = false;

	/* -- -- -- */

	/* initializing Qt signals */
	connect(_devicesComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updateInterface);
	connect( _backlightColorTab->getApplyButton(), &QPushButton::clicked,
			 std::bind(&MainWindow::saveFile, this, TabApplyButton::TAB_BACKLIGHT) );
	connect(     _LCDPluginsTab->getApplyButton(), &QPushButton::clicked,
			 std::bind(&MainWindow::saveFile, this, TabApplyButton::TAB_LCD_PLUGINS) );

	/* initializing GKDBus signals */
	_pDBus->NSGKDBus::EventGKDBusCallback<VoidToVoid>::exposeSignal(
		NSGKDBus::BusConnection::GKDBUS_SESSION,
		GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		"DevicesUpdated",
		{},
		std::bind(&MainWindow::resetInterface, this)
	);

	_pDBus->NSGKDBus::EventGKDBusCallback<StringToVoid>::exposeSignal(
		NSGKDBus::BusConnection::GKDBUS_SESSION,
		GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		"DeviceConfigurationSaved",
		{ {"s", "device_id", "in", "device ID"}, },
		std::bind(&MainWindow::configurationFileUpdated, this, std::placeholders::_1)
	);
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

void MainWindow::configurationFileUpdated(const std::string & devID)
{
	if( _devID != devID)
		return;

	{
		LOG(WARNING) << "configuration file updated since last read for device : " << devID;
	}

	QMessageBox msgBox;

	QString icon(DATA_DIR);
	icon += "/icons/hicolor/48x48/apps/GLogiK.png";
	msgBox.setWindowIcon(QIcon(icon));

	msgBox.setWindowTitle("Configuration file modified since last read.");
	msgBox.setIcon(QMessageBox::Warning);
	msgBox.setText("<b>Configuration file modified since last read.</b>");

	QString msg("The configuration file for device ");
	msg += devID.c_str();
	msg += " has been modified since it was last read. It must be reloaded. Unsaved changes are lost.";

	msgBox.setInformativeText(msg);

	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setDefaultButton(QMessageBox::Ok);

	int ret = msgBox.exec();

#if DEBUGGING_ON
	LOG(DEBUG1) << "got ret value from messageBox : " << ret;
#endif
	if(ret == QMessageBox::Ok) {
		this->updateInterface( _devicesComboBox->currentIndex() );
	}
}

void MainWindow::aboutDialog(void)
{
	try {
		AboutDialog* about = new AboutDialog(this);
		about->buildDialog();
		about->setModal(true);
		about->setAttribute(Qt::WA_DeleteOnClose);
		about->setFixedSize(400, 300);
		about->setWindowTitle("About GKcQt5");
		about->open();
	}
	catch (const std::bad_alloc& e) {
		LOG(ERROR) << "catched bad_alloc : " << e.what();
	}
}

void MainWindow::saveFile(const TabApplyButton tab)
{
	bool dosave = false;

	if( tab == TabApplyButton::TAB_BACKLIGHT ) {
		int r, g, b = 255;
		_backlightColorTab->getAndSetNewColor().getRgb(&r, &g, &b);
		/* setting color */
		_openedConfigurationFile.setRGBBytes(r, g, b);
		dosave = true;
#if DEBUGGING_ON
		LOG(DEBUG3) << "backlight color updated";
#endif
	}
	else if( tab == TabApplyButton::TAB_LCD_PLUGINS ) {
		auto maskID = toEnumType(LCDPluginsMask::GK_LCD_PLUGINS_MASK_1);
		/* setting new LCD Plugins mask */
		_openedConfigurationFile.setLCDPluginsMask(maskID, _LCDPluginsTab->getAndSetNewLCDPluginsMask());
		dosave = true;
#if DEBUGGING_ON
		LOG(DEBUG3) << "LCD plugins mask updated";
#endif
	}

	if(dosave) {
#if DEBUGGING_ON
		LOG(DEBUG2) << "saving file";
#endif

		DeviceConfigurationFile::save(_configurationFilePath.string(), _openedConfigurationFile);

		QString msg("Configuration file saved");
		this->statusBar()->showMessage(msg, _statusBarTimeout);
	}
	else  {
		QString msg("Error updating configuration file, nothing changed");
		this->statusBar()->showMessage(msg, _statusBarTimeout);
		LOG(WARNING) << msg.toStdString();
	}
}

void MainWindow::updateInterface(int index)
{
	/* don't update interface on combo clear() */
	if(index == -1) {
		return;
	}
#if DEBUGGING_ON
	LOG(DEBUG2) << "updating interface with index : " << index;
#endif

	try {
		if(index == 0) {
			_devID.clear();
			_deviceControlTab->disableAndHide();
			this->setTabEnabled("BacklightColor", false);
			this->setTabEnabled("LCDPlugins", false);
			this->statusBar()->showMessage("Selected device : none", _statusBarTimeout);
		}
		else {
			_devID = _devicesComboBox->currentText().split(" ").at(0).toStdString();
#if DEBUGGING_ON
			LOG(DEBUG) << "updating tabs";
#endif

			try {
				const Device & device = _devices.at(_devID);
				const bool status = (device.getStatus() == "started");

				_deviceControlTab->updateTab(_devID, status);

				if(status) {
					_configurationFilePath = _configurationRootDirectory;
					_configurationFilePath /= device.getVendor();
					_configurationFilePath /= device.getConfigFilePath();

					_openedConfigurationFile.setVendor(device.getVendor());
					_openedConfigurationFile.setModel(device.getModel());
					_openedConfigurationFile.setConfigFilePath(device.getConfigFilePath());
					DeviceConfigurationFile::load(_configurationFilePath.string(), _openedConfigurationFile);

					_backlightColorTab->updateTab(_openedConfigurationFile);

					_LCDPluginsTab->updateTab(_devID, _openedConfigurationFile);
				}
				this->setTabEnabled("BacklightColor", status);
				this->setTabEnabled("LCDPlugins", status);
			}
			catch (const std::out_of_range& oor) {
				std::string error("device not found in container : ");
				error += _devID;
				throw GLogiKExcept(error);
			}

			QString msg("Selected device : ");
			msg += _devicesComboBox->currentText();
			this->statusBar()->showMessage(msg, _statusBarTimeout);
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

			_devices = _pDBus->getNextDevicesMapArgument();

			QString msg("Detected ");
			msg += QString::number(_devices.size());
			msg += " device(s)";

			LOG(INFO) << msg.toStdString();
			this->statusBar()->showMessage(msg, _statusBarTimeout);
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
		/* clear() set current index to -1 */
		_devicesComboBox->clear();

		try {
			_daemonAndServiceTab->updateTab();
		}
		catch (const GLogiKExcept & e) {
			/* used only over initialization */
			_serviceStartRequest = (_daemonAndServiceTab->isServiceStarted() == false);

			/* additems() set current index to 0
			 * ::updateInterface() will be called with index 0 */
			_devicesComboBox->addItems({""});
			throw;
		}

		/* don't try to update devices list if the service
		 * is not registered against the daemon */
		if( ! _daemonAndServiceTab->isServiceRegistered() )
			return;

		this->updateDevicesList();

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

		_deviceControlTab->disableButtons();
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << "error resetting interface : " << e.what();
		if(_GUIResetThrow) {
			throw GLogiKExcept("interface reset failure");
		}
	}
}

QWidget* MainWindow::getTabbedWidget(const std::string & name)
{
	QString n(name.c_str());
	QWidget* pTab = _tabbedWidgets->findChild<QWidget *>(n);
	if(pTab == 0) {
		LOG(ERROR) << "tab not found : " << name;
		throw GLogiKExcept("tab not found in tabWidget");
	}
	return pTab;
}

void MainWindow::setTabEnabled(const std::string & name, const bool status)
{
	try {
		QWidget* pTab = this->getTabbedWidget(name);
		int index = _tabbedWidgets->indexOf(pTab);
		if(index == -1) {
			LOG(ERROR) << "index not found : " << name;
			throw GLogiKExcept("tab index not found");
		}

		_tabbedWidgets->setTabEnabled(index, status);
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << "error setting TabEnabled property : " << e.what();
		throw;
	}
}

void MainWindow::checkDBusMessages(void)
{
	_pDBus->checkForMessages();
}

} // namespace GLogiK

