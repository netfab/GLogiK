/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2017  Fabrice Delliaux <netbox253@gmail.com>
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

#include <stdexcept>

#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <config.h>

#include "lib/utils/utils.h"

#include "serviceDBusHandler.h"

namespace fs = boost::filesystem;

namespace GLogiK
{

/* --
 * DBus Stuff
 * --
 * DCM - Daemon ClientsManager - to contact the ClientsManager
 * SMH - System Message Handler - to declare signals we interested in from the daemon
 * DDM - Daemon DevicesManager - to contact the DevicesManager
 *
 */

ServiceDBusHandler::ServiceDBusHandler() : warn_count_(0), DBus(nullptr), buffer_("", std::ios_base::app) {
	this->cfgfile_fullpath_ = XDGUserDirs::getConfigDirectory();
	this->cfgfile_fullpath_ += "/";
	this->cfgfile_fullpath_ += PACKAGE_NAME;

	bool dir_created = false;

	try {
		fs::path path(this->cfgfile_fullpath_);
		dir_created = create_directory(path);
		fs::permissions(path, fs::owner_all);
	}
	catch (const fs::filesystem_error & e) {
		this->buffer_.str("configuration directory creation or set permissions failure : ");
		this->buffer_ << this->cfgfile_fullpath_ << " : " << e.what();
		throw GLogiKExcept(this->buffer_.str());
	}

	if( ! dir_created ) {
		LOG(DEBUG) << "configuration directory not created because it seems already exists";
	}
	else {
		LOG(DEBUG) << "configuration directory created";
	}

	this->cfgfile_fullpath_ += "/filename";

	try {
		this->DBus = new GKDBus(GLOGIK_DESKTOP_SERVICE_DBUS_ROOT_NODE);
		this->DBus->connectToSystemBus(GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME);

		this->setCurrentSessionObjectPath();
		this->session_state_ = this->getCurrentSessionState();

		/* telling the daemon we are alive */
		this->DBus->initializeRemoteMethodCall(BusConnection::GKDBUS_SYSTEM, GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			this->DBus_DCM_object_path_, this->DBus_DCM_interface_, "RegisterClient");
		this->DBus->appendToRemoteMethodCall(this->current_session_);
		this->DBus->sendRemoteMethodCall();

		this->DBus->waitForRemoteMethodCallReply();
		const bool ret = this->DBus->getNextBooleanArgument();
		if( ret ) {
			const char * success = "successfully registered with daemon";
			LOG(DEBUG2) << success;
			GK_STAT << success << "\n";
		}
		else {
			const char * failure = "failed to register with daemon : false";
			LOG(ERROR) << failure;
			GK_ERR << failure << "\n";
			throw GLogiKExcept(failure);
		}

		/* want to be warned by the daemon about those signals */
		this->DBus->addSignal_VoidToVoid_Callback(BusConnection::GKDBUS_SYSTEM,
			this->DBus_SMH_object_, this->DBus_SMH_interface_, "SomethingChanged",
			{}, // FIXME
			std::bind(&ServiceDBusHandler::somethingChanged, this) );

		/* set GKDBus pointer */
		this->devices.setDBus(this->DBus);
	}
	catch ( const GLogiKExcept & e ) {
		delete this->DBus;
		this->DBus = nullptr;
		throw;
	}
}

ServiceDBusHandler::~ServiceDBusHandler() {
	try {
		/* telling the daemon we're killing ourself */
		this->DBus->initializeRemoteMethodCall(BusConnection::GKDBUS_SYSTEM, GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			this->DBus_DCM_object_path_, this->DBus_DCM_interface_, "UnregisterClient");
		this->DBus->appendToRemoteMethodCall(this->current_session_);
		this->DBus->sendRemoteMethodCall();

		this->DBus->waitForRemoteMethodCallReply();
		const bool ret = this->DBus->getNextBooleanArgument();
		if( ret ) {
			const char * success = "successfully unregistered with daemon";
			LOG(DEBUG2) << success;
			GK_STAT << success << "\n";
		}
		else {
			const char * failure = "failed to unregister with daemon : false";
			LOG(ERROR) << failure;
			GK_ERR << failure << "\n";
		}
	}
	catch ( const GLogiKExcept & e ) {
		std::string err("failure to unregister with daemon : ");
		err += e.what();
		LOG(ERROR) << err;
		GK_ERR << err << "\n";
	}

	this->saveDevicesProperties();

	delete this->DBus;
	this->DBus = nullptr;
}

void ServiceDBusHandler::saveDevicesProperties(void) {
	try {
		std::ofstream ofs;
		ofs.exceptions(std::ofstream::failbit|std::ofstream::badbit);
		ofs.open(this->cfgfile_fullpath_, std::ofstream::out|std::ofstream::trunc);

		fs::path path(this->cfgfile_fullpath_);
		fs::permissions(path, fs::owner_read|fs::owner_write|fs::group_read|fs::others_read);

		LOG(DEBUG) << "configuration file successfully opened for writing";

		boost::archive::text_oarchive output_archive(ofs);
		output_archive << this->devices;
	}
	catch (const std::ofstream::failure & e) {
		this->buffer_.str("fail to open configuration file : ");
		this->buffer_ << this->cfgfile_fullpath_ << " : " << e.what();
		LOG(ERROR) << this->buffer_.str();
		GK_ERR << this->buffer_.str() << "\n";
	}
	catch (const fs::filesystem_error & e) {
		this->buffer_.str("set permissions failure on configuration file : ");
		this->buffer_ << this->cfgfile_fullpath_ << " : " << e.what();
		LOG(ERROR) << this->buffer_.str();
		GK_ERR << this->buffer_.str() << "\n";
	}
	/*
	 * catch std::ios_base::failure on buggy compilers
	 * should be fixed with gcc >= 7.0
	 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145
	 */
	catch( const std::exception & e ) {
		this->buffer_.str("(buggy exception) fail to open configuration file : ");
		this->buffer_ << e.what();
		LOG(ERROR) << this->buffer_.str();
		GK_ERR << this->buffer_.str() << "\n";
	}
}

void ServiceDBusHandler::setCurrentSessionObjectPath(void) {
	// TODO logind support
	try {
		/* getting consolekit current session */
		this->DBus->initializeRemoteMethodCall(BusConnection::GKDBUS_SYSTEM, "org.freedesktop.ConsoleKit",
			"/org/freedesktop/ConsoleKit/Manager", "org.freedesktop.ConsoleKit.Manager", "GetCurrentSession");
		this->DBus->sendRemoteMethodCall();

		this->DBus->waitForRemoteMethodCallReply();
		this->current_session_ = this->DBus->getNextStringArgument();
		LOG(DEBUG1) << "current session : " << this->current_session_;
	}
	catch ( const GLogiKExcept & e ) {
		std::string err("unable to get current session path from session manager");
		LOG(ERROR) << err;
		GK_ERR << err << "\n";
		throw;
	}
}

void ServiceDBusHandler::warnOrThrows(const std::string & warn) {
	LOG(WARNING) << warn;
	GK_WARN << warn << "\n";
	if(this->warn_count_ >= MAXIMUM_WARNINGS_BEFORE_FATAL_ERROR) {
		const std::string last = "maximum warning count reached, this is fatal";
		LOG(ERROR) << last;
		GK_ERR << last << "\n";
		throw GLogiKExcept(last);
	}
	this->warn_count_++;
}

/*
 * is the session in active, online or closing state ?
 */
const std::string ServiceDBusHandler::getCurrentSessionState(const bool logoff) {
	// TODO logind support
	try {
		this->DBus->initializeRemoteMethodCall(BusConnection::GKDBUS_SYSTEM, "org.freedesktop.ConsoleKit",
			this->current_session_.c_str(), "org.freedesktop.ConsoleKit.Session", "GetSessionState", logoff);
		this->DBus->sendRemoteMethodCall();

		this->DBus->waitForRemoteMethodCallReply();
		const std::string ret_string = this->DBus->getNextStringArgument();
#if DEBUGGING_ON
		LOG(DEBUG5) << "current session state : " << ret_string;
#endif
		return ret_string;
	}
	catch ( const GLogiKExcept & e ) {
		std::string warn(__func__);
		warn += " failure : ";
		warn += e.what();
		this->warnOrThrows(warn);
	}

	return this->session_state_;
}

void ServiceDBusHandler::reportChangedState(void) {
	try {
		this->DBus->initializeRemoteMethodCall(BusConnection::GKDBUS_SYSTEM, GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			this->DBus_DCM_object_path_, this->DBus_DCM_interface_, "UpdateClientState");
		this->DBus->appendToRemoteMethodCall(this->current_session_);
		this->DBus->appendToRemoteMethodCall(this->session_state_);
		this->DBus->sendRemoteMethodCall();

		this->DBus->waitForRemoteMethodCallReply();
		const bool ret = this->DBus->getNextBooleanArgument();
		if( ret ) {
			const char * success = "successfully reported changed state";
			LOG(DEBUG2) << success;
			GK_STAT << success << "\n";
		}
		else {
			const char * failure = "failed to report changed state : false";
			LOG(ERROR) << failure;
			GK_ERR << failure << "\n";
		}
	}
	catch ( const GLogiKExcept & e ) {
		std::string warn(__func__);
		warn += " failure : ";
		warn += e.what();
		this->warnOrThrows(warn);
	}
}

void ServiceDBusHandler::warnUnhandledSessionState(const std::string & state) {
	std::string warn = "unhandled session state : ";
	warn += state;
	this->warnOrThrows(warn);
}

void ServiceDBusHandler::updateSessionState(void) {
	/* if debug output is ON, force-disable it, else debug file
	 * will be spammed by the following DBus request debug output */
	const bool logoff = true;
	const std::string new_state = this->getCurrentSessionState(logoff);
	if(this->session_state_ == new_state) {
#if DEBUGGING_ON
		LOG(DEBUG5) << "session state did not changed";
#endif
		return;
	}

#if DEBUGGING_ON
	LOG(DEBUG1) << "current session state : " << this->session_state_;
#endif

	if( this->session_state_ == "active" ) {
		if(new_state == "online") {
		}
		else if(new_state == "closing") {
		}
		else {
			this->warnUnhandledSessionState(new_state);
			return;
		}
	}
	else if( this->session_state_ == "online" ) {
		if(new_state == "active") {
		}
		else if(new_state == "closing") {
		}
		else {
			this->warnUnhandledSessionState(new_state);
			return;
		}
	}
	else if( this->session_state_ == "closing" ) {
		if(new_state == "active") {
		}
		else if(new_state == "online") {
		}
		else {
			this->warnUnhandledSessionState(new_state);
			return;
		}
	}
	else {
		this->warnUnhandledSessionState(this->session_state_);
		return;
	}

#if DEBUGGING_ON
	LOG(DEBUG1) << "switching session state to : " << new_state;
#endif

	this->session_state_ = new_state;
	this->reportChangedState();
}

void ServiceDBusHandler::somethingChanged(void) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "it seems that something changed ! ";
#endif

