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

#include <algorithm>
#include <string>

#include "GKDBus.h"

#include "lib/utils/utils.h"

namespace GLogiK
{

GKDBus::GKDBus(const std::string & rootnode) : buffer_("", std::ios_base::app),
	message_(nullptr),
	pending_(nullptr),
	current_conn_(nullptr),
	session_conn_(nullptr),
	system_conn_(nullptr),
	reply_(nullptr),
	method_call_(nullptr),
	signal_(nullptr),
	error_reply_(nullptr)
{
	LOG(DEBUG1) << "dbus object initialization";
	dbus_error_init(&(this->error_));
	this->defineRootNode(rootnode);
}

GKDBus::~GKDBus()
{
	LOG(DEBUG1) << "dbus object destruction";
	if(this->session_conn_) {
		LOG(DEBUG) << "closing DBus Session connection";
		dbus_connection_unref(this->session_conn_);
	}
	if(this->system_conn_) {
		LOG(DEBUG) << "closing DBus System connection";
		dbus_connection_unref(this->system_conn_);
	}
}

/* -- */
/*
 * DBus connections
 */

void GKDBus::connectToSessionBus(const char* connection_name) {
	this->session_conn_ = dbus_bus_get(DBUS_BUS_SESSION, &this->error_);
	this->checkDBusError("DBus Session connection failure");
	LOG(DEBUG1) << "DBus Session connection opened";

	// TODO check name flags
	int ret = dbus_bus_request_name(this->session_conn_, connection_name,
		DBUS_NAME_FLAG_REPLACE_EXISTING|DBUS_NAME_FLAG_ALLOW_REPLACEMENT, &this->error_);
	this->checkDBusError("DBus Session request name failure");
	if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		throw GLogiKExcept("DBus Session request name failure : not owner");
	}

	LOG(DEBUG1) << "DBus Session requested connection name : " << connection_name;
}

void GKDBus::connectToSystemBus(const char* connection_name) {
	this->system_conn_ = dbus_bus_get(DBUS_BUS_SYSTEM, &this->error_);
	this->checkDBusError("DBus System connection failure");
	LOG(DEBUG1) << "DBus System connection opened";

	// TODO check name flags
	int ret = dbus_bus_request_name(this->system_conn_, connection_name,
		DBUS_NAME_FLAG_REPLACE_EXISTING|DBUS_NAME_FLAG_ALLOW_REPLACEMENT, &this->error_);
	this->checkDBusError("DBus System request name failure");

	// FIXME single instance application, or DBUS_NAME_FLAG_ALLOW_REPLACEMENT
	if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		throw GLogiKExcept("DBus System request name failure : not owner");
	}

	LOG(DEBUG1) << "DBus System requested connection name : " << connection_name;
}

/* -- */
/*
 * check message methods
 */

const bool GKDBus::checkForNextMessage(BusConnection current) {
	this->setCurrentConnection(current);
	dbus_connection_read_write(this->current_conn_, 0);
	this->message_ = dbus_connection_pop_message(this->current_conn_);
	return (this->message_ != nullptr);
}

void GKDBus::freeMessage(void) {
	if( this->message_ == nullptr ) {
		LOG(WARNING) << "trying to free null message";
		return;
	}
	LOG(DEBUG3) << "freeing message";
	dbus_message_unref(this->message_);
}

const bool GKDBus::checkMessageForSignalReceipt(const char* interface, const char* signal) {
	if(this->message_ == nullptr)
		return false;
	if( dbus_message_is_signal(this->message_, interface, signal) ) {
		LOG(DEBUG1) << "DBus " << signal << " signal receipted !";
		this->fillInArguments(this->message_);
		return true;
	}
	return false;
}

const bool GKDBus::checkMessageForMethodCall(const char* interface, const char* method) {
	if(this->message_ == nullptr)
		return false;
	if( dbus_message_is_method_call(this->message_, interface, method) ) {
		LOG(DEBUG1) << "DBus " << method << " method called !";
		this->fillInArguments(this->message_);
		return true;
	}
	return false;
}

