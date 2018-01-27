/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2018  Fabrice Delliaux <netbox253@gmail.com>
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

#include "GKDBusEventTemplates.h"

namespace NSGKDBus
{

using namespace NSGKUtils;

void SignalRule::addSignalRuleMatch(
		DBusConnection* connection,
		const char* interface,
		const char* eventName
	)
{
	std::string rule = "type='signal',interface='";
	rule += interface;
	rule += "',member='";
	rule += eventName;
	rule += "'";
	dbus_bus_add_match(connection, rule.c_str(), nullptr);
	dbus_connection_flush(connection);
	// FIXME check error
	//this->checkDBusError("DBus Session match error");
#if DEBUGGING_ON
	LOG(DEBUG1) << "DBus Signal match rule sent : " << rule;
#endif
}

template <>
	void GKDBusCallbackEvent<VoidToVoid>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	)
{
	/* don't need arguments */

	try {
		/* call void to void callback */
		this->callback();
	}
	catch ( const GLogiKExcept & e ) {
		const char* error = e.what();
		LOG(ERROR) << error;
	}
	/* don't need to send a reply */
}

template <>
	void GKDBusCallbackEvent<StringToBool>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	)
{
	GKDBusArgumentString::fillInArguments(message);

	bool ret = false;

	try {
		const std::string arg( GKDBusArgumentString::getNextStringArgument() );

		/* call string to bool callback */
		ret = this->callback(arg);
	}
	catch ( const GLogiKExcept & e ) {
		if( this->sendCallbackError(connection, message, e.what()) )
			return;
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendBooleanToReply(ret);
	}
	catch ( const GLogiKExcept & e ) {
		this->sendError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<TwoStringsToBool>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	)
{
	GKDBusArgumentString::fillInArguments(message);

	bool ret = false;

	try {
		const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
		const std::string arg2( GKDBusArgumentString::getNextStringArgument() );

		/* call two strings to bool callback */
		ret = this->callback(arg1, arg2);
	}
	catch ( const GLogiKExcept & e ) {
		if( this->sendCallbackError(connection, message, e.what()) )
			return;
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendBooleanToReply(ret);
	}
	catch ( const GLogiKExcept & e ) {
		this->sendError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<StringToString>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	)
{
	std::string arg;
	std::string ret;

	try {
		/* handle introspection special case */
		if(this->eventName == "Introspect") {
			arg = to_string( dbus_message_get_path(message) );
		}
		else {
			GKDBusArgumentString::fillInArguments(message);
			arg = GKDBusArgumentString::getNextStringArgument();
		}

		/* call string to string callback */
		ret = this->callback(arg);
	}
	catch ( const GLogiKExcept & e ) {
		if( this->sendCallbackError(connection, message, e.what()) )
			return;
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendStringToReply(ret);
	}
	catch ( const GLogiKExcept & e ) {
		this->sendError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<TwoStringsToString>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	)
{
	GKDBusArgumentString::fillInArguments(message);

	std::string ret;

	try {
	 const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
	 const std::string arg2( GKDBusArgumentString::getNextStringArgument() );

		/* call two strings to string callback */
		ret = this->callback(arg1, arg2);
	}
	catch ( const GLogiKExcept & e ) {
		if( this->sendCallbackError(connection, message, e.what()) )
			return;
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendStringToReply(ret);
	}
	catch ( const GLogiKExcept & e ) {
		this->sendError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<StringToStringsArray>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	)
{
	GKDBusArgumentString::fillInArguments(message);

	std::vector<std::string> ret;

	try {
		const std::string arg( GKDBusArgumentString::getNextStringArgument() );

		/* call string to strings array callback */
		ret = this->callback(arg);
	}
	catch ( const GLogiKExcept & e ) {
		if( this->sendCallbackError(connection, message, e.what()) )
			return;
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendStringVectorToReply(ret);
	}
	catch ( const GLogiKExcept & e ) {
		this->sendError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<TwoStringsToStringsArray>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	)
{
	GKDBusArgumentString::fillInArguments(message);

	std::vector<std::string> ret;

	try {
		const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
		const std::string arg2( GKDBusArgumentString::getNextStringArgument() );

		/* call two strings to strings array callback */
		ret = this->callback(arg1, arg2);
	}
	catch ( const GLogiKExcept & e ) {
		const char* error = e.what();
		LOG(ERROR) << error;

		if(this->eventType != GKDBusEventType::GKDBUS_EVENT_SIGNAL) {
			/* send error if something was wrong when running callback */
			this->buildAndSendErrorReply(connection, message, error);
			return;
		}
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendStringVectorToReply(ret);
	}
	catch ( const GLogiKExcept & e ) {
		this->sendError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<TwoStringsOneByteToBool>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	)
{
	GKDBusArgumentString::fillInArguments(message);

	bool ret = false;

	try {
		const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
		const std::string arg2( GKDBusArgumentString::getNextStringArgument() );
		const uint8_t arg3 = this->getNextByteArgument();

		/* call two strings one byte to bool callback */
		ret = this->callback(arg1, arg2, arg3);
	}
	catch ( const GLogiKExcept & e ) {
		if( this->sendCallbackError(connection, message, e.what()) )
			return;
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendBooleanToReply(ret);
	}
	catch ( const GLogiKExcept & e ) {
		this->sendError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<TwoStringsThreeBytesToBool>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	)
{
	GKDBusArgumentString::fillInArguments(message);

	bool ret = false;

	try {
		const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
		const std::string arg2( GKDBusArgumentString::getNextStringArgument() );
		const uint8_t arg3 = this->getNextByteArgument();
		const uint8_t arg4 = this->getNextByteArgument();
		const uint8_t arg5 = this->getNextByteArgument();

		/* call two strings three bytes to bool callback */
		ret = this->callback(arg1, arg2, arg3, arg4, arg5);
	}
	catch ( const GLogiKExcept & e ) {
		if( this->sendCallbackError(connection, message, e.what()) )
			return;
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendBooleanToReply(ret);
	}
	catch ( const GLogiKExcept & e ) {
		this->sendError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<StringsArrayToVoid>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	)
{
	GKDBusArgumentString::fillInArguments(message);

	try {
		const std::vector<std::string> arg( GKDBusArgumentString::getStringsArray() );

		/* call string to void callback */
		this->callback(arg);
	}
	catch ( const GLogiKExcept & e ) {
		const char* error = e.what();
		LOG(ERROR) << error;
	}

	/* don't need to send a reply */
}

template <>
	void GKDBusCallbackEvent<ThreeStringsOneByteToMacro>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	)
{
	GKDBusArgumentString::fillInArguments(message);

	GLogiK::macro_t ret;

	try {
		const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
		const std::string arg2( GKDBusArgumentString::getNextStringArgument() );
		const std::string arg3( GKDBusArgumentString::getNextStringArgument() );
		const uint8_t arg4 = this->getNextByteArgument();

		/* call three strings one byte to macro_t callback */
		ret = this->callback(arg1, arg2, arg3, arg4);
	}
	catch ( const GLogiKExcept & e ) {
		if( this->sendCallbackError(connection, message, e.what()) )
			return;
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendMacroToReply(ret);
	}
	catch ( const GLogiKExcept & e ) {
		this->sendError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<TwoStringsOneByteOneMacrosBankToBool>::runCallback(
		DBusConnection* connection,
		DBusMessage* message
	)
{
	GKDBusArgumentString::fillInArguments(message);
	bool ret = false;

	try {
		const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
		const std::string arg2( GKDBusArgumentString::getNextStringArgument() );
		const uint8_t arg3 = this->getNextByteArgument();
		const GLogiK::macros_bank_t arg4 = this->getNextMacrosBankArgument();

		/* call two strings one byte one MacrosBank(macros_bank_t) to bool callback */
		ret = this->callback(arg1, arg2, arg3, arg4);
	}
	catch ( const GLogiKExcept & e ) {
		if( this->sendCallbackError(connection, message, e.what()) )
			return;
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendBooleanToReply(ret);
	}
	catch ( const GLogiKExcept & e ) {
		this->sendError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

} // namespace NSGKDBus

