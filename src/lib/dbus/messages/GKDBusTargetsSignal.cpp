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

#include <new>

#include "lib/utils/utils.h"

#include "GKDBusTargetsSignal.h"

namespace NSGKDBus
{

using namespace NSGKUtils;

void GKDBusMessageTargetsSignal::initializeTargetsSignal(
	BusConnection wantedConnection,
	const char* destination,
	const char* objectPath,
	const char* interface,
	const char* signal
) {
	this->initializeTargetsSignal(
		this->getConnection(wantedConnection),
		destination, objectPath, interface, signal);
}

void GKDBusMessageTargetsSignal::initializeTargetsSignal(
	DBusConnection* connection,
	const char* destination,
	const char* objectPath,
	const char* interface,
	const char* signal
) {
	if(_signals.size() > 0) { /* sanity check */
		throw GKDBusMessageWrongBuild("signals container not cleared");
	}

	std::vector<std::string> uniqueNames;

	try {
		/* asking the bus targets unique names */
		this->initializeRemoteMethodCall(
			connection,
			"org.freedesktop.DBus",
			"/org/freedesktop/DBus",
			"org.freedesktop.DBus",
			"ListQueuedOwners"
		);
		this->appendStringToRemoteMethodCall(destination);
		this->sendRemoteMethodCall();

		this->waitForRemoteMethodCallReply();

		uniqueNames = GKDBusArgumentString::getStringsArray();
	}
	catch ( const GKDBusMessageWrongBuild & e ) {
		this->abandonRemoteMethodCall();
		LOG(WARNING) << "failure to ask the bus for signal targets";
		throw;
	}

#if DEBUGGING_ON
	LOG(DEBUG) << "will send " << signal << " signal to " << uniqueNames.size() << " targets";
#endif

	try {
		for(const auto & uniqueName : uniqueNames) {
			_signals.push_back(
				new GKDBusBroadcastSignal(
					connection,
					uniqueName.c_str(),
					objectPath,
					interface,
					signal )
			);
		}
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		LOG(ERROR) << "GKDBus targets signal allocation failure : " << e.what();

		/* sending and freeing already allocated signals */
		for(const auto & targetSignal : _signals) {
			delete targetSignal;
		}
		_signals.clear();

		throw GKDBusMessageWrongBuild("allocation error");
	}

#if DEBUGGING_ON
	LOG(DEBUG1) << _signals.size() << " targets for " << signal << " signal initialized";
#endif
}

void GKDBusMessageTargetsSignal::appendStringToTargetsSignal(const std::string & value) {
	for(const auto & targetSignal : _signals) {
		if(targetSignal == nullptr) { /* sanity check */
			LOG(WARNING) << "signal object is null";
			continue;
		}
		targetSignal->appendString(value);
	}
}

void GKDBusMessageTargetsSignal::appendUInt8ToTargetsSignal(const uint8_t value) {
	for(const auto & targetSignal : _signals) {
		if(targetSignal == nullptr) {
			LOG(WARNING) << "signal object is null";
			continue;
		}
		targetSignal->appendUInt8(value);
	}
}

void GKDBusMessageTargetsSignal::appendStringVectorToTargetsSignal(const std::vector<std::string> & list) {
	for(const auto & targetSignal : _signals) {
		if(targetSignal == nullptr) {
			LOG(WARNING) << "signal object is null";
			continue;
		}
		targetSignal->appendStringVector(list);
	}
}

void GKDBusMessageTargetsSignal::sendTargetsSignal(void) {
	/* sending and freeing allocated signals */
	for(const auto & targetSignal : _signals) {
		if(targetSignal != nullptr) {
			delete targetSignal;
		}
		else {
			LOG(WARNING) << __func__ << " signal from container is null";
		}
	}
	_signals.clear();
}

void GKDBusMessageTargetsSignal::abandonTargetsSignal(void) {
	for(const auto & targetSignal : _signals) {
		if(targetSignal != nullptr) {
			targetSignal->abandon();
			delete targetSignal;
		}
		else {
			LOG(WARNING) << __func__ << " signal from container is null";
		}
	}
	_signals.clear();
}

} // namespace NSGKDBus