/* -- */
/*
 *	Signal setup
 */

void GKDBus::addSignal_StringToBool_Callback(BusConnection current,
	const char* object, const char* interface, const char* eventName,
	std::vector<DBusMethodArgument> args, std::function<const bool(const std::string&)> callback)
{
	this->setCurrentConnection(current);
	this->addEvent_StringToBool_Callback(object, interface, eventName, args, callback, GKDBusEventType::GKDBUS_EVENT_SIGNAL);
	this->addSignalRuleMatch(interface, eventName);
}

void GKDBus::addSignal_VoidToVoid_Callback(BusConnection current,
	const char* object, const char* interface, const char* eventName,
	std::vector<DBusMethodArgument> args, std::function<void(void)> callback)
{
	this->setCurrentConnection(current);
	this->addEvent_VoidToVoid_Callback(object, interface, eventName, args, callback, GKDBusEventType::GKDBUS_EVENT_SIGNAL);
	this->addSignalRuleMatch(interface, eventName);
}

void GKDBus::initializeBroadcastSignal(BusConnection current, const char* object,
	const char* interface, const char* signal)
{
	if(this->signal_) /* sanity check */
		throw GLogiKExcept("DBus signal object already allocated");
	this->setCurrentConnection(current);

	try {
		this->signal_ = new GKDBusBroadcastSignal(this->current_conn_, object, interface, signal);
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		LOG(ERROR) << "GKDBus signal object allocation failure : " << e.what();
		throw GLogiKExcept("caught bad_alloc, do not like that");
	}
}

void GKDBus::appendToBroadcastSignal(const std::string & value) {
	if(this->signal_ == nullptr) /* sanity check */
		throw GLogiKExcept("DBus signal object not initialized");
	this->signal_->appendToBroadcastSignal(value);
}

void GKDBus::sendBroadcastSignal(void) {
	if(this->signal_) { /* sanity check */
		delete this->signal_;
		this->signal_ = nullptr;
	}
	else {
		LOG(WARNING) << __func__ << " failure because signal object not contructed";
	}
}

/* -- */
/*
 *	DBus Method Call Error Reply
 */

void GKDBus::buildAndSendErrorReply(BusConnection current) {
	try {
		this->initializeMessageErrorReply(current);
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << "DBus error reply failure : " << e.what();
	}

	/* delete error_reply object if allocated */
	this->sendMessageErrorReply();
}

void GKDBus::initializeMessageErrorReply(BusConnection current) {
	if(this->error_reply_) /* sanity check */
		throw GLogiKExcept("DBus reply object already allocated");
	this->setCurrentConnection(current);

	try {
		this->error_reply_ = new GKDBusMessageErrorReply(this->current_conn_, this->message_);
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		LOG(ERROR) << "GKDBus message error object allocation failure : " << e.what();
		throw GLogiKExcept("caught bad_alloc, do not like that");
	}
}

void GKDBus::sendMessageErrorReply(void) {
	if(this->error_reply_) { /* sanity check */
		delete this->error_reply_;
		this->error_reply_ = nullptr;
	}
	else {
		LOG(WARNING) << __func__ << " failure because error_reply object not contructed";
	}
}

/* -- */
/*
 *	DBus Method Call Reply
 */

void GKDBus::initializeMethodCallReply(BusConnection current) {
	if(this->reply_) /* sanity check */
		throw GLogiKExcept("DBus reply object already allocated");
	this->setCurrentConnection(current);

	try {
		this->reply_ = new GKDBusMessageReply(this->current_conn_, this->message_);
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		LOG(ERROR) << "GKDBus message reply object allocation failure : " << e.what();
		throw GLogiKExcept("caught bad_alloc, do not like that");
	}
}