	unsigned int num = 0;
	std::string device;

	/* check started devices */
	try {
		this->DBus->initializeRemoteMethodCall(BusConnection::GKDBUS_SYSTEM, GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			this->DBus_DDM_object_path_, this->DBus_DDM_interface_, "GetStartedDevices");
		this->DBus->sendRemoteMethodCall();

		this->DBus->waitForRemoteMethodCallReply();

		try {
			while( true ) {
				device = this->DBus->getNextStringArgument();
				num++;
				this->devices.checkStartedDevice(device);
			}
		}
		catch (const EmptyContainer & e) {
			LOG(DEBUG3) << "daemon says " << num << " devices started";
			// nothing to do here
		}
	}
	catch (const GLogiKExcept & e) {
		std::string warn(__func__);
		warn += " failure : ";
		warn += e.what();
		this->warnOrThrows(warn);
	}

	num = 0;

	/* check stopped devices */
	try {
		this->DBus->initializeRemoteMethodCall(BusConnection::GKDBUS_SYSTEM, GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME,
			this->DBus_DDM_object_path_, this->DBus_DDM_interface_, "GetStoppedDevices");
		this->DBus->sendRemoteMethodCall();

		this->DBus->waitForRemoteMethodCallReply();

		try {
			while( true ) {
				device = this->DBus->getNextStringArgument();
				num++;
				this->devices.checkStoppedDevice(device);
			}
		}
		catch (const EmptyContainer & e) {
			LOG(DEBUG3) << "daemon says " << num << " devices stopped";
			// nothing to do here
		}
	}
	catch (const GLogiKExcept & e) {
		std::string warn(__func__);
		warn += " failure : ";
		warn += e.what();
		this->warnOrThrows(warn);
	}
}

void ServiceDBusHandler::checkDBusMessages(void) {
	if( this->DBus->checkForNextMessage(BusConnection::GKDBUS_SYSTEM) ) {
		try {
			this->DBus->checkForSignalReceipt(BusConnection::GKDBUS_SYSTEM);
			this->DBus->freeMessage();
		}
		catch ( const GLogiKExcept & e ) {
			this->DBus->freeMessage();
			throw;
		}
	}
}

} // namespace GLogiK

