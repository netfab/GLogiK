/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
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
#include <string>
#include <fstream>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <iomanip>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "lib/utils/utils.hpp"

#include "netSnapshots.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

NetSnapshots::NetSnapshots()
	:	_rxDiff(0),
		_txDiff(0),
		_defaultNetworkInterfaceName(""),
		_networkInterfaceName("")
{
	GK_LOG_FUNC

	try {
		this->findDefaultRouteNetworkInterfaceName();
	}
	catch (const GLogiKExcept & e) {
		GKSysLogError(e.what());
	}

	if( _defaultNetworkInterfaceName.empty() ) {
		throw GLogiKExcept("unable to find default route interface name");
	}

	GKLog2(trace, "found default route interface name : ", _defaultNetworkInterfaceName)

	_networkInterfaceName = _defaultNetworkInterfaceName;

	try {
		unsigned long long s1, s2 = 0;
		unsigned long long s3, s4 = 0;
		this->setBytesSnapshotValue(NetDirection::NET_RX, s1);
		this->setBytesSnapshotValue(NetDirection::NET_TX, s3);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		this->setBytesSnapshotValue(NetDirection::NET_RX, s2);
		this->setBytesSnapshotValue(NetDirection::NET_TX, s4);
		if( (s1 == 0) or (s2 == 0) )
			throw GLogiKExcept("wrong RX bytes snapshot");
		if( (s3 == 0) or (s4 == 0) )
			throw GLogiKExcept("wrong TX bytes snapshot");
		_rxDiff = (10 * (s2 - s1)); /* extrapolation */
		_txDiff = (10 * (s4 - s3)); /* extrapolation */
	}
	catch (const GLogiKExcept & e) {
		GKSysLogError(e.what());
	}
}

NetSnapshots::~NetSnapshots()
{
}

const std::string NetSnapshots::getRateString(NetDirection direction)
{
	if(direction == NetDirection::NET_RX)
		return this->getRateString(_rxDiff, " - download");
	else
		return this->getRateString(_txDiff, " - upload  ");
}

const std::string NetSnapshots::getRateString(
	unsigned long long value,
	const std::string & direction)
{
	std::ostringstream buffer("", std::ios_base::app);
	std::string unit;
	if(value < 1024) {
		buffer << std::setw(4) << std::to_string(value);
		unit = " B/s";
	}
	else {
		float kB = value / 1024.f;
		if(kB < 1024) {
			buffer << std::setw(7) << std::fixed << kB;
			unit = " kB/s";
		}
		else {
			float mB = kB / 1024.f;
			buffer << std::setw(7) << std::fixed << mB;
			unit = " mB/s";
		}
	}

	const std::string rate(buffer.str());
	const std::size_t pos = rate.find_first_of('.');

	std::string out;
	if(pos == std::string::npos) {
		out += rate;
	}
	else {
		out += rate.substr(0, pos+3);
	}

	out += unit;
	out += direction;
	return out;
}

void NetSnapshots::findDefaultRouteNetworkInterfaceName(void)
{
	GK_LOG_FUNC

	try {
		std::ifstream routeFile("/proc/net/route");

		std::string line;
		std::getline(routeFile, line); /* skip first line */
		while( std::getline(routeFile, line) )
		{
			std::vector<std::string> results;
			boost::split(results, line, [](char c){return c == '\t';});

			if(results.at(1) == "00000000") { /* default route */
				_defaultNetworkInterfaceName = results[0];
				return;
			}
		}
	}
	catch (const std::out_of_range& oor) {
		GKSysLogError("vector index out of bounds : ", oor.what());
		throw GLogiKExcept("route line parsing error");
	}
	catch (const std::ifstream::failure & e) {
		GKSysLogError("error opening/reading/closing kernel route file : ", e.what());
		throw GLogiKExcept("ifstream error");
	}
}

void NetSnapshots::setBytesSnapshotValue(const NetDirection d, unsigned long long & value)
{
	GK_LOG_FUNC

	try {
		fs::path file("/sys/class/net");
		file /= _networkInterfaceName;
		file /= "statistics";
		if(d == NetDirection::NET_RX)
			file /= "rx_bytes";
		else
			file /= "tx_bytes";

		std::ifstream snapshotFile( file.string() );
		std::string line;
		std::getline(snapshotFile, line);
		value = toULL(line);
	}
	catch (const std::ifstream::failure & e) {
		GKSysLogError("error opening/reading/closing kernel route file : ", e.what());
		throw GLogiKExcept("ifstream error");
	}
}

} // namespace GLogiK