void GKDBus::appendToMethodCallReply(const bool value) {
	if(this->reply_ == nullptr) /* sanity check */
		throw GLogiKExcept("DBus reply object not initialized");
	this->reply_->appendToMessageReply(value);
}

void GKDBus::appendToMethodCallReply(const std::string & value) {
	if(this->reply_ == nullptr) /* sanity check */
		throw GLogiKExcept("DBus reply object not initialized");
	this->reply_->appendToMessageReply(value);
}

void GKDBus::appendToMethodCallReply(const std::vector<std::string> & list) {
	if(this->reply_ == nullptr) /* sanity check */
		throw GLogiKExcept("DBus reply object not initialized");
	this->reply_->appendToMessageReply(list);
}

void GKDBus::appendExtraToMethodCallReply(const std::string & value) {
	this->extra_strings_.push_back(value);
}

void GKDBus::sendMethodCallReply(void) {
	if(this->reply_) { /* sanity check */
		delete this->reply_;
		this->reply_ = nullptr;
	}
	else {
		LOG(WARNING) << __func__ << " failure because reply object not contructed";
	}

	/*
	 * sendMethodCallReply is always called when processing DBus messages,
	 * even in case of exception, so we can clear extra strings container here
	 */
	this->extra_strings_.clear();
}

/* -- */
/*
 *	DBus Remote Object Method Call
 */

void GKDBus::initializeRemoteMethodCall(BusConnection current, const char* dest,
	const char* object, const char* interface, const char* method, const bool logoff)
{
	if(this->method_call_) /* sanity check */
		throw GLogiKExcept("DBus method_call object already allocated");
	this->setCurrentConnection(current);

	try {
		this->method_call_ = new GKDBusRemoteMethodCall(this->current_conn_, dest, object, interface, method, &this->pending_, logoff);
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		LOG(ERROR) << "GKDBus remote method call object allocation failure : " << e.what();
		throw GLogiKExcept("caught bad_alloc, do not like that");
	}
}

void GKDBus::appendToRemoteMethodCall(const std::string & value) {
	if(this->method_call_ == nullptr) /* sanity check */
		throw GLogiKExcept("DBus remote method call object not initialized");
	this->method_call_->appendToRemoteMethodCall(value);
}

void GKDBus::appendToRemoteMethodCall(const uint32_t value) {
	if(this->method_call_ == nullptr) /* sanity check */
		throw GLogiKExcept("DBus remote method call object not initialized");
	this->method_call_->appendToRemoteMethodCall(value);
}

void GKDBus::sendRemoteMethodCall(void) {
	if(this->method_call_) { /* sanity check */
		delete this->method_call_;
		this->method_call_ = nullptr;
	}
	else {
		LOG(WARNING) << __func__ << " failure because method_call object not contructed";
	}
}

void GKDBus::waitForRemoteMethodCallReply(void) {
	dbus_pending_call_block(this->pending_);

	uint8_t c = 0;

	DBusMessage* message = nullptr;
	// TODO could set a timer between retries ?
	while( message == nullptr and c < 10 ) {
		message = dbus_pending_call_steal_reply(this->pending_);
		c++;
	}

	dbus_pending_call_unref(this->pending_);

	if(message == nullptr) {
		LOG(DEBUG3) << __func__ << " message is NULL, retried 10 times";
		throw GKDBusRemoteCallNoReply("can't get pending call reply");
	}

	if(dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_ERROR) {
		dbus_message_unref(message);
		throw GKDBusRemoteCallNoReply("got error as reply");
	}

	this->fillInArguments(message);
	dbus_message_unref(message);
}

/* -- */
/*
 *	Functions to get DBus message arguments in the order they were provided
 */

std::string GKDBus::getNextStringArgument(void) {
	if( this->string_arguments_.empty() )
		throw EmptyContainer("no string argument");
	std::string ret = this->string_arguments_.back();
	this->string_arguments_.pop_back();
	return ret;
}

