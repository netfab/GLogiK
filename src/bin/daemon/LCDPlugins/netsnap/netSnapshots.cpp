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
	try {
		this->findDefaultRouteNetworkInterfaceName();
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << e.what();
	}

	if( _defaultNetworkInterfaceName.empty() ) {
		throw GLogiKExcept("unable to find default route interface name");
	}
#if DEBUGGING_ON
	LOG(DEBUG2) << "found default route interface name: " << _defaultNetworkInterfaceName;
#endif

	_networkInterfaceName = _defaultNetworkInterfaceName;

	try {
		unsigned long long s1, s2 = 0;
		this->setBytesSnapshotValue(NetDirection::NET_RX, s1);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		this->setBytesSnapshotValue(NetDirection::NET_RX, s2);
		if( (s1 == 0) or (s2 == 0) )
			throw GLogiKExcept("wrong bytes snapshot");
		_rxDiff = (10 * (s2 - s1)); /* extrapolation */
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << e.what();
	}
}

NetSnapshots::~NetSnapshots()
{
}

const std::string NetSnapshots::getRxRateString(void)
{
	return this->getPaddedRateString(_rxDiff);
}

const std::string NetSnapshots::getPaddedRateString(unsigned long long value)
{
	std::ostringstream out("", std::ios_base::app);
	if(value < 1024) {
		out << std::setw(6) << std::to_string(value) << " B/s";
	}
	else {
		float kB = value / 1024.f;
		if(kB < 1024) {
			out << std::setw(5) << std::setprecision(2) << std::fixed << kB << " kB/s";
		}
		else {
			float mB = kB / 1024.f;
			out << std::setw(5) << std::setprecision(2) << std::fixed << mB << " mB/s";
		}
	}
	return out.str();
}

void NetSnapshots::findDefaultRouteNetworkInterfaceName(void)
{
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
		LOG(ERROR) << "vector index out of bounds : " << oor.what();
		throw GLogiKExcept("route line parsing error");
	}
	catch (const std::ifstream::failure & e) {
		LOG(ERROR) << "error opening/reading/closing kernel route file : " << e.what();
		throw GLogiKExcept("ifstream error");
	}
	/*
	 * catch std::ios_base::failure on buggy compilers
	 * should be fixed with gcc >= 7.0
	 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145
	 */
	catch( const std::exception & e ) {
		LOG(ERROR) << "(buggy exception) error opening/reading/closing route file : " << e.what();
		throw GLogiKExcept("ifstream error");
	}
}

void NetSnapshots::setBytesSnapshotValue(const NetDirection d, unsigned long long & value)
{
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
		value = to_ull(line);
	}
	catch (const std::ifstream::failure & e) {
		LOG(ERROR) << "error opening/reading/closing kernel route file : " << e.what();
		throw GLogiKExcept("ifstream error");
	}
	/*
	 * catch std::ios_base::failure on buggy compilers
	 * should be fixed with gcc >= 7.0
	 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145
	 */
	catch( const std::exception & e ) {
		LOG(ERROR) << "(buggy exception) error opening/reading/closing route file : " << e.what();
		throw GLogiKExcept("ifstream error");
	}
}

} // namespace GLogiK

