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

#include "lib/utils/utils.h"

#include "warningCheck.h"

namespace GLogiK
{

uint8_t WarningCheck::warn_count_ = 0;

WarningCheck::WarningCheck() {
}

WarningCheck::~WarningCheck() {
}

void WarningCheck::warnOrThrows(const std::string & warn) {
	LOG(WARNING) << warn;
	if(WarningCheck::warn_count_ >= MAXIMUM_WARNINGS_BEFORE_FATAL_ERROR) {
		const std::string last = "maximum warning count reached, this is fatal";
		LOG(ERROR) << last;
		throw GLogiKExcept(last);
	}
	WarningCheck::warn_count_++;
}

} // namespace GLogiK