const std::vector<std::string> & GKDBus::getAllStringArguments(void) const {
	return this->string_arguments_;
}

const bool GKDBus::getNextBooleanArgument(void) {
	if( this->boolean_arguments_.empty() )
		throw EmptyContainer("no boolean argument");
	const bool ret = this->boolean_arguments_.back();
	this->boolean_arguments_.pop_back();
	return ret;
}

/* -- */
/*
 * check if :
 *      one of the registered signals
 *   on one of the registered interfaces
 *   on one of the registered object path
 * was receipted. if yes, then run the corresponding callback function
 * TODO loop on other containers
 */
void GKDBus::checkForSignalReceipt(BusConnection current) {
	const char* obj = dbus_message_get_path(this->message_);
	std::string asked_object_path("");
	if(obj != nullptr)
		asked_object_path = obj;

	/*
	 * loops on containers
	 */

	std::string object_path;

	for(const auto & object_it : this->events_string_to_bool_) {
		/* object path must match */
		object_path = this->getNode(object_it.first);
		if(object_path != asked_object_path)
			continue;

		for(const auto & interface : object_it.second) {
			LOG(DEBUG2) << "checking " << interface.first << " interface";

			for(const auto & DBusEvent : interface.second) { /* vector of struct */
				/* we want only signals */
				if( DBusEvent.eventType != GKDBusEventType::GKDBUS_EVENT_SIGNAL )
					continue;

				const char* signal = DBusEvent.eventName.c_str();
				LOG(DEBUG3) << "checking for " << signal << " receipt";
				if( this->checkMessageForSignalReceipt(interface.first.c_str(), signal) ) {
					/* free message, callback could send DBus requests */
					this->freeMessage();
					try {
						/* call string to bool callback */
						const std::string arg( this->getNextStringArgument() );
						DBusEvent.callback(arg);
					}
					catch ( const EmptyContainer & e ) {
						LOG(WARNING) << e.what();
					}

					return; /* only one by message */
				}
			}
		}
	}

	for(const auto & object_it : this->events_void_to_void_) {
		/* object path must match */
		object_path = this->getNode(object_it.first);
		if(object_path != asked_object_path)
			continue;

		for(const auto & interface : object_it.second) {
			LOG(DEBUG2) << "checking " << interface.first << " interface";

			for(const auto & DBusEvent : interface.second) { /* vector of struct */
				/* we want only signals */
				if( DBusEvent.eventType != GKDBusEventType::GKDBUS_EVENT_SIGNAL )
					continue;

				const char* signal = DBusEvent.eventName.c_str();
				LOG(DEBUG3) << "checking for " << signal << " receipt";
				if( this->checkMessageForSignalReceipt(interface.first.c_str(), signal) ) {
					/* free message, callback could send DBus requests */
					this->freeMessage();
					/* call void to void callback */
					DBusEvent.callback();
					return; /* only one by message */
				}
			}
		}
	}
}

/* -- */
/*
 * check if :
 *      one of the registered methods
 *   on one of the registered interfaces
 *   on one of the registered object path
 * was called. if yes, then run the corresponding callback function
 * and send DBus reply after appending the return value
 */
