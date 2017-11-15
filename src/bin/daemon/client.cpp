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

#include "lib/utils/utils.h"

#include "client.h"

namespace GLogiK
{

Client::Client(const std::string & object_path) : session_state_("unknown"),
	client_session_object_path_(object_path), check_("true")
{
	LOG(DEBUG2) << "initializing new client";
}

Client::~Client() {
	LOG(DEBUG2) << "destroying client";
}

const std::string & Client::getSessionObjectPath(void) const {
	return this->client_session_object_path_;
}

const std::string & Client::getSessionCurrentState(void) const {
	return this->session_state_;
}

void Client::updateSessionState(const std::string & new_state) {
	this->session_state_ = new_state;
	this->check_ = true;
}

void Client::uncheck(void) {
	this->check_ = false;
}

const bool Client::isAlive(void) {
	return this->check_;
}

} // namespace GLogiK

