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

#ifndef SRC_BIN_DAEMON_LCDPLUGINS_NETSNAP_NET_SNAPSHOTS_HPP_
#define SRC_BIN_DAEMON_LCDPLUGINS_NETSNAP_NET_SNAPSHOTS_HPP_

#include <string>

namespace GLogiK
{

enum class NetDirection
{
	NET_RX = 0,
	NET_TX
};

class NetSnapshots
{
	public:
		NetSnapshots(void);
		~NetSnapshots(void);

		const std::string getRateString(NetDirection direction);

	protected:

	private:
		unsigned long long _rxDiff;
		unsigned long long _txDiff;
		std::string _defaultNetworkInterfaceName;
		std::string _networkInterfaceName;
		void findDefaultRouteNetworkInterfaceName(void);
		void setBytesSnapshotValue(const NetDirection d, unsigned long long & value);
		const std::string getRateString(
			unsigned long long value,
			const std::string & direction
		);
};

} // namespace GLogiK

#endif