void GKDBus::checkForMethodCall(BusConnection current) {
	const char* obj = dbus_message_get_path(this->message_);
	std::string asked_object_path("");
	if(obj != nullptr)
		asked_object_path = obj;

	/* handle particular case */
	if(asked_object_path == this->getRootNode()) {
		if( this->checkMessageForMethodCall("org.freedesktop.DBus.Introspectable", "Introspect") ) {
			const std::string ret( this->introspectRootNode() );

			try {
				this->initializeMethodCallReply(current);
				this->appendToMethodCallReply(ret);
				/* no extra values */
			}
			catch (const GKDBusOOMWrongBuild & e) {
				LOG(ERROR) << "DBus build reply failure : " << e.what();
				/* delete reply object if allocated */
				this->sendMethodCallReply();
				this->buildAndSendErrorReply(current);
				return;
			}
			catch (const GLogiKExcept & e) {
				LOG(ERROR) << "DBus reply failure : " << e.what();
				/* delete reply object if allocated */
				this->sendMethodCallReply();
				throw;
			}

			/* delete reply object if allocated */
			this->sendMethodCallReply();

			return; /* only one by message */
		}
	}
	/* end particular case */


	/*
	 * loops on containers
	 */

	std::string object_path;

	for(const auto & object_it : this->events_void_to_string_) {
		/* object path must match */
		object_path = this->getNode(object_it.first);
		if(object_path != asked_object_path)
			continue;

		for(const auto & interface : object_it.second) {
			LOG(DEBUG2) << "checking " << interface.first << " interface";

			for(const auto & DBusEvent : interface.second) { /* vector of struct */
				/* we want only methods */
				if( DBusEvent.eventType != GKDBusEventType::GKDBUS_EVENT_METHOD )
					continue;

				const char* method = DBusEvent.eventName.c_str();
				LOG(DEBUG3) << "checking for " << method << " call";
				if( this->checkMessageForMethodCall(interface.first.c_str(), method) ) {
					/* call void to string callback */
					const std::string ret( DBusEvent.callback() );

					try {
						this->initializeMethodCallReply(current);
						this->appendToMethodCallReply(ret);
						/* extra values to send */
						this->appendExtraValuesToReply();
					}
					catch (const GKDBusOOMWrongBuild & e) {
						LOG(ERROR) << "DBus build reply failure : " << e.what();
						/* delete reply object if allocated */
						this->sendMethodCallReply();
						this->buildAndSendErrorReply(current);
						return;
					}
					catch ( const GLogiKExcept & e ) {
						LOG(ERROR) << "DBus reply failure : " << e.what();
						/* delete reply object if allocated */
						this->sendMethodCallReply();
						throw;
					}

					/* delete reply object if allocated */
					this->sendMethodCallReply();

					return; /* only one by message */
				}
			}
		}
	}

	for(const auto & object_it : this->events_void_to_stringsarray_) {
		/* object path must match */
		object_path = this->getNode(object_it.first);
		if(object_path != asked_object_path)
			continue;

		for(const auto & interface : object_it.second) {
			LOG(DEBUG2) << "checking " << interface.first << " interface";

			for(const auto & DBusEvent : interface.second) { /* vector of struct */
				/* we want only methods */
				if( DBusEvent.eventType != GKDBusEventType::GKDBUS_EVENT_METHOD )
					continue;

				const char* method = DBusEvent.eventName.c_str();
				LOG(DEBUG3) << "checking for " << method << " call";
				if( this->checkMessageForMethodCall(interface.first.c_str(), method) ) {
					/* call void to strings array callback */
					const std::vector<std::string> ret = DBusEvent.callback();

					try {
						this->initializeMethodCallReply(current);
						this->appendToMethodCallReply(ret);
						/* extra values to send */
						this->appendExtraValuesToReply();
					}
					catch (const GKDBusOOMWrongBuild & e) {
						LOG(ERROR) << "DBus build reply failure : " << e.what();
						/* delete reply object if allocated */
						this->sendMethodCallReply();
						this->buildAndSendErrorReply(current);
						return;
					}
					catch ( const GLogiKExcept & e ) {
						LOG(ERROR) << "DBus reply failure : " << e.what();
						/* delete reply object if allocated */
						this->sendMethodCallReply();
						throw;
					}

					/* delete reply object if allocated */
					this->sendMethodCallReply();

					return; /* only one by message */
				}
			}
		}
	}

	for(const auto & object_it : this->events_string_to_stringsarray_) {
		/* object path must match */
		object_path = this->getNode(object_it.first);
		if(object_path != asked_object_path)
			continue;

		for(const auto & interface : object_it.second) {
			LOG(DEBUG2) << "checking " << interface.first << " interface";

			for(const auto & DBusEvent : interface.second) { /* vector of struct */
				/* we want only methods */
				if( DBusEvent.eventType != GKDBusEventType::GKDBUS_EVENT_METHOD )
					continue;

				const char* method = DBusEvent.eventName.c_str();
				LOG(DEBUG3) << "checking for " << method << " call";
				if( this->checkMessageForMethodCall(interface.first.c_str(), method) ) {
					std::vector<std::string> ret;

					try {
						/* call string to strings array callback */
						const std::string arg( this->getNextStringArgument() );
						ret = DBusEvent.callback(arg);
					}
					catch ( const EmptyContainer & e ) {
						LOG(WARNING) << e.what();
					}


					try {
						this->initializeMethodCallReply(current);
						this->appendToMethodCallReply(ret);
						/* extra values to send */
						this->appendExtraValuesToReply();
					}
					catch (const GKDBusOOMWrongBuild & e) {
						LOG(ERROR) << "DBus build reply failure : " << e.what();
						/* delete reply object if allocated */
						this->sendMethodCallReply();
						this->buildAndSendErrorReply(current);
						return;
					}
					catch ( const GLogiKExcept & e ) {
						LOG(ERROR) << "DBus reply failure : " << e.what();
						/* delete reply object if allocated */
						this->sendMethodCallReply();
						throw;
					}

					/* delete reply object if allocated */
					this->sendMethodCallReply();

					return; /* only one by message */
				}
			}
		}
	}

	for(const auto & object_it : this->events_string_to_bool_) {
		/* object path must match */
		object_path = this->getNode(object_it.first);
		if(object_path != asked_object_path)
			continue;

		for(const auto & interface : object_it.second) {
			LOG(DEBUG2) << "checking " << interface.first << " interface";

			for(const auto & DBusEvent : interface.second) { /* vector of struct */
				/* we want only methods */
				if( DBusEvent.eventType != GKDBusEventType::GKDBUS_EVENT_METHOD )
					continue;

				const char* method = DBusEvent.eventName.c_str();
				LOG(DEBUG3) << "checking for " << method << " call";
				if( this->checkMessageForMethodCall(interface.first.c_str(), method) ) {
					bool ret = false;

					try {
						/* call string to bool callback */
						const std::string arg( this->getNextStringArgument() );
						ret = DBusEvent.callback(arg);
					}
					catch ( const EmptyContainer & e ) {
						LOG(WARNING) << e.what();
					}

					try {
						this->initializeMethodCallReply(current);
						this->appendToMethodCallReply(ret);
						/* extra values to send */
						this->appendExtraValuesToReply();
					}
					catch (const GKDBusOOMWrongBuild & e) {
						LOG(ERROR) << "DBus build reply failure : " << e.what();
						/* delete reply object if allocated */
						this->sendMethodCallReply();
						this->buildAndSendErrorReply(current);
						return;
					}
					catch ( const GLogiKExcept & e ) {
						LOG(ERROR) << "DBus reply failure : " << e.what();
						/* delete reply object if allocated */
						this->sendMethodCallReply();
						throw;
					}

					/* delete reply object if allocated */
					this->sendMethodCallReply();

					return; /* only one by message */
				}
			}
		}
	}

	for(const auto & object_it : this->events_twostrings_to_bool_) {
		/* object path must match */
		object_path = this->getNode(object_it.first);
		if(object_path != asked_object_path)
			continue;

		for(const auto & interface : object_it.second) {
			LOG(DEBUG2) << "checking " << interface.first << " interface";

			for(const auto & DBusEvent : interface.second) { /* vector of struct */
				/* we want only methods */
				if( DBusEvent.eventType != GKDBusEventType::GKDBUS_EVENT_METHOD )
					continue;

				const char* method = DBusEvent.eventName.c_str();
				LOG(DEBUG3) << "checking for " << method << " call";
				if( this->checkMessageForMethodCall(interface.first.c_str(), method) ) {
					bool ret = false;

					try {
						/* call two strings to bool callback */
						const std::string arg1( this->getNextStringArgument() );
						const std::string arg2( this->getNextStringArgument() );
						ret = DBusEvent.callback(arg1, arg2);
					}
					catch ( const EmptyContainer & e ) {
						LOG(WARNING) << e.what();
					}

					try {
						this->initializeMethodCallReply(current);
						this->appendToMethodCallReply(ret);
						/* extra values to send */
						this->appendExtraValuesToReply();
					}
					catch (const GKDBusOOMWrongBuild & e) {
						LOG(ERROR) << "DBus build reply failure : " << e.what();
						/* delete reply object if allocated */
						this->sendMethodCallReply();
						this->buildAndSendErrorReply(current);
						return;
					}
					catch ( const GLogiKExcept & e ) {
						LOG(ERROR) << "DBus reply failure : " << e.what();
						/* delete reply object if allocated */
						this->sendMethodCallReply();
						throw;
					}

					/* delete reply object if allocated */
					this->sendMethodCallReply();

					return; /* only one by message */
				}
			}
		}
	}

	for(const auto & object_it : this->events_string_to_string_) {
		/* object path must match */
		object_path = this->getNode(object_it.first);
		if(object_path != asked_object_path)
			continue;

		for(const auto & interface : object_it.second) {
			LOG(DEBUG2) << "checking " << interface.first << " interface";

			for(const auto & DBusEvent : interface.second) { /* vector of struct */
				/* we want only methods */
				if( DBusEvent.eventType != GKDBusEventType::GKDBUS_EVENT_METHOD )
					continue;

				const char* method = DBusEvent.eventName.c_str();
				LOG(DEBUG3) << "checking for " << method << " call";
				if( this->checkMessageForMethodCall(interface.first.c_str(), method) ) {
					/* call string to string callback */
					const std::string & arg = object_it.first;
					const std::string ret( DBusEvent.callback(arg) );

					try {
						this->initializeMethodCallReply(current);
						this->appendToMethodCallReply(ret);
						/* extra values to send */
						this->appendExtraValuesToReply();
					}
					catch (const GKDBusOOMWrongBuild & e) {
						LOG(ERROR) << "DBus build reply failure : " << e.what();
						/* delete reply object if allocated */
						this->sendMethodCallReply();
						this->buildAndSendErrorReply(current);
						return;
					}
					catch ( const GLogiKExcept & e ) {
						LOG(ERROR) << "DBus reply failure : " << e.what();
						/* delete reply object if allocated */
						this->sendMethodCallReply();
						throw;
					}

					/* delete reply object if allocated */
					this->sendMethodCallReply();

					return; /* only one by message */
				}
			}
		}
	}
}

