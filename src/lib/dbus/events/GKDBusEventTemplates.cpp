/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2022  Fabrice Delliaux <netbox253@gmail.com>
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

#include "arguments/GKDBusArgument.hpp"

#include "GKDBusEventTemplates.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

template <>
	void GKDBusCallbackEvent<VoidToVoid>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	)
{
	GK_LOG_FUNC

	/* don't need arguments */

	try {
		/* call void to void callback */
		this->callback();
	}
	catch ( const GLogiKExcept & e ) {
		LOG(error) << e.what();
	}
	/* don't need to send a reply */
}

template <>
	void GKDBusCallbackEvent<StringToVoid>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	)
{
	GKDBusArgument::fillInArguments(message);

	try {
		const std::string arg1( GKDBusArgumentString::getNextStringArgument() );

		/* call string to void callback */
		this->callback(arg1);
	}
	catch ( const GLogiKExcept & e ) {
		/* send error if necessary when something was wrong */
		this->sendCallbackError(connection, message, e.what());
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);

		this->appendAsyncArgsToReply();
	}
	catch ( const GLogiKExcept & e ) {
		/* delete reply object if allocated and send error reply */
		this->sendReplyError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<TwoStringsToVoid>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	)
{
	GKDBusArgument::fillInArguments(message);

	try {
		const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
		const std::string arg2( GKDBusArgumentString::getNextStringArgument() );

		/* call two strings to void callback */
		this->callback(arg1, arg2);
	}
	catch ( const GLogiKExcept & e ) {
		/* send error if necessary when something was wrong */
		this->sendCallbackError(connection, message, e.what());
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);

		this->appendAsyncArgsToReply();
	}
	catch ( const GLogiKExcept & e ) {
		/* delete reply object if allocated and send error reply */
		this->sendReplyError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<StringToBool>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	)
{
	GKDBusArgument::fillInArguments(message);

	bool ret = false;

	try {
		const std::string arg( GKDBusArgumentString::getNextStringArgument() );

		/* call string to bool callback */
		ret = this->callback(arg);
	}
	catch ( const GLogiKExcept & e ) {
		/* send error if necessary when something was wrong */
		this->sendCallbackError(connection, message, e.what());
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendBooleanToReply(ret);

		this->appendAsyncArgsToReply();
	}
	catch ( const GLogiKExcept & e ) {
		/* delete reply object if allocated and send error reply */
		this->sendReplyError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<TwoStringsToBool>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	)
{
	GKDBusArgument::fillInArguments(message);

	bool ret = false;

	try {
		const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
		const std::string arg2( GKDBusArgumentString::getNextStringArgument() );

		/* call two strings to bool callback */
		ret = this->callback(arg1, arg2);
	}
	catch ( const GLogiKExcept & e ) {
		/* send error if necessary when something was wrong */
		this->sendCallbackError(connection, message, e.what());
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendBooleanToReply(ret);

		this->appendAsyncArgsToReply();
	}
	catch ( const GLogiKExcept & e ) {
		/* delete reply object if allocated and send error reply */
		this->sendReplyError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<StringToString>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	)
{
	std::string arg;
	std::string ret;

	try {
		/* handle introspection special case */
		if(this->eventName == "Introspect") {
			arg = toString( dbus_message_get_path(message) );
		}
		else {
			GKDBusArgument::fillInArguments(message);
			arg = GKDBusArgumentString::getNextStringArgument();
		}

		/* call string to string callback */
		ret = this->callback(arg);
	}
	catch ( const GLogiKExcept & e ) {
		/* send error if necessary when something was wrong */
		this->sendCallbackError(connection, message, e.what());
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendStringToReply(ret);

		this->appendAsyncArgsToReply();
	}
	catch ( const GLogiKExcept & e ) {
		/* delete reply object if allocated and send error reply */
		this->sendReplyError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<TwoStringsToString>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	)
{
	GKDBusArgument::fillInArguments(message);

	std::string ret;

	try {
	 const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
	 const std::string arg2( GKDBusArgumentString::getNextStringArgument() );

		/* call two strings to string callback */
		ret = this->callback(arg1, arg2);
	}
	catch ( const GLogiKExcept & e ) {
		/* send error if necessary when something was wrong */
		this->sendCallbackError(connection, message, e.what());
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendStringToReply(ret);

		this->appendAsyncArgsToReply();
	}
	catch ( const GLogiKExcept & e ) {
		/* delete reply object if allocated and send error reply */
		this->sendReplyError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<StringToStringsArray>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	)
{
	GKDBusArgument::fillInArguments(message);

	std::vector<std::string> ret;

	try {
		const std::string arg( GKDBusArgumentString::getNextStringArgument() );

		/* call string to strings array callback */
		ret = this->callback(arg);
	}
	catch ( const GLogiKExcept & e ) {
		/* send error if necessary when something was wrong */
		this->sendCallbackError(connection, message, e.what());
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendStringVectorToReply(ret);

		this->appendAsyncArgsToReply();
	}
	catch ( const GLogiKExcept & e ) {
		/* delete reply object if allocated and send error reply */
		this->sendReplyError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<TwoStringsToStringsArray>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	)
{
	GKDBusArgument::fillInArguments(message);

	std::vector<std::string> ret;

	try {
		const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
		const std::string arg2( GKDBusArgumentString::getNextStringArgument() );

		/* call two strings to strings array callback */
		ret = this->callback(arg1, arg2);
	}
	catch ( const GLogiKExcept & e ) {
		/* send error if necessary when something was wrong */
		this->sendCallbackError(connection, message, e.what());
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendStringVectorToReply(ret);

		this->appendAsyncArgsToReply();
	}
	catch ( const GLogiKExcept & e ) {
		/* delete reply object if allocated and send error reply */
		this->sendReplyError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<TwoStringsOneByteToBool>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	)
{
	GKDBusArgument::fillInArguments(message);

	bool ret = false;

	try {
		const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
		const std::string arg2( GKDBusArgumentString::getNextStringArgument() );
		const uint8_t arg3 = GKDBusArgumentByte::getNextByteArgument();

		/* call two strings one byte to bool callback */
		ret = this->callback(arg1, arg2, arg3);
	}
	catch ( const GLogiKExcept & e ) {
		/* send error if necessary when something was wrong */
		this->sendCallbackError(connection, message, e.what());
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendBooleanToReply(ret);

		this->appendAsyncArgsToReply();
	}
	catch ( const GLogiKExcept & e ) {
		/* delete reply object if allocated and send error reply */
		this->sendReplyError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

/* TODO currently same signature as ResetDeviceMacrosBank
template <>
	void GKDBusCallbackEvent<DeviceMacroChanged>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	)
{
	GKDBusArgument::fillInArguments(message);

	bool ret = false;

	try {
		const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
		const std::string arg2( GKDBusArgumentString::getNextStringArgument() );
		const GLogiK::MKeysID arg3 = GKDBusArgumentMKeysID::getNextMKeysIDArgument();

		// call DeviceMacroChanged callback
		ret = this->callback(arg1, arg2, arg3);
	}
	catch ( const GLogiKExcept & e ) {
		// send error if necessary when something was wrong
		this->sendCallbackError(connection, message, e.what());
	}

	// signals don't send reply
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendBooleanToReply(ret);

		this->appendAsyncArgsToReply();
	}
	catch ( const GLogiKExcept & e ) {
		// delete reply object if allocated and send error reply
		this->sendReplyError(connection, message, e.what());
		return;
	}

	// delete reply object if allocated
	this->sendReply();
}
*/

template <>
	void GKDBusCallbackEvent<ResetDeviceMacrosBank>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	)
{
	GKDBusArgument::fillInArguments(message);

	bool ret = false;

	try {
		const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
		const std::string arg2( GKDBusArgumentString::getNextStringArgument() );
		const GLogiK::MKeysID arg3 = GKDBusArgumentMKeysID::getNextMKeysIDArgument();

		/* call ResetDeviceMacrosBank callback */
		ret = this->callback(arg1, arg2, arg3);
	}
	catch ( const GLogiKExcept & e ) {
		/* send error if necessary when something was wrong */
		this->sendCallbackError(connection, message, e.what());
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendBooleanToReply(ret);

		this->appendAsyncArgsToReply();
	}
	catch ( const GLogiKExcept & e ) {
		/* delete reply object if allocated and send error reply */
		this->sendReplyError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<TwoStringsThreeBytesToBool>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	)
{
	GKDBusArgument::fillInArguments(message);

	bool ret = false;

	try {
		const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
		const std::string arg2( GKDBusArgumentString::getNextStringArgument() );
		const uint8_t arg3 = GKDBusArgumentByte::getNextByteArgument();
		const uint8_t arg4 = GKDBusArgumentByte::getNextByteArgument();
		const uint8_t arg5 = GKDBusArgumentByte::getNextByteArgument();

		/* call two strings three bytes to bool callback */
		ret = this->callback(arg1, arg2, arg3, arg4, arg5);
	}
	catch ( const GLogiKExcept & e ) {
		/* send error if necessary when something was wrong */
		this->sendCallbackError(connection, message, e.what());
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendBooleanToReply(ret);

		this->appendAsyncArgsToReply();
	}
	catch ( const GLogiKExcept & e ) {
		/* delete reply object if allocated and send error reply */
		this->sendReplyError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<StringsArrayToVoid>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	)
{
	GK_LOG_FUNC

	GKDBusArgument::fillInArguments(message);

	try {
		const std::vector<std::string> arg( GKDBusArgumentString::getStringsArray() );

		/* call string to void callback */
		this->callback(arg);
	}
	catch ( const GLogiKExcept & e ) {
		LOG(error) << e.what();
	}

	/* don't need to send a reply */
}

template <>
	void GKDBusCallbackEvent<GetDeviceMacro>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	)
{
	GKDBusArgument::fillInArguments(message);

	GLogiK::macro_type ret;

	try {
		const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
		const std::string arg2( GKDBusArgumentString::getNextStringArgument() );
		const std::string arg3( GKDBusArgumentString::getNextStringArgument() );
		const GLogiK::MKeysID arg4 = GKDBusArgumentMKeysID::getNextMKeysIDArgument();

		/* call GetDeviceMacro callback */
		ret = this->callback(arg1, arg2, arg3, arg4);
	}
	catch ( const GLogiKExcept & e ) {
		/* send error if necessary when something was wrong */
		this->sendCallbackError(connection, message, e.what());
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendMacroToReply(ret);

		this->appendAsyncArgsToReply();
	}
	catch ( const GLogiKExcept & e ) {
		/* delete reply object if allocated and send error reply */
		this->sendReplyError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<TwoStringsToLCDPluginsPropertiesArray>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	)
{
	GKDBusArgument::fillInArguments(message);

	GLogiK::LCDPluginsPropertiesArray_type ret;

	try {
		const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
		const std::string arg2( GKDBusArgumentString::getNextStringArgument() );

		/* call two strings to LCDPluginsProperties array callback */
		ret = this->callback(arg1, arg2);
	}
	catch ( const GLogiKExcept & e ) {
		/* send error if necessary when something was wrong */
		this->sendCallbackError(connection, message, e.what());
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendLCDPluginsPropertiesArrayToReply(ret);

		this->appendAsyncArgsToReply();
	}
	catch ( const GLogiKExcept & e ) {
		/* delete reply object if allocated and send error reply */
		this->sendReplyError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<SetDeviceMacrosBank>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	)
{
	GKDBusArgument::fillInArguments(message);
	bool ret = false;

	try {
		const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
		const std::string arg2( GKDBusArgumentString::getNextStringArgument() );
		const GLogiK::MKeysID arg3 = GKDBusArgumentMKeysID::getNextMKeysIDArgument();
		const GLogiK::mBank_type arg4 = this->getNextMacrosBankArgument();

		/* call SetDeviceMacrosBank callback */
		ret = this->callback(arg1, arg2, arg3, arg4);
	}
	catch ( const GLogiKExcept & e ) {
		/* send error if necessary when something was wrong */
		this->sendCallbackError(connection, message, e.what());
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendBooleanToReply(ret);

		this->appendAsyncArgsToReply();
	}
	catch ( const GLogiKExcept & e ) {
		/* delete reply object if allocated and send error reply */
		this->sendReplyError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

template <>
	void GKDBusCallbackEvent<TwoStringsOneByteOneUInt64ToBool>::runCallback(
		DBusConnection* const connection,
		DBusMessage* message
	)
{
	GKDBusArgument::fillInArguments(message);
	bool ret = false;

	try {
		const std::string arg1( GKDBusArgumentString::getNextStringArgument() );
		const std::string arg2( GKDBusArgumentString::getNextStringArgument() );
		const uint8_t arg3 = GKDBusArgumentByte::getNextByteArgument();
		const uint64_t arg4 = GKDBusArgumentUInt64::getNextUInt64Argument();

		/* call two strings one byte one UInt64_t to bool callback */
		ret = this->callback(arg1, arg2, arg3, arg4);
	}
	catch ( const GLogiKExcept & e ) {
		/* send error if necessary when something was wrong */
		this->sendCallbackError(connection, message, e.what());
	}

	/* signals don't send reply */
	if(this->eventType == GKDBusEventType::GKDBUS_EVENT_SIGNAL)
		return;

	try {
		this->initializeReply(connection, message);
		this->appendBooleanToReply(ret);

		this->appendAsyncArgsToReply();
	}
	catch ( const GLogiKExcept & e ) {
		/* delete reply object if allocated and send error reply */
		this->sendReplyError(connection, message, e.what());
		return;
	}

	/* delete reply object if allocated */
	this->sendReply();
}

} // namespace NSGKDBus

