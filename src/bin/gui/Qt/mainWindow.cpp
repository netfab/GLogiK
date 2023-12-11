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

#include <csignal>

#include <syslog.h>

#include <new>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <functional>

#include <QtGlobal>
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
#include <QFile>

#include <config.h>

#include <boost/program_options.hpp>

#include "lib/shared/glogik.hpp"
#include "lib/shared/deviceConfigurationFile.hpp"
#include "lib/utils/utils.hpp"

#include "include/enums.hpp"

#include "AboutDialog/AboutDialog.hpp"
#include "mainWindow.hpp"

namespace po = boost::program_options;

namespace GLogiK
{

using namespace NSGKUtils;

MainWindow::MainWindow(QWidget *parent)
	:	QMainWindow(parent),
		_pDBus(nullptr),
		_devicesComboBox(nullptr),
		_tabbedWidgets(nullptr),
		_daemonAndServiceTab(nullptr),
		_deviceControlTab(nullptr),
		_backlightColorTab(nullptr),
		_LCDPluginsTab(nullptr),
		_statusBarTimeout(3000),
		_pid(0),
		_ignoreNextSignal(false)
{
	openlog("GKcQt5", LOG_PID|LOG_CONS, LOG_USER);
}

MainWindow::~MainWindow()
{
	closelog();
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

void MainWindow::handleSignal(int signum)
{
	LOG(info) << process::getSignalHandlingDesc(signum, " --> bye bye");
	QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
}

void MainWindow::init(const int& argc, char *argv[])
{
	GK_LOG_FUNC

	// initialize logging
	try {
		/* boost::po may throw */
		this->parseCommandLine(argc, argv);

#if DEBUGGING_ON
		if(GKLogging::GKDebug) {
			GKLogging::initDebugFile("GKcQt5", fs::owner_read|fs::owner_write|fs::group_read);
		}
#endif
		GKLogging::initConsoleLog();
	}
	catch (const std::exception & e) {
		syslog(LOG_ERR, "%s", e.what());
		throw GLogiKExcept("logging initialization failure");
	}

	/* -- -- -- */
	/* -- -- -- */
	/* -- -- -- */

	LOG(info) << "Starting GKcQt5 vers. " << VERSION;

	std::signal(SIGINT, MainWindow::handleSignal);
	std::signal(SIGTERM, MainWindow::handleSignal);
	std::signal(SIGHUP, MainWindow::handleSignal);

	try {
		_pDBus = new NSGKDBus::GKDBus(GLOGIK_DESKTOP_QT5_DBUS_ROOT_NODE, GLOGIK_DESKTOP_QT5_DBUS_ROOT_NODE_PATH);
		_pDBus->init();
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
	GK_LOG_FUNC

	QVBoxLayout* vBox = nullptr;

	try {
		{ /* loading stylesheet */
			QString stylepath(QT_DATA_DIR); stylepath += "/qss/stylesheet.qss";

			QFile qssfile(stylepath);
			qssfile.open(QFile::ReadOnly);

			QString styleSheet = QLatin1String(qssfile.readAll());
			qApp->setStyleSheet(styleSheet);
		}

		QFrame *frame = new QFrame(this);
		this->setCentralWidget(frame);

		vBox = new QVBoxLayout(frame);
		GKLog(trace, "allocated QVBoxLayout")

		/* -- -- -- */

		_devicesComboBox = new QComboBox();
		GKLog(trace, "allocated QComboBox")

		_devicesComboBox->setObjectName("devicesList");

		vBox->addWidget(_devicesComboBox);

		/* -- -- -- */

		QFrame* line = new QFrame();
		GKLog(trace, "allocated QFrame")

		line->setFrameShape(QFrame::HLine);
		line->setFrameShadow(QFrame::Sunken);

		vBox->addWidget(line);

		/* -- -- -- */

		_tabbedWidgets = new QTabWidget();
		GKLog(trace, "allocated QTabWidget")

		_tabbedWidgets->setObjectName("Tabs");

		vBox->addWidget(_tabbedWidgets);

		/* -- -- -- */

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

		_GKeysTab = new GKeysTab(_pDBus, "GKeys");
		_tabbedWidgets->addTab(_GKeysTab, tr("G-Keys"));
		_GKeysTab->buildTab();

		GKLog(trace, "allocated 5 tabs")

		this->setTabEnabled("DaemonAndService", true);
		this->setTabEnabled("DeviceControl", false);
		this->setTabEnabled("BacklightColor", false);
		this->setTabEnabled("LCDPlugins", false);
		this->setTabEnabled("GKeys", false);

		this->setCurrentTab("DaemonAndService");

		/* -- -- -- */

		/* required by aboutDialog */
		try {
			this->getExecutablesDependenciesMap();
		}
		catch (const GLogiKExcept & e) {
			_daemonAndServiceTab->sendServiceStartRequest();
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			this->getExecutablesDependenciesMap();
		}

		QMenu* fileMenu = this->menuBar()->addMenu("&File");
		QAction* quit = new QAction("&Quit", this);
		fileMenu->addAction(quit);

		QMenu* helpMenu = this->menuBar()->addMenu("&Help");
		QAction* about = new QAction("&About", this);
		helpMenu->addAction(about);

		QObject::connect(quit, &QAction::triggered, qApp, QApplication::quit);
		QObject::connect(about, &QAction::triggered, this, &MainWindow::aboutDialog);

		GKLog(trace, "built Qt menu")

		/* -- -- -- */
		/* initializing timer */
		QTimer* timer = new QTimer(this);

		QObject::connect(timer, &QTimer::timeout, this, &MainWindow::checkDBusMessages);
		timer->start(100);

		GKLog(trace, "Qt timer started")
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		throw GLogiKBadAlloc("Qt bad alloc :(");
	}

	/* -- -- -- */

	this->resetInterface();

	/* -- -- -- */

	/* initializing Qt signals */
	QObject::connect(_devicesComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updateInterface);
	QObject::connect( _backlightColorTab->getApplyButton(), &QPushButton::clicked,
			 std::bind(&MainWindow::saveConfigurationFileAndUpdateInterface, this, TabApplyButton::TAB_BACKLIGHT) );
	QObject::connect( _LCDPluginsTab->getApplyButton(), &QPushButton::clicked,
			 std::bind(&MainWindow::saveConfigurationFileAndUpdateInterface, this, TabApplyButton::TAB_LCD_PLUGINS) );
	QObject::connect( _GKeysTab->getApplyButton(), &QPushButton::clicked,
			 std::bind(&MainWindow::saveConfigurationFileAndUpdateInterface, this, TabApplyButton::TAB_GKEYS) );

	GKLog(trace, "Qt signals connected to slots")

	/* initializing GKDBus signals */
	_pDBus->NSGKDBus::Callback<SIGv2v>::exposeSignal(
		_sessionBus,
		GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		"DevicesUpdated",
		{},
		std::bind(&MainWindow::resetInterface, this)
	);

	_pDBus->NSGKDBus::Callback<SIGs2v>::exposeSignal(
		_sessionBus,
		GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
		"DeviceConfigurationSaved",
		{ {"s", "device_id", "in", "device ID"}, },
		std::bind(&MainWindow::configurationFileUpdated, this, std::placeholders::_1)
	);

	QObject::connect(qApp, &QCoreApplication::aboutToQuit, this, &MainWindow::aboutToQuit);
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

void MainWindow::parseCommandLine(const int& argc, char *argv[])
{
	GK_LOG_FUNC

	po::options_description desc("Allowed options");

#if DEBUGGING_ON
	desc.add_options()
		("debug,D", po::bool_switch()->default_value(false), "run in debug mode")
	;
#endif

	po::variables_map vm;

	po::store(po::parse_command_line(argc, argv, desc), vm);

	po::notify(vm);

#if DEBUGGING_ON
	bool debug = vm.count("debug") ? vm["debug"].as<bool>() : false;

	if( debug ) {
		GKLogging::GKDebug = true;
	}
#endif
}

void MainWindow::aboutToQuit(void)
{
	GK_LOG_FUNC

	_pDBus->removeSignalsInterface(
		_sessionBus,
		GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT,
		GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE);

	_pDBus->exit();

	delete _pDBus; _pDBus = nullptr;

	LOG(info) << "GKcQt5 MainWindow process exiting, bye !";
}

void MainWindow::configurationFileUpdated(const std::string & devID)
{
	GK_LOG_FUNC

	if( _ignoreNextSignal ) {
		GKLog(trace, "DeviceConfigurationSaved signal ignored")

		_ignoreNextSignal = false;
		return;
	}

	if(_devID != devID)
		return;

	{
		LOG(warning) << "configuration file updated since last read for device : " << devID;
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
	GKLog2(trace, "got ret value from messageBox : ", ret)

	if(ret == QMessageBox::Ok) {
		this->updateInterface( _devicesComboBox->currentIndex() );
	}
}

void MainWindow::getExecutablesDependenciesMap(void)
{
	GK_LOG_FUNC

	GKLog(trace, "getting executables dependencies map")

	const std::string remoteMethod("GetExecutablesDependenciesMap");
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
			_DepsMap = _pDBus->getNextGKDepsMapArgument();

			_DepsMap[GKBinary::GK_GUI_QT] =
				{ /* qVersion() from <QtGlobal> */
					{"Qt5", GK_DEP_QT5_VERSION_STRING, qVersion()},
				};
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

void MainWindow::aboutDialog(void)
{
	GK_LOG_FUNC

	try {
		AboutDialog* about = new AboutDialog(this);
		about->buildDialog(_pDepsMap);
		about->setModal(true);
		about->setAttribute(Qt::WA_DeleteOnClose);
		about->setFixedSize(560, 300);
		about->setWindowTitle("About GKcQt5");
		about->open();
	}
	catch (const std::bad_alloc& e) {
		LOG(error) << "catched bad_alloc : " << e.what();
	}
}

void MainWindow::saveConfigurationFile(const TabApplyButton tab)
{
	GK_LOG_FUNC

	bool dosave = false;

	if( tab == TabApplyButton::TAB_BACKLIGHT ) {
		int r, g, b = 255;
		_backlightColorTab->getAndSetNewColor().getRgb(&r, &g, &b);
		/* setting color */
		_openedConfigurationFile.setRGBBytes(r, g, b);
		dosave = true;

		GKLog(trace, "backlight color updated")
	}
	else if( tab == TabApplyButton::TAB_LCD_PLUGINS ) {
		auto maskID = toEnumType(LCDPluginsMask::GK_LCD_PLUGINS_MASK_1);
		/* setting new LCD Plugins mask */
		_openedConfigurationFile.setLCDPluginsMask(maskID, _LCDPluginsTab->getAndSetNewLCDPluginsMask());
		dosave = true;

		GKLog(trace, "LCD plugins mask updated")
	}
	else if( tab == TabApplyButton::TAB_GKEYS ) {
		MKeysID bankID;
		GKeysID keyID;
		GKeyEventType eventType;
		std::string eventCommand;
		try {
			_GKeysTab->getGKeyEventParams(bankID, keyID, eventType, eventCommand);

			GKLog4(trace, "MBank: ", bankID, "GKey: ", getGKeyName(keyID))
			GKLog4(trace, "eventType: ", GKeysTab::getEventTypeString(eventType), "command: ", eventCommand)

			banksMap_type & banks = _openedConfigurationFile.getBanks();
			mBank_type & bank = banks.at(bankID);
			GKeysEvent & event = bank.at(keyID);

			event.setCommand(eventCommand);
			event.setEventType(eventType);

			dosave = true;

			GKLog(trace, "GKeys updated")
		}
		catch (const GLogiKExcept & e) {
			LOG(error) << "can't get GKeyEvent parameters " << e.what();
		}
		catch (const std::out_of_range& oor) {
			LOG(error) << "out of range detected: " << oor.what();
		}
	}

	if(dosave) {
		GKLog(trace, "saving file")

		/* desktop service will detect that configuration file was modified,
		 * and will send us a signal. Ignore it. */
		_ignoreNextSignal = true;
		DeviceConfigurationFile::save(_configurationFilePath.string(), _openedConfigurationFile);

		QString msg("Configuration file saved");
		this->statusBar()->showMessage(msg, _statusBarTimeout);
	}
	else  {
		QString msg("Error updating configuration file, nothing changed");
		this->statusBar()->showMessage(msg, _statusBarTimeout);
		LOG(warning) << msg.toStdString();
	}
}

void MainWindow::updateInterface(int index)
{
	GK_LOG_FUNC

	/* update interface with index 0 on combobox clear()
	 * this happens at the very beginning of ::resetInterface() */
	if(index == -1) {
		index = 0;
	}

	GKLog2(trace, "updating interface with index : ", index)

	try {
		if(index == 0) {
			_devID.clear();
			_deviceControlTab->disableAndHide();
			this->setTabEnabled("BacklightColor", false);
			this->setTabEnabled("LCDPlugins", false);
			this->setTabEnabled("GKeys", false);
			this->statusBar()->showMessage("Selected device : none", _statusBarTimeout);
		}
		else {
			_devID = _devicesComboBox->currentText().split(" ").at(0).toStdString();

			GKLog(trace, "updating tabs")

			try {
				const DeviceID & device = _devices.at(_devID);
				const bool status = (device.getStatus() == "started");

				if(status) {
					_configurationFilePath = _configurationRootDirectory;
					_configurationFilePath /= device.getVendor();
					_configurationFilePath /= device.getConfigFilePath();

					_openedConfigurationFile.setVendor(device.getVendor());
					_openedConfigurationFile.setProduct(device.getProduct());
					_openedConfigurationFile.setName(device.getName());
					_openedConfigurationFile.setConfigFilePath(device.getConfigFilePath());
					DeviceConfigurationFile::load(_configurationFilePath.string(), _openedConfigurationFile);

				}

				/* device status always required by deviceControlTab */
				_openedConfigurationFile.setStatus(device.getStatus());

				/* updating and enabling/disabling tabs */
				_deviceControlTab->updateTab(_openedConfigurationFile, _devID);

				if(status) {
					_backlightColorTab->updateTab(_openedConfigurationFile, _devID);
					_LCDPluginsTab->updateTab(_openedConfigurationFile, _devID);
					_GKeysTab->updateTab(_openedConfigurationFile, _devID);
				}

				this->setTabEnabled("BacklightColor", status);
				this->setTabEnabled("LCDPlugins", status);
				this->setTabEnabled("GKeys", status);
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
		LOG(error) << "error updating interface : " << e.what();
	}
}

void MainWindow::saveConfigurationFileAndUpdateInterface(const TabApplyButton tab)
{
	this->saveConfigurationFile(tab);
	/* update interface to reload the configuration file */
	this->updateInterface( _devicesComboBox->currentIndex() );
}

void MainWindow::updateDevicesList(void)
{
	GK_LOG_FUNC

	GKLog(trace, "updating devices list")

	_devices.clear();

	const std::string remoteMethod("GetDevicesList");
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

			_devices = _pDBus->getNextDevicesMapArgument();

			QString msg("Detected ");
			msg += QString::number(_devices.size());
			msg += " device(s)";

			LOG(info) << msg.toStdString();
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
	GK_LOG_FUNC

	GKLog(trace, "resetting interface")

	try {
		this->setCurrentTab("DaemonAndService");

		_devicesComboBox->setEnabled(false);
		/* clear() set current index to -1 */
		_devicesComboBox->clear();

		this->setTabEnabled("DeviceControl", false);

		_daemonAndServiceTab->updateTab(); /* may throw */

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
				item += devicePair.second.getProduct();
				item += " ";
				item += devicePair.second.getName();
				std::replace( item.begin(), item.end(), '_', ' ');
				items << item.c_str();
			}
			/* additems() set current index to 0 */
			_devicesComboBox->addItems(items);
			_devicesComboBox->setEnabled(true);
		}

		_deviceControlTab->disableButtons();
		this->setTabEnabled("DeviceControl", true);
	}
	catch (const GLogiKExcept & e) {
		LOG(error) << "error resetting interface : " << e.what();
	}
}

QWidget* MainWindow::getTabbedWidget(const std::string & name)
{
	GK_LOG_FUNC

	QString n(name.c_str());
	QWidget* pTab = _tabbedWidgets->findChild<QWidget *>(n);
	if(pTab == 0) {
		LOG(error) << "tab not found : " << name;
		throw GLogiKExcept("tab not found in tabWidget");
	}
	return pTab;
}

void MainWindow::setTabEnabled(const std::string & name, const bool status)
{
	GK_LOG_FUNC

	try {
		QWidget* pTab = this->getTabbedWidget(name);
		int index = _tabbedWidgets->indexOf(pTab);
		if(index == -1) {
			LOG(error) << "index not found : " << name;
			throw GLogiKExcept("tab index not found");
		}

		_tabbedWidgets->setTabEnabled(index, status);
	}
	catch (const GLogiKExcept & e) {
		LOG(error) << "error setting TabEnabled property : " << e.what();
		throw;
	}
}

void MainWindow::setCurrentTab(const std::string & name)
{
	GK_LOG_FUNC

	try {
		QWidget* pTab = this->getTabbedWidget(name);
		int index = _tabbedWidgets->indexOf(pTab);
		if(index == -1) {
			LOG(error) << "index not found : " << name;
			throw GLogiKExcept("tab index not found");
		}

		_tabbedWidgets->setCurrentIndex(index);
	}
	catch (const GLogiKExcept & e) {
		LOG(error) << "error setting currentIndex : " << e.what();
		throw;
	}
}

void MainWindow::checkDBusMessages(void)
{
	_pDBus->checkForMessages();
}

} // namespace GLogiK