/* -- */
/*
 * private
 */

void GKDBus::checkDBusError(const char* error_message) {
	if( dbus_error_is_set(&this->error_) ) {
		this->buffer_.str(error_message);
		this->buffer_ << " : " << this->error_.message;
		dbus_error_free(&this->error_);
		throw GLogiKExcept(this->buffer_.str());
	}
}

void GKDBus::fillInArguments(DBusMessage* message) {
	this->string_arguments_.clear();
	this->boolean_arguments_.clear();

	if(message == nullptr) {
		LOG(WARNING) << __func__ << " : message is NULL";
		return;
	}

	int current_type = 0;
	int sub_type = 0;
	char* arg_value = nullptr;
	bool bool_arg = false;
	DBusMessageIter arg_it;
	char* sub_value = nullptr;

	dbus_message_iter_init(message, &arg_it);
	while ((current_type = dbus_message_iter_get_arg_type(&arg_it)) != DBUS_TYPE_INVALID) {
		DBusMessageIter sub_it;
		bool_arg = false;
		sub_type = 0;
		arg_value = nullptr;
		sub_value = nullptr;

		switch(current_type) {
			case DBUS_TYPE_STRING:
				dbus_message_iter_get_basic(&arg_it, &arg_value);
				this->string_arguments_.push_back(arg_value);
				//LOG(DEBUG4) << "string arg value : " << arg_value;
				break;
			case DBUS_TYPE_OBJECT_PATH:
				dbus_message_iter_get_basic(&arg_it, &arg_value);
				this->string_arguments_.push_back(arg_value);
				//LOG(DEBUG4) << "object path arg value : " << arg_value;
				break;
			case DBUS_TYPE_BOOLEAN:
				dbus_message_iter_get_basic(&arg_it, &bool_arg);
				this->boolean_arguments_.push_back(bool_arg);
				//LOG(DEBUG4) << "bool arg value : " << bool_arg;
				break;
			case DBUS_TYPE_ARRAY:
				dbus_message_iter_recurse(&arg_it, &sub_it);
				switch( dbus_message_iter_get_element_type(&arg_it) ) {
					case DBUS_TYPE_STRING:
						//LOG(DEBUG3) << "found array of strings";
						while ((sub_type = dbus_message_iter_get_arg_type(&sub_it)) == DBUS_TYPE_STRING) {
							dbus_message_iter_get_basic(&sub_it, &arg_value);
							this->string_arguments_.push_back(arg_value);
							//LOG(DEBUG4) << "object path arg value : " << arg_value;
							dbus_message_iter_next(&sub_it);
						}
						break;
					default:
						LOG(WARNING) << "unhandled type for dbus array";
						break;
				}
				break;
			case DBUS_TYPE_VARIANT:
				//LOG(DEBUG4) << "need to parse variant";
				dbus_message_iter_recurse(&arg_it, &sub_it);
				sub_type = dbus_message_iter_get_arg_type(&sub_it);

				switch(sub_type) {
					case DBUS_TYPE_STRING:
						dbus_message_iter_get_basic(&sub_it, &sub_value);
						this->string_arguments_.push_back(sub_value);
						//LOG(DEBUG4) << "variant string sub value : " << sub_value;
						break;
					default:
						LOG(WARNING) << "unhandled type for variant parsing";
						break;
				}
				break;
			default: /* other dbus type */
				break;
		}

		dbus_message_iter_next(&arg_it);
	}

	if( ! this->string_arguments_.empty() )
		std::reverse(this->string_arguments_.begin(), this->string_arguments_.end());
	if( ! this->boolean_arguments_.empty() )
		std::reverse(this->boolean_arguments_.begin(), this->boolean_arguments_.end());
}

void GKDBus::setCurrentConnection(BusConnection current) {
	switch(current) {
		case BusConnection::GKDBUS_SESSION :
			if(this->session_conn_ == nullptr)
				throw GLogiKExcept("DBus Session connection not opened");
			this->current_conn_ = this->session_conn_;
			break;
		case BusConnection::GKDBUS_SYSTEM :
			if(this->system_conn_ == nullptr)
				throw GLogiKExcept("DBus System connection not opened");
			this->current_conn_ = this->system_conn_;
			break;
		default:
			throw GLogiKExcept("asked connection not handled");
			break;
	}
}

void GKDBus::addSignalRuleMatch(const char* interface, const char* eventName) {
	std::string rule = "type='signal',interface='";
	rule += interface;
	rule += "',member='";
	rule += eventName;
	rule += "'";
	dbus_bus_add_match(this->current_conn_, rule.c_str(), &this->error_);
	dbus_connection_flush(this->current_conn_);
	this->checkDBusError("DBus Session match error");
	LOG(DEBUG1) << "DBus Signal match rule sent : " << rule;
}

void GKDBus::appendExtraValuesToReply(void) {
	if( ! this->extra_strings_.empty() ) {
		for( const std::string & value : this->extra_strings_ ) {
			this->appendToMethodCallReply(value);
		}
	}
}

/* -- */

} // namespace GLogiK

